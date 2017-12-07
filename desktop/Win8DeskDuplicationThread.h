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

#ifndef __WIN8DESKDUPLICATIONTHREAD_H__
#define __WIN8DESKDUPLICATIONTHREAD_H__

#include "rfb/FrameBuffer.h"
#include "Win8CursorShape.h"
#include "thread/LocalMutex.h"
#include "thread/GuiThread.h"
#include "Win8DuplicationListener.h"
#include "log-writer/LogWriter.h"

#include "WinCustomD3D11Texture2D.h"
#include "WinDxgiOutputDuplication.h"

class Win8DeskDuplicationThread : public GuiThread
{
public:
  // The WinDxgiOutput *dxgiOutput passed object can be destroyed right after the constructor calling.
  // The WinD3D11Device *device passed object can be destroyed right after the constructor calling.
  Win8DeskDuplicationThread(FrameBuffer *targetFb,
                            const Rect *targetRect,
                            Win8CursorShape *targetCurShape,
                            LONGLONG *cursorTimeStamp,
                            LocalMutex *cursorMutex,
                            Win8DuplicationListener *duplListener,
                            WinDxgiOutput *dxgiOutput,
                            int threadNumber,
                            LogWriter *log);
  virtual ~Win8DeskDuplicationThread();

  bool isValid();

private:
  virtual void execute();
  virtual void onTerminate();

  void setCriticalError(const TCHAR *reason);
  void setRecoverableError(const TCHAR *reason);

  void processMoveRects(size_t moveCount);
  void processDirtyRects(size_t dirtyCount,
                         WinD3D11Texture2D *acquiredDesktopImage);
  void processCursor(const DXGI_OUTDUPL_FRAME_INFO *info);

  Dimension getStageDimension() const;

  void rotateRectInsideStage(Rect *toTranspose, const Dimension *stageDim, DXGI_MODE_ROTATION rotation);

  int m_threadNumber;

  FrameBuffer *m_targetFb;

  Rect m_targetRect;
  Win8CursorShape *m_targetCurShape;
  LONGLONG *m_cursorTimeStamp;
  LocalMutex *m_cursorMutex;

  DXGI_MODE_ROTATION m_rotation;

  Win8DuplicationListener *m_duplListener;

  WinD3D11Device m_device;
  WinDxgiOutput1 m_dxgiOutput1;
  WinDxgiOutputDuplication m_outDupl;

  // The duplication interface can't be used
  bool m_hasCriticalError;
  // The interface can be used but it should be reinitialized.
  bool m_hasRecoverableError;

  // Use this variables as class fields to avoid frequency memory allocations.
  std::vector<RECT> m_dirtyRects;
  std::vector<DXGI_OUTDUPL_MOVE_RECT> m_moveRects;

  WinCustomD3D11Texture2D m_stageTexture2D;
  FrameBuffer m_auxiliaryFrameBuffer;

  LogWriter *m_log;
};

#endif // __WIN8DESKDUPLICATIONTHREAD_H__
