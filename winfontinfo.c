#include <winfont.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    fprintf(stderr, "winfont %s\n", winfont_version());
    return 0;
}
