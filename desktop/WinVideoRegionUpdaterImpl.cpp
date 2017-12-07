// Copyright (C) 2013 GlavSoft LLC.
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

#include "WinVideoRegionUpdaterImpl.h"
#include "server-config-lib/Configurator.h"
#include "gui/WindowFinder.h"
#include "region/RectSerializer.h"

WinVideoRegionUpdaterImpl::WinVideoRegionUpdaterImpl()
{
}

WinVideoRegionUpdaterImpl::~WinVideoRegionUpdaterImpl()
{
}

void WinVideoRegionUpdaterImpl::getVideoRegion(Region *dstVidRegion)
{
  updateVideoRegion();
  *dstVidRegion = m_vidRegion;
}

void WinVideoRegionUpdaterImpl::updateVideoRegion()
{
  ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();
  unsigned int interval = srvConf->getVideoRecognitionInterval();

  DateTime curTime = DateTime::now();
  if ((curTime - m_lastVidUpdTime).getTime() > interval) {
    m_lastVidUpdTime = DateTime::now();
    StringVector classNames;
    std::vector<Rect> rects;
    m_vidRegion.clear();
    {
      AutoLock al(srvConf);
      classNames = *srvConf->getVideoClassNames();
      rects = *srvConf->getVideoRects();
    }
    addRectsByClass(&classNames);
    addRectsByCoords(&rects);
  }
}

void WinVideoRegionUpdaterImpl::addRectsByClass(StringVector *classNames)
{
  std::vector<HWND> hwndVector;
  std::vector<HWND>::iterator hwndIter;

  WindowFinder::findWindowsByClass(classNames, &hwndVector);

  for (hwndIter = hwndVector.begin(); hwndIter != hwndVector.end(); hwndIter++) {
    HWND videoHWND = *hwndIter;
    if (videoHWND != 0) {
      WINDOWINFO wi;
      wi.cbSize = sizeof(WINDOWINFO);
      if (GetWindowInfo(videoHWND, &wi)) {
        Rect videoRect(wi.rcClient.left, wi.rcClient.top,
                       wi.rcClient.right, wi.rcClient.bottom);
        if (videoRect.isValid()) {
          videoRect.move(-GetSystemMetrics(SM_XVIRTUALSCREEN),
                         -GetSystemMetrics(SM_YVIRTUALSCREEN));
          m_vidRegion.addRect(&videoRect);
        }
      }
    }
  }
}

void WinVideoRegionUpdaterImpl::addRectsByCoords(std::vector<Rect> *rects)
{
  std::vector<Rect> r = *rects;
  std::vector<Rect>::iterator rIter;
  Rect videoRect;
  for (rIter = r.begin(); rIter != r.end(); rIter++) {
    videoRect  = *rIter;
    if (videoRect.isValid()) {
      m_vidRegion.addRect(&videoRect);
    }
  }
}