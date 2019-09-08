#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int c, argi;
    size_t i;
    char *vname;
    FILE *fv;

    if (argc < 2) goto argerr;

    for (argi = 1; argi < argc; ++argi) {
        vname = argv[argi];

        fv = fopen(vname, "rb");
        if (!fv) goto ioerr;

        for (i = 0; i < 4; c == ';' ? ++i : 0) {
            c = fgetc(fv);
            if (feof(fv)) goto eofhand;
        }

        for (i = 0; i < 5; ++i) fgetc(fv);

        putchar('"');
        for (;;) {
            c = fgetc(fv);
            if (feof(fv)) goto eofhand;
            if (c == 0xD) break;
            putchar(c);
        }            /* Name */
        putchar('"');

        putchar(',');
        for (i = 0; i < 3; c == ';' ? ++i : 0) {
            c = fgetc(fv);
            if (feof(fv)) goto eofhand;
        }
        while (c != ':') {
            c = fgetc(fv);
            if (feof(fv)) goto eofhand;
        }

        putchar('"');
        for (;;) {
            c = fgetc(fv);
            if (feof(fv)) goto eofhand;
            if (c == 0xD) break;
            putchar(c);
        }            /* Phone Number */
        putchar('"');

        putchar('\n');

    eofhand:
        fclose(fv);
    }

    return 0;

argerr:
    fprintf(stderr, "Usage:\n\tvcfex <file>...\n");
    return 2;

ioerr:
    fprintf(stderr, "vcfex: File could not be opened!\n");
    return 3;
}
