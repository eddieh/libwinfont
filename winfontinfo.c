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

#include <winfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
usage()
{
    (void)fprintf(stderr, "usage: %s [-c char] [-s] fontpath ...\n",
	    getprogname());
}

void
print_ascii_art_glyph(WinFont *wf, int glyph)
{
#define UTF8_BIT_OFF " "
#define UTF8_BIT_ON  "█"
    int w, h, wBytes;
    uint8_t *bitmap;

    w = wf->width;
    h = wf->height;
    wBytes = wf->wbytes;
    bitmap = wf->bitmap;

    /* should use 'w' since there's no reason to print the pad
     * bits */
    (void)w;

    uint8_t *p = bitmap + (wBytes * h) * glyph;
    for (int i = 0; i < wBytes * h; i++) {
        if (i != 0 && i % wBytes == 0)
            fprintf(stderr, "\n");
        for (int j = 1; j < 9; j++)
            fprintf(stderr, "%s", ((1 << (8-j)) & *p) ?
                UTF8_BIT_ON : UTF8_BIT_OFF);
        p++;
    }
    fprintf(stderr, "\n");
}

int
main(int argc, char **argv)
{
    int ch, cflag = 0,
        sflag = 0;
    FILE *font;
    char *path;
    WinFont *wf = NULL;
    int glyph = 0;

    const char *opts = "c:s";
    while ((ch = getopt(argc, argv, opts)) != -1) {
        switch (ch) {
        case 'c':
            /* get character argument and print as ASCII art */
            if (sscanf(optarg, "%d", &glyph) == 0) {
                fprintf(stderr, "expected number got %s\n", optarg);
                exit(1);
            }
            cflag = 1;
            break;
        case 's':
            /* print short information */
            /* fprintf(stderr, "-%c not implemented yet\n", ch); */
            sflag = 1;
            break;
        default:
            usage();
            exit(1);
        }
    }

    argc -= optind;
	argv += optind;

    if (*argv == NULL) {
        usage();
        exit(1);
    }

    /* the rest of argv are paths */
    for (; *argv != NULL; argv++) {
        path = *argv;
        /* fprintf(stderr, "path: %s\n", path); */
        font = fopen(path, "rb");
        if (font == NULL) {
            fprintf(stderr, "Could not open: %s\n", path);
            continue;
        }

        wf = winfont_read_file(font);
        if (wf == NULL) {
            fprintf(stderr, "Unable to read: %s\n", path);
            fclose(font);
            continue;
        }

        /* fprintf(stderr, "WinFont[%s] ptr=%p\n", wf->facename, wf); */

        if (cflag)
            print_ascii_art_glyph(wf, glyph);

        if (sflag)
            (void)sflag;

        fclose(font);
        winfont_free(wf);
        wf = NULL;
    }

    return 0;
}
