#include <stdio.h>
#include <stdlib.h>

char *fgetstr(FILE * fp, int delim)
{
    int c;
    size_t n;
    char *s;

    if (!(s = malloc(sizeof(*s)))) return NULL;

    for (n = 0; (c = fgetc(fp)) != EOF && c != delim; s[n++] = c)
        if (!(s = realloc(s, sizeof(*s) * (n + 2)))) return NULL;

    s[n] = '\0';
    return s;
}

int main(int argc, char **argv)
{
    int c;
    size_t i;
    char *vname;
    FILE *fl, *fv;        /* fl: list file; fv: vcf file */

    if (argc != 2) goto argerr;

    if (!(fl = fopen(argv[1], "r"))) goto ioerr;

    while ((vname = fgetstr(fl, '\n'))) {
        if (feof(fl)) {
            free(vname);
            break;
        }

        fv = fopen(vname, "rb");
        free(vname);
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

    fclose(fl);
    return 0;

argerr:
    fprintf(stderr, "Usage:\n\tvcfex filelist.lst\n");
    return 2;

ioerr:
    if (fl) fclose(fl);
    fprintf(stderr, "vcfex: File could not be opened!\n");
    return 3;
}
