/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2009 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include <stdint.h>
#include "red_window.h"
#include "red_sw_canvas.h"
#include "utils.h"
#include "debug.h"
#include "region.h"
#include "red_pixmap_sw.h"

SCanvas::SCanvas(bool onscreen,
                 int width, int height, uint32_t format, RedWindow *win,
                 PixmapCache& pixmap_cache, PaletteCache& palette_cache,
                 GlzDecoderWindow &glz_decoder_window, CSurfaces& csurfaces)
    : Canvas (pixmap_cache, palette_cache, glz_decoder_window, csurfaces)
    , _pixmap (0)
#ifdef USE_BENCHMARK
    , _record_fp (NULL)
#endif
{
    if (onscreen) {
        _pixmap = new RedPixmapSw(width, height,
                                  RedDrawable::format_from_surface(format),
                                  true, win);
        _canvas = canvas_create_for_data(width, height, format,
                                         _pixmap->get_data(),
                                         _pixmap->get_stride(),
                                         &pixmap_cache.base,
                                         &palette_cache.base,
                                         &csurfaces.base,
                                         &glz_decoder(),
                                         &jpeg_decoder(),
                                         &zlib_decoder());
    } else {
        _canvas = canvas_create(width, height, format,
                                &pixmap_cache.base,
                                &palette_cache.base,
                                &csurfaces.base,
                                &glz_decoder(),
                                &jpeg_decoder(),
                                &zlib_decoder());
    }
    if (_canvas == NULL) {
        THROW("create canvas failed");
    }
}

SCanvas::~SCanvas()
{
    _canvas->ops->destroy(_canvas);
    _canvas = NULL;
    if (_pixmap) {
        delete _pixmap;
        _pixmap = NULL;
    }
}

void SCanvas::copy_pixels(const QRegion& region, RedDrawable& dest_dc)
{
    pixman_box32_t *rects;
    int num_rects;
#ifdef USE_BENCHMARK
    int update_pct = 0;
#endif

    ASSERT(_pixmap != NULL);

    rects = pixman_region32_rectangles((pixman_region32_t *)&region, &num_rects);
    for (int i = 0; i < num_rects; i++) {
        SpiceRect r;

        r.left = rects[i].x1;
        r.top = rects[i].y1;
        r.right = rects[i].x2;
        r.bottom = rects[i].y2;
        dest_dc.copy_pixels(*_pixmap, r.left, r.top, r);
    }
#ifdef USE_BENCHMARK
    if( _record_fp && _is_record_display && update_pct )
        fprintf(_record_fp, "%Ld O %d\n", Platform::get_monolithic_time() / 1000 - _record_start_time, update_pct * 100 / (_pixmap->get_height() * _pixmap->get_width()) );
#endif
}

#ifdef USE_BENCHMARK
void SCanvas::set_record_info(FILE *record_fp, uint64_t record_start_time, bool is_record_display)
{
    _record_fp = record_fp;
    _record_start_time = record_start_time;
    _is_record_display = is_record_display;

    printf( "%s: record_fp=%p, record_start_time=%ld, is_record_display=%d\n", __func__, record_fp, record_start_time, is_record_display );
}

void SCanvas::record_pixels(SpiceRect rect)
{
    ASSERT(_pixmap != NULL);

    if( _record_fp ) {
        int i, j;
        uint8_t *data = _pixmap->get_data();
        int pixmap_width = _pixmap->get_width();
        int pixmap_height = _pixmap->get_height();
        int left  = rect.left >= 0 ? rect.left : 0;
        int right = rect.right < pixmap_width ? rect.right : pixmap_width;
        int top = rect.top >= 0 ? rect.top : 0;
        int bottom = rect.bottom < pixmap_height ? rect.bottom : pixmap_height;
        int width = right - left + 1;
        int height = bottom - top + 1;
        
        fprintf( _record_fp, "%Ld S %d\n", Platform::get_monolithic_time() / 1000 - _record_start_time, width * height );
        for( i=0 ; i < width ; i++ ) {
            for( j=0 ; j < height ; j++ ) {
                int x = left + i;
                int y = top + j;
                fprintf( _record_fp, "%x\n", *((uint32_t *)data + x + (y * pixmap_width)) );
            }
        }
    }
}

int32_t SCanvas::check_snapshot_sync(uint32_t *snapshot_pixels, SpiceRect rect)
{
    int32_t num_diff = 0, k = 0;
    int i, j;
    uint8_t *data = _pixmap->get_data();
    int pixmap_width = _pixmap->get_width();
    int pixmap_height = _pixmap->get_height();
    int left  = rect.left >= 0 ? rect.left : 0;
    int right = rect.right < pixmap_width ? rect.right : pixmap_width;
    int top = rect.top >= 0 ? rect.top : 0;
    int bottom = rect.bottom < pixmap_height ? rect.bottom : pixmap_height;
    int width = right - left + 1;
    int height = bottom - top + 1;
    for( i=0 ; i < width ; i++ ) {
        for( j=0 ; j < height ; j++ ) {
            int x = left + i;
            int y = top + j;
            //fprintf( record_fp, "%x\n", *((uint32_t *)data + x + (y * pixmap_width)) );
            if( snapshot_pixels[k] != *((uint32_t *)data + x + (y * pixmap_width)) ) {
                num_diff++;
            }
            printf( "[SNAP] %d: cur=%x <-> snap=%x\n", k, 
                    *((uint32_t *)data + x + (y * pixmap_width)), snapshot_pixels[k] );
            k++;
        }
    }
    printf( "[SNAP] --> num_diff=%d\n", num_diff );
    return num_diff;
}
#endif

void SCanvas::copy_pixels(const QRegion& region, RedDrawable* dest_dc, const PixmapHeader* pixmap)
{
    copy_pixels(region, *dest_dc);
}

CanvasType SCanvas::get_pixmap_type()
{
    return CANVAS_TYPE_SW;
}

