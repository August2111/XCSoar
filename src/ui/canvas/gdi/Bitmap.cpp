/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "ui/canvas/Bitmap.hpp"
#include "ui/canvas/gdi/GdiPlusBitmap.hpp"

#include "Screen/Debug.hpp"
#include "system/Path.hpp"

#include <cassert>

#ifdef _AUG_MSC  // auch für MinGW & Co?...
// "hat bereits einen Funktionsrumpf"
// Bitmap::Bitmap() {
// 
// }

Bitmap::Bitmap(Bitmap&& src)
#   ifdef USE_MEMORY_CANVAS
#       if _AUG_MSC
{
  // TODO!!!!!!!!!!!!!!! bitmap gibt es nicht!
}
#       else
  :bitmap(src.bitmap) {
  src.bitmap = nullptr;
}
#       endif
// #endif
#   else
{
  // TODO!!!!!!!!!!!!!!! bitmap gibt es nicht!
}
#   endif
#endif


#ifdef _AUG_MSC  // Eingeführt, weil unter Windows LoadResource nicht geht ;-(
#define WITH_BOOST_GIL    1
#if WITH_BOOST_GIL
#   include <iostream>
#   include "boost/gil/extension/io/png.hpp"
#   include "boost/gil.hpp"
#   include "boost/gil/io/base.hpp"
#endif

HBITMAP
test_gil(std::string filename) {
    HBITMAP oBitmap = nullptr;
    DIBSECTION d;
    /*  geht nicht !!!!!!!!!!!!!!!!!!!!!!! */
#if WITH_BOOST_GIL
    boost::gil::rgb8_image_t image;
    try {
        boost::gil::read_image(filename, image, boost::gil::png_tag());
      // boost::gil::read_image(filename, image, boost::gil::png_tag());
    }
    catch (...) {
//        LogFormat(_T("boost exception!"));
    }

    if (image.width() > 0) {
        BITMAPINFO bi24BitInfo; //testing, populate to match rgb8_image_t
        memset(&bi24BitInfo, 0, sizeof(BITMAPINFO));
        bi24BitInfo.bmiHeader.biSize = sizeof(bi24BitInfo.bmiHeader);
        bi24BitInfo.bmiHeader.biBitCount = 24; // rgb 8 bytes for each  component(3)
        bi24BitInfo.bmiHeader.biCompression = BI_RGB;// rgb = 3 components
        bi24BitInfo.bmiHeader.biPlanes = 1;
        bi24BitInfo.bmiHeader.biWidth = image.width(); // = d.dsBm.bmWidth
        bi24BitInfo.bmiHeader.biHeight = image.height();  //  = d.dsBm.bmHeight

        oBitmap = ::CreateDIBSection(NULL, &bi24BitInfo,
            DIB_RGB_COLORS, 0, 0, 0);

        uint32_t nBytes = ::GetObject(oBitmap, sizeof(DIBSECTION), &d);


#if 1
        memcpy(d.dsBm.bmBits, &image, d.dsBmih.biSizeImage);
#else
        for (size_t l = image.height(); l-- > 0; ) {
            memcpy((char*)d.dsBm.bmBits + l * d.dsBmih.biBitCount * image.width(),
                &image, d.dsBmih.biBitCount * image.width());
        }
#endif
    }
#endif  // WITH_BOOST_GIL

    return oBitmap;
}

bool Bitmap::LoadFile(Path path) {
    // std::string img_filename = ;
    bitmap = (HBITMAP)LoadImageA(nullptr, path.ToUTF8().c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return bitmap;
}
#else  // _AUG_MSC
bool
Bitmap::LoadFile(Path path)
{
  bitmap = GdiLoadImage(path.c_str());
  return IsDefined();
}
#endif  // _AUG_MSC

void
Bitmap::Reset()
{
  if (bitmap != nullptr) {
    assert(IsScreenInitialized());

#ifndef NDEBUG
    bool success =
#endif
      ::DeleteObject(bitmap);
#if  !_AUG_MSC  // strirbt hier momentan ???
    assert(success);
#endif

    bitmap = nullptr;
  }
}

PixelSize
Bitmap::GetSize() const
{
  assert(IsDefined());

  BITMAP bm;
  ::GetObject(bitmap, sizeof(bm), &bm);
  const PixelSize size = { bm.bmWidth, bm.bmHeight };
  return size;
}
