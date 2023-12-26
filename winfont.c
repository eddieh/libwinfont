/* I did examine FontForge and FreeType. However, this has an
 * independent implemtation. */

#include <winfont.h>

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PACKED __attribute__((__packed__))

/* MZ are the initials of Mark Zbikowski. He created the MS-DOS
 * executable format. */
#define IMAGE_DOS_SIGNATURE    0x5A4D     /* MZ   */
#define IMAGE_OS2_SIGNATURE    0x454E     /* NE   */

#define FON_MZ_MAGIC    IMAGE_DOS_SIGNATURE
#define FON_NE_MAGIC    IMAGE_OS2_SIGNATURE

typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;

/* Called IMAGE_DOS_HEADER in winnt.h */
typedef struct PACKED {
    WORD   e_magic;          /* Magic number */
    WORD   _skip[29];
    DWORD  e_lfanew;         /* File address of new exe (NE) header */
} MZ_Header;

/* Called IMAGE_OS2_HEADER in winnt.h */
typedef struct PACKED {
    WORD  ne_magic;             /* NE signature 'NE' */
    BYTE  _skip[34];
    WORD  ne_rsrctab;           /* Offset to resource table */
    WORD  ne_restab;            /* Offset to resident-name table */
    /* The struct has more fields here
        ...
       But we have no need for them
    */
    /* WORD  ne_expver; */
} NE_Header;

#define RT_FONTDIR 0x8007
#define RT_FONT    0x8008

typedef struct PACKED {
    WORD  reType;
    WORD  reCount;
    DWORD _pad;
    WORD  reOffset;
    BYTE  _pad2[10];
} ResEntry;

/* Unused. Included as informal documentation. This struct proceeds
 * the FontDirEntry structure of type RT_FONTDIR. It is at the
 * location that the ResEntry offset points to. */
typedef struct PACKED {
    WORD  NumberOfFonts;
    WORD  fontOrdinal;          /* Actually a DirEntry struct with one
                                   member called fontOrdinal, but why
                                   the indirection?  */
} FontGroupHdr;

#define DF_VER2 0x200
#define DF_VER3 0x300

/* According to MS docs no header actually contains this struct:

       The structure definition provided here is for explanation only;
       it is not present in any standard header file.

   https://learn.microsoft.com/en-us/windows/win32/menurc/fontdirentry

   The MS docs also mention there's one FONTDIRENTRY struct for every
   font in the resource file and that applications that generate font
   resource must add a FONTDIRENTRY struct for each font. This makes
   it sound like there's two FONTDIRENTRY structs for every font
   resource.

   The ResEntry of the type RT_FONT points to the start of this
   stucture and the CharTable follows.

   This struct is called FONTINFO in some contexts. */
typedef struct PACKED {
    WORD   dfVersion;
    DWORD  dfSize; /* ATTN: Struct must be packed, otherwise offset of
                      field is 4 instead of 2 */
    char   dfCopyright[60];
    WORD   dfType;
    WORD   dfPoints;
    WORD   dfVertRes;
    WORD   dfHorizRes;
    WORD   dfAscent;
    WORD   dfInternalLeading;
    WORD   dfExternalLeading;
    BYTE   dfItalic;
    BYTE   dfUnderline;
    BYTE   dfStrikeOut;
    WORD   dfWeight;
    BYTE   dfCharSet;
    WORD   dfPixWidth;
    WORD   dfPixHeight;
    BYTE   dfPitchAndFamily;
    WORD   dfAvgWidth;
    WORD   dfMaxWidth;
    BYTE   dfFirstChar;
    BYTE   dfLastChar;
    BYTE   dfDefaultChar;
    BYTE   dfBreakChar;
    WORD   dfWidthBytes;
    DWORD  dfDevice;
    DWORD  dfFace;
    DWORD  dfBitsPointer;
    DWORD  dfBitsOffset;
    BYTE   dfReserved;

    /* The FontDirEntry_v3_Fields follow, but are excluded so that
       this struct works with both v2 and v3 fonts. */

    /* For raster fonts, the CharTable is an array of entries each
       consisting of two 2-byte WORDs for Windows 2.x and three 2-byte
       WORDs for Windows 3.0.

       CharTable v2 len = 2 * ((lastchar - firstchar) + 2)
       CharTable v3 len = 3 * ((lastchar - firstchar) + 2)
    */

    /* The struct ends with a flexible member, but is excluded for
       this struct to works with both v2 and v3 fonts.

       WORD   dfCharTable[];
    */
} FontDirEntry;

/* Win 3.x fields */
typedef struct PACKED {
    DWORD  dfFlags;
    WORD   dfAspace;
    WORD   dfBspace;
    WORD   dfCspace;
    DWORD  dfColorPointer;
    BYTE   dfReserved1[16];
} FontDirEntry_v3_Fields;

typedef struct PACKED {
    WORD  width;
    WORD  offset;
} CharInfo_v2;

typedef struct PACKED {
    WORD   width;
    DWORD  offset;
} CharInfo_v3;

/* From KB 65123 - dfFlags */
#define DFF_FIXED            0x0001 /* font is fixed pitch */
#define DFF_PROPORTIONAL     0x0002 /* font is proportional pitch */
#define DFF_ABCFIXED         0x0004 /* font is an ABC fixed font */
#define DFF_ABCPROPORTIONAL  0x0008 /* font is an ABC pro-portional font */
#define DFF_1COLOR           0x0010 /* font is one color */
#define DFF_16COLOR          0x0020 /* font is 16 color */
#define DFF_256COLOR         0x0040 /* font is 256 color */
#define DFF_RGBCOLOR         0x0080 /* font is RGB color */

/* wingdi.h */

/* Pitch */
#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2
#define MONO_FONT               8

/* Font Families */
#define FF_DONTCARE   (0<<4)    /* Don't care or don't know. */
#define FF_ROMAN      (1<<4)    /* Proportionally spaced fonts
                                   with serifs. */
#define FF_SWISS      (2<<4)    /* Proportionally spaced fonts
                                   without serifs. */
#define FF_MODERN     (3<<4)    /* Fixed-pitch fonts. */
#define FF_SCRIPT     (4<<4)
#define FF_DECORATIVE (5<<4)

/* Roman:  Variable stroke width, serifed.
           e.g. Times Roman, Century Schoolbook, etc.

   Swiss:  Variable stroke width, sans-serifed.
           e.g. Helvetica, Swiss, etc.

   Modern: Constant stroke width, serifed or sans-serifed.
           e.g. Pica, Elite, Courier, etc.

   Script: Cursive, etc.

   Decorative: Old English, etc.
*/

/* Font Weights */
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

/* Character Sets */
#define CHARSET_ANSI            0
#define CHARSET_DEFAULT         1
#define CHARSET_SYMBOL          2
#define CHARSET_SHIFTJIS        128
#define CHARSET_HANGEUL         129
#define CHARSET_HANGUL          129
#define CHARSET_GB2312          134
#define CHARSET_CHINESEBIG5     136
#define CHARSET_OEM             255 /* Alias for CP437 (aka IBM437) */

#define CHARSET_JOHAB           130
#define CHARSET_HEBREW          177
#define CHARSET_ARABIC          178
#define CHARSET_GREEK           161
#define CHARSET_TURKISH         162
#define CHARSET_VIETNAMESE      163
#define CHARSET_THAI            222
#define CHARSET_EASTEUROPE      238
#define CHARSET_RUSSIAN         204

#define CHARSET_MAC             77
#define CHARSET_BALTIC          186

/* end wingdi.h */

char *
winfont_read_string(long stroff, FILE *fnt)
{
    long saveoff;
    int ch, len = 0;
    char *str = NULL;

    saveoff = ftell(fnt);
    if (fseek(fnt, stroff, SEEK_SET) == -1) {
        return NULL;
    }

    while ((ch = getc(fnt)))
        len++;

    if (fseek(fnt, stroff, SEEK_SET) == -1) {
        goto restoreoffset;
    }

    str = malloc(len + 1);
    if (!str) {
        fprintf(stderr, "OOM\n");
        goto restoreoffset;
    }

    if (fread(str, sizeof(char), len, fnt) < len) {
        free(str);
        str = NULL;
        goto restoreoffset;
    }

    str[len] = 0;

restoreoffset:
    fseek(fnt, saveoff, SEEK_SET);
    return str;
}

uint8_t *
winfont_read_bitmap(int w, int h, int wbytes,
    int nglyphs, FILE *fnt)
{
    int bmBytes;
    uint8_t *bm = NULL;

    bmBytes = wbytes * h * nglyphs;
    bm = calloc(bmBytes, sizeof(uint8_t));
    if (!bm) {
        fprintf(stderr, "OOM\n");
        return NULL;
    }

    uint8_t *cb = bm;
    for (int c = 0; c < nglyphs; c++) {
        uint8_t *dest = cb;
        uint8_t *col = cb;
        for (int i = 0; i < wbytes * h; i++) {
            if (i != 0 && i % h == 0)
                dest = ++col;
            *dest = getc(fnt);
            dest += wbytes;
        }
        cb += wbytes * h;
    }

    return bm;
}

WinFont *
winfont_load_fnt_resource(WinFont *wf, FILE *fnt)
{
    FontDirEntry fd;
    FontDirEntry_v3_Fields extras;
    size_t nglyphs, ctsize;
    WORD fnt_base;
    CharInfo_v2 *ct2 = NULL;
    CharInfo_v3 *ct3 = NULL;
    char *facestr = NULL;
    uint8_t *bitmap = NULL;
    int w, h, wBytes, offset;

    fnt_base = ftell(fnt);
    if (fread(&fd, sizeof(FontDirEntry), 1, fnt) == 0) {
        fprintf(stderr, "Error reading font\n");
        goto cleanup;
    }

    if (fd.dfType & 1) {
        fprintf(stderr, "Not a bitmap font\n");
        goto cleanup;
    }

    fprintf(stderr, "Version: 0x%X\n", fd.dfVersion);
    fprintf(stderr, "Size: %u\n", fd.dfSize);

    /* fprintf(stderr, "f0(%lu), f1(%lu)\n",
           offsetof(FontDirEntry, dfVersion),
           offsetof(FontDirEntry, dfSize));
    */

    fprintf(stderr, "Copyright: %s\n", fd.dfCopyright);
    fprintf(stderr, "Type: 0x%X\n", fd.dfType);
    fprintf(stderr, "Pts: %d\n", fd.dfPoints);
    fprintf(stderr, "VRes: %d\n", fd.dfVertRes);
    fprintf(stderr, "HRes: %d\n", fd.dfHorizRes);
    fprintf(stderr, "Asc: %d\n", fd.dfAscent);
    fprintf(stderr, "IntL: %d\n", fd.dfInternalLeading);
    fprintf(stderr, "ExtL: %d\n", fd.dfExternalLeading);
    fprintf(stderr, "Itl: %d\n", fd.dfItalic);
    fprintf(stderr, "Und: %d\n", fd.dfUnderline);
    fprintf(stderr, "StO: %d\n", fd.dfStrikeOut);
    fprintf(stderr, "Lbs: %d\n", fd.dfWeight);
    fprintf(stderr, "ChS: %d\n", fd.dfCharSet);
    fprintf(stderr, "PixW: %d\n", fd.dfPixWidth);
    fprintf(stderr, "PixH: %d\n", fd.dfPixHeight);
    fprintf(stderr, "AvgW: %d\n", fd.dfAvgWidth);
    fprintf(stderr, "MaxW: %d\n", fd.dfMaxWidth);
    fprintf(stderr, "PnF: %d\n", fd.dfPitchAndFamily);
    fprintf(stderr, "FC: %d\n", fd.dfFirstChar);
    fprintf(stderr, "LC: %d\n", fd.dfLastChar);
    fprintf(stderr, "DC: %d\n", fd.dfDefaultChar);
    fprintf(stderr, "BC: %d\n", fd.dfBreakChar);
    fprintf(stderr, "WidthInBytes: %d\n", fd.dfWidthBytes);
    fprintf(stderr, "Device: %d\n", fd.dfDevice);
    fprintf(stderr, "FO: %d\n", fd.dfFace);
    fprintf(stderr, "BP: %d\n", fd.dfBitsPointer);
    fprintf(stderr, "BO: %d\n", fd.dfBitsOffset);

    facestr = winfont_read_string(fnt_base + fd.dfFace, fnt);
    if (facestr)
        fprintf(stderr, "Face: %s\n", facestr);

    if (fd.dfVersion == DF_VER3)
        if (fread(&extras, sizeof(extras), 1, fnt) == 0) {
            fprintf(stderr, "Expected v3 FNT fields\n");
            goto cleanup;
        }

    nglyphs = fd.dfLastChar - fd.dfFirstChar + 2;
    if (fd.dfVersion == DF_VER2) {
        ctsize = nglyphs * sizeof(CharInfo_v2);
        ct2 = malloc(ctsize);
        if (!ct2) {
            fprintf(stderr, "OOM\n");
            goto cleanup;
        }
        if (fread(ct2, ctsize, 1, fnt) == 0) {
            fprintf(stderr, "Error reading: %s\n", ".fon");
            goto cleanup;
        }
    } else {
        ctsize = nglyphs * sizeof(CharInfo_v3);
        ct3 = malloc(ctsize);
        if (!ct3) {
            fprintf(stderr, "OOM\n");
            goto cleanup;
        }
        if (fread(ct3, ctsize, 1, fnt) == 0) {
            fprintf(stderr, "Error reading: %s\n", ".fon");
            goto cleanup;
        }
    }

    offset = fnt_base + fd.dfBitsOffset;
    if (fseek(fnt, offset, SEEK_SET) == -1) {
        fprintf(stderr, "Error reading font\n");
        goto cleanup;
    }

    w = fd.dfPixWidth;
    h = fd.dfPixHeight;

    wBytes = (int)ceilf((float)w / 8.0f);
    bitmap = winfont_read_bitmap(w, h, wBytes, nglyphs, fnt);

    if (!wf) {
        wf = calloc(1, sizeof(WinFont));
        if (!wf) {
            fprintf(stderr, "OOM\n");
            goto cleanup;
        }
    }

    wf->facename = facestr;
    wf->nglyphs = nglyphs;
    wf->width = w;
    wf->height = h;
    wf->wbytes = wBytes;
    wf->charset = WinFont_CharSetCP437;
    wf->_fn_info = calloc(1, sizeof(FontDirEntry));
    if (!wf->_fn_info) {
        fprintf(stderr, "OOM\n");
        goto cleanup;
    }
    memmove(wf->_fn_info, &fd, sizeof(FontDirEntry));
    /* TODO: zero out fields that don't apply outside of the file
     * context */
    wf->bitmap = bitmap;

    return wf;

cleanup:
    if (facestr)
        free(facestr);
    if (ct2)
        free(ct2);
    if (ct3)
        free(ct3);

    return wf;
}

WinFont *
winfont_read_file(FILE *f)
{
    MZ_Header mz;
    NE_Header ne;
    ResEntry re;
    long foff, rtoff, fntoff;
    int rcount, fntcount;
    uint16_t shift;
    WinFont *wf = NULL;

    if ((foff = ftell(f)) == -1)
        return NULL;

    if (foff != 0)
        return NULL;

    if (fread(&mz, sizeof(MZ_Header), 1, f) == 0) {
        fprintf(stderr, "Error reading font\n");
        return NULL;
    }

    if (mz.e_magic != FON_MZ_MAGIC) {
        fprintf(stderr, "Invalid font\n");
        return NULL;
    }

    /* fprintf(stderr, "MZ magic=0x%X\n", mz.e_magic); */
    /* fprintf(stderr, "NE offset=%d\n", mz.e_lfanew); */

    if (fseek(f, mz.e_lfanew, SEEK_SET) == -1) {
        fprintf(stderr, "Error reading font\n");
        return NULL;
    }

    if (fread(&ne, sizeof(NE_Header), 1, f) == 0) {
        fprintf(stderr, "Error reading font\n");
        return NULL;
    }

    if (ne.ne_magic != FON_NE_MAGIC) {
        fprintf(stderr, "Invalid font\n");
        return NULL;
    }

    /* fprintf(stderr, "NE magic=0x%X\n", ne.ne_magic); */
    /* fprintf(stderr, "NE rsrctab=%d\n", ne.ne_rsrctab); */

    /* Move to the resource table. */
    rtoff = mz.e_lfanew + ne.ne_rsrctab;
    /* fprintf(stderr, "rtoff=%ld\n", rtoff); */
    if (fseek(f, rtoff, SEEK_SET) == -1) {
        fprintf(stderr, "Error reading font\n");
        return NULL;
    }

    if (fread(&shift, sizeof(shift), 1, f) == 0) {
        fprintf(stderr, "Error reading font\n");
        return NULL;
    }
    /* fprintf(stderr, "shift=%d\n", shift); */

    fntoff = 0;
    rcount = 0;
    fntcount = 0;

    for (;;) {
        if (fread(&re, sizeof(ResEntry), 1, f) == 0) {
            fprintf(stderr, "Error reading font\n");
            return NULL;
        }
        if (re.reType == 0)
            break;
        foff = ftell(f);
        if (re.reType == RT_FONT) {
            fntcount = re.reCount;
            fntoff = re.reOffset << shift;
            /* fprintf(stderr, "fntoff=%ld\n", fntoff); */
            if (fseek(f, fntoff, SEEK_SET) == -1) {
                fprintf(stderr, "Error reading resource table\n");
                return NULL;
            }
            wf = winfont_load_fnt_resource(wf, f);
            if (fntcount == ++rcount)
                break;
        }
        if (fseek(f, foff, SEEK_SET) == -1) {
            fprintf(stderr, "Error reading resource table\n");
            return NULL;
        }
    }

    if (fntcount == 0) {
        fprintf(stderr, "No FNT resources found\n");
        return NULL;
    }

    return wf;
}

WinFont *
winfont_read_path(char *path)
{
    FILE *f;
    WinFont *wf = NULL;
    (void)f;

    return wf;
}

void
winfont_free(WinFont *wf)
{

}
