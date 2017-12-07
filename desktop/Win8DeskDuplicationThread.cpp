// Copyright (C) 2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "WinDxRecoverableException.h"
#include "WinDxCriticalException.h"
#include "thread/AutoLock.h"

#include "WinDxgiAcquiredFrame.h"
#include "WinD3D11Texture2D.h"
#include "WinAutoMapDxgiSurface.h"

#include "Win8DeskDuplicationThread.h"

Win8DeskDuplicationThread::Win8DeskDuplicationThread(FrameBuffer *targetFb,
                                                     const Rect *targetRect,
                                                     Win8CursorShape *targetCurShape,
                                                     LONGLONG *cursorTimeStamp,
                                                     LocalMutex *cursorMutex,
                                                     Win8DuplicationListener *duplListener,
                                                     WinDxgiOutput *dxgiOutput,
                                                     int threadNumber,
                                                     LogWriter *log)
: m_targetFb(targetFb),
  m_targetRect(*targetRect),
  m_targetCurShape(targetCurShape),
  m_cursorTimeStamp(cursorTimeStamp),
  m_cursorMutex(cursorMutex),
  m_rotation(dxgiOutput->getRotation()),
  m_duplListener(duplListener),
  m_dxgiOutput1(dxgiOutput),
  m_device(log),
  m_outDupl(&m_dxgiOutput1, &m_device),
  m_hasCriticalError(false),
  m_hasRecoverableError(false),
  m_stageTexture2D(m_device.getDevice(),
                   (UINT)targetRect->getWidth(),
                   (UINT)targetRect->getHeight(),
                   m_rotation),
  m_threadNumber(threadNumber),
  m_log(log)
{
  resume();
}

Win8DeskDuplicationThread::~Win8DeskDuplicationThread()
{
  terminate();
  wait();
}

bool Win8DeskDuplicationThread::isValid()
{
  return !m_hasRecoverableError && !m_hasCriticalError;
}

void Win8DeskDuplicationThread::execute()
{
  try {
    while (!isTerminating() && isValid()) {
      WinDxgiAcquiredFrame acquiredFrame(&m_outDupl, 20);
      if (acquiredFrame.wasTimeOut()) {
		m_log->debug(_T("Timeout on acquire frame for output:%d"), m_threadNumber);
        continue;
      }

      WinD3D11Texture2D acquiredDesktopImage(acquiredFrame.getDxgiResource());
      DXGI_OUTDUPL_FRAME_INFO *info = acquiredFrame.getFrameInfo();

      // Get metadata
      if (info->TotalMetadataBufferSize) {
        size_t moveCount = m_outDupl.getFrameMoveRects(&m_moveRects);
        size_t dirtyCount = m_outDupl.getFrameDirtyRects(&m_dirtyRects);

        processMoveRects(moveCount);
        processDirtyRects(dirtyCount, &acquiredDesktopImage);
      }

      // Check cursor pointer for updates.
      {
        processCursor(info);
      } // Cursor
    }
  } catch (WinDxRecoverableException &e) {
    StringStorage errMess;
    errMess.format(_T("Catched WinDxRecoverableException: %s, (%d)"), e.getMessage(), (int)e.getErrorCode());
    setRecoverableError(errMess.getString());
  } catch (WinDxCriticalException &e) {
    StringStorage errMess;
    errMess.format(_T("Catched WinDxCriticalException: %s, (%d)"), e.getMessage(), (int)e.getErrorCode());
    setRecoverableError(errMess.getString());
  } catch (Exception &e) {
    StringStorage errMess;
    errMess.format(_T("Catched WinDxCriticalException: %s") , e.getMessage());
    setRecoverableError(errMess.getString());
  }
}

void Win8DeskDuplicationThread::onTerminate()
{
}

void Win8DeskDuplicationThread::setCriticalError(const TCHAR *reason)
{
  m_hasCriticalError = true;
  m_duplListener->onCriticalError(reason);
}

void Win8DeskDuplicationThread::setRecoverableError(const TCHAR *reason)
{
  m_hasRecoverableError = true;
  m_duplListener->onRecoverableError(reason);
}

Dimension Win8DeskDuplicationThread::getStageDimension() const
{
  return Dimension(m_stageTexture2D.getDesc()->Width, m_stageTexture2D.getDesc()->Height);
}

void Win8DeskDuplicationThread::processMoveRects(size_t moveCount)
{
  _ASSERT(moveCount <= m_moveRects.size());
  Rect destinationRect;
  Rect sourceRect;
  for (size_t iRect = 0; iRect < moveCount; iRect++) {
    destinationRect.fromWindowsRect(&m_moveRects[iRect].DestinationRect);
    sourceRect = destinationRect;
    POINT srcPoint = m_moveRects[iRect].SourcePoint;
    sourceRect.setLocation(srcPoint.x, srcPoint.y);
    rotateRectInsideStage(&destinationRect, &getStageDimension(), m_rotation);
    rotateRectInsideStage(&sourceRect, &getStageDimension(), m_rotation);
    // Translate the rect and point to the frame buffer coordinates.
    destinationRect.move(m_targetRect.left, m_targetRect.top);
    sourceRect.move(m_targetRect.left, m_targetRect.top);
    int x = sourceRect.left;
    int y = sourceRect.top;
    m_targetFb->move(&destinationRect, x, y);

    m_duplListener->onCopyRect(&destinationRect, x, y);
  }
}

void Win8DeskDuplicationThread::processDirtyRects(size_t dirtyCount,
                                                  WinD3D11Texture2D *acquiredDesktopImage)
{
  _ASSERT(dirtyCount <= m_dirtyRects.size());

  Region changedRegion;

  Rect dirtyRect;
  Dimension stageDim = getStageDimension();
  Rect stageRect = stageDim.getRect();
  for (size_t iRect = 0; iRect < dirtyCount; iRect++) {
    dirtyRect.fromWindowsRect(&m_dirtyRects[iRect]);

    if (!stageRect.isFullyContainRect(&dirtyRect)) {
      dirtyRect = dirtyRect.intersection(&stageRect);
      /* Disabled the followed throwing because it realy may happen and better is to see any picture
      // instead of a black screen.
      StringStorage errMess;
      errMess.format(_T("During processDirtyRects has been got a rect (%d, %d, %dx%d) which outside")
                     _T(" from the stage rect (%d, %d, %dx%d)"),
                     rect.left, rect.top, rect.getWidth(), rect.getHeight(),
                     stageRect.left, stageRect.top, stageRect.getWidth(), stageRect.getHeight());
      throw Exception(errMess.getString());
      */
    }

    m_device.copySubresourceRegion(m_stageTexture2D.getTexture(), dirtyRect.left, dirtyRect.top,
      acquiredDesktopImage->getTexture(), &dirtyRect, 0, 1);

    WinDxgiSurface surface(m_stageTexture2D.getTexture());
    WinAutoMapDxgiSurface autoMapSurface(&surface, DXGI_MAP_READ);

    Rect dstRect(dirtyRect);
    rotateRectInsideStage(&dstRect, &stageDim, m_rotation);
    // Translate the rect to the frame buffer coordinates.
    dstRect.move(m_targetRect.left, m_targetRect.top);
    m_log->debug(_T("Destination dirty rect = %d, %d, %dx%d"), dstRect.left, dstRect.top, dstRect.getWidth(), dstRect.getHeight());

    stageDim.width = static_cast<int> (autoMapSurface.getStride() / 4);
    m_auxiliaryFrameBuffer.setPropertiesWithoutResize(&stageDim, &m_targetFb->getPixelFormat());
    m_auxiliaryFrameBuffer.setBuffer(autoMapSurface.getBuffer());
    switch (m_rotation)
    {
      case DXGI_MODE_ROTATION_UNSPECIFIED:
      case DXGI_MODE_ROTATION_IDENTITY:
      {
        m_targetFb->copyFrom(&dstRect, &m_auxiliaryFrameBuffer, dirtyRect.left, dirtyRect.top);
        break;
      }
      case DXGI_MODE_ROTATION_ROTATE90:
      {
        m_targetFb->copyFromRotated90(&dstRect, &m_auxiliaryFrameBuffer, dirtyRect.left, dirtyRect.top);
        break;
      }
      case DXGI_MODE_ROTATION_ROTATE180:
      {
        m_targetFb->copyFromRotated180(&dstRect, &m_auxiliaryFrameBuffer, dirtyRect.left, dirtyRect.top);
        break;
      }
      case DXGI_MODE_ROTATION_ROTATE270:
      {
        m_targetFb->copyFromRotated270(&dstRect, &m_auxiliaryFrameBuffer, dirtyRect.left, dirtyRect.top);
        break;
      }
    }
    m_auxiliaryFrameBuffer.setBuffer(0);

    changedRegion.addRect(&dstRect);
  }

  m_duplListener->onFrameBufferUpdate(&changedRegion);
}

void Win8DeskDuplicationThread::rotateRectInsideStage(Rect *toTranspose,
                                                      const Dimension *stageDim,
                                                      DXGI_MODE_ROTATION rotation)
{
  int left = toTranspose->left;
  int top = toTranspose->top;
  int width = toTranspose->getWidth();
  int height = toTranspose->getHeight();
  switch (rotation)
  {
    case DXGI_MODE_ROTATION_UNSPECIFIED:
    case DXGI_MODE_ROTATION_IDENTITY:
    {
      return;
    }
    case DXGI_MODE_ROTATION_ROTATE90:
    {
      toTranspose->rotateOn90InsideDimension(stageDim->height);
      break;
    }
    case DXGI_MODE_ROTATION_ROTATE180:
    {
      toTranspose->rotateOn180InsideDimension(stageDim->width, stageDim->height);
      break;
    }
    case DXGI_MODE_ROTATION_ROTATE270:
    {
      toTranspose->rotateOn270InsideDimension(stageDim->width);
      break;
    }
  }
}

void Win8DeskDuplicationThread::processCursor(const DXGI_OUTDUPL_FRAME_INFO *info)
{
  AutoLock al(m_cursorMutex);
  LONGLONG lastUpdateTime = info->LastMouseUpdateTime.QuadPart;
  if (lastUpdateTime != 0 && lastUpdateTime > *m_cursorTimeStamp) {
    *m_cursorTimeStamp = lastUpdateTime;

    //
    DXGI_OUTDUPL_POINTER_POSITION pointerPos = info->PointerPosition;

    bool newVisibility = pointerPos.Visible != FALSE;
    bool visibleChanged = m_targetCurShape->getIsVisible() != newVisibility;
    if (visibleChanged) {
      m_targetCurShape->setVisibility(newVisibility, m_threadNumber);
      m_duplListener->onCursorShapeChanged();
    }

    //
    bool shapeChanged = info->PointerShapeBufferSize != 0;
    if (shapeChanged) {
      m_outDupl.getFrameCursorShape(m_targetCurShape->getCursorShapeForWriting(), info->PointerShapeBufferSize);
      m_duplListener->onCursorShapeChanged();
    }

    if (pointerPos.Visible) {
      Point hotPoint = m_targetCurShape->getCursorShape()->getHotSpot();
      m_duplListener->onCursorPositionChanged(pointerPos.Position.x + m_targetRect.left + hotPoint.x,
                                              pointerPos.Position.y + m_targetRect.top + hotPoint.y);
    }
  }
}
