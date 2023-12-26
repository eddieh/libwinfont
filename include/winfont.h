/*
 * MIT License
 *
 * Copyright (c) 2023 Eddie Hillenbrand
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef WINFONT_H
#define WINFONT_H

#include <stdio.h>
#include <stdint.h>

typedef enum {
    WinFont_CharSetANSI = 0,
    WinFont_CharSetDefault = 1,
    WinFont_CharSetSymbol = 2,
    WinFont_CharSetOEM = 255,
    WinFont_CharSetCP437 = WinFont_CharSetOEM,
    WinFont_CharSetIBM437 = WinFont_CharSetOEM,
} WinFont_CharSet;

typedef struct FontDirEntry WinFont_Info;

typedef struct {
    char *facename;             /* null-terminated face name */
    int nglyphs;                /* number of glyphs in font */
    int width;                  /* glyph width in pixels */
    int height;                 /* glyph height in pixels */
    int wbytes;                 /* glyph byte width */
    WinFont_CharSet charset;    /* probably CP437 */
    WinFont_Info *_fn_info;     /* private */
    uint8_t *bitmap;            /* all glyphs */
} WinFont;

const char *
winfont_version();

WinFont *
winfont_read_file(FILE *f);

WinFont *
winfont_read_path(char *path);

void
winfont_free(WinFont *wf);

size_t
winfont_glyph_required_size(WinFont *wf, int g);

int
winfont_glyph_bitmap(WinFont *wf, int g, uint8_t *bm, size_t sz);

#endif /* WINFONT_H */
