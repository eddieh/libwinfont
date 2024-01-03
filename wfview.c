#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <winfont.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

int sflag = 0;
int scale = 2;
char *font_path;
FILE *font;

int winw = 640;
int winh = 240;

int ccol = 32;
int crow = 8;

static void
usage()
{
    (void)fprintf(stderr, "usage: %s [-s factor] fontpath ...\n",
        getprogname());
}

static void
winfont_render_all(WinFont *wf, SDL_Renderer *renderer)
{
    int x = 0, y = 0;
    int px = 0, py = 0;
    uint8_t *gb;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    if (sflag)
        (void)scale;

    gb = wf->bitmap;
    for (int g = 0; g < wf->nglyphs; g++) {
        gb = wf->bitmap + (wf->wbytes * wf->height) * g;

        if ((g != 0) && ((g % ccol) == 0)) {
            y += wf->height;
            x = 0;
        }

        px = x;
        py = y;

        for (int i = 0; i < wf->wbytes * wf->height; i ++) {
            if (i != 0 && i % wf->wbytes == 0) {
                py++;
                px = x;
            }
            for (int b = 7; b >= 0; b--) {
                if ((1 << b) & *gb)
                    SDL_RenderDrawPoint(renderer, px, py);
                px++;
            }
            gb++;
        }

        x += wf->width;
    }
}

int
main(int argc, char **argv)
{
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Event event;
    int ch;
    WinFont *wf = NULL;

    const char *opts = "s:";
    while ((ch = getopt(argc, argv, opts)) != -1) {
        switch (ch) {
        case 's':
            if (sscanf(optarg, "%d", &scale) == 0) {
                fprintf(stderr, "expected number got %s\n", optarg);
                exit(1);
            }
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

    font_path = *argv;          /* most be a path */

    font = fopen(font_path, "rb");
    if (font == NULL) {
        fprintf(stderr, "Could not open: %s\n", font_path);
        exit(1);
    }

    wf = winfont_read_file(font);
    if (wf == NULL) {
        fprintf(stderr, "Unable to read: %s\n", font_path);
        fclose(font);
        exit(1);
    }

    /* TODO: sanity checks */

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        wf->facename,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, winw, winh, 0);

    if (!window) {
        printf("Failed to open window: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    for (;;) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        winfont_render_all(wf, renderer);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                if (wf)
                    winfont_free(wf);
                exit(0);
                break;
            default:
                break;
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }


    return 0;
}
