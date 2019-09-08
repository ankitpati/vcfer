#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *bingetlin(FILE * fp) /* fp MUST be opened as binary */
{
    static int newl = 0;
    static FILE *oldf = NULL;
    int c;
    char *s;
    size_t n;
    fpos_t pos;

    if (!fp) return (char *) newl;

    if (oldf != fp) {
        newl = 0;
        oldf = fp;
    }

    if (!(s = malloc(sizeof(*s)))) return NULL;

    if (!newl) {
        for (
            n = 0;
            (c = getc(fp)) != '\n' && c != '\r' && !feof(fp) && !ferror(fp);
            s[n++] = c
        ) if (!(s = realloc(s, sizeof(*s) * (n + 2)))) return NULL;

        switch (c) {
        case '\n':
            newl = 1;       /* Unix newline */
            break;
        case '\r':
            fgetpos(fp, &pos);
            if (getc(fp) == '\n')
                newl = 2;   /* Windows newline */
            else {
                newl = 3;   /* Mac newline */
                fsetpos(fp, &pos);
            }
            break;
        }
    }
    else {
        for (
            n = 0;
            (c = getc(fp)) != (newl == 1 ? '\n' : '\r') && !feof(fp)
                && !ferror(fp);
            s[n++] = c
        ) if (!(s = realloc(s, sizeof(*s) * (n + 2)))) return NULL;

        if (newl == 2) getc(fp);
    }

    s[n] = '\0';
    return s;
}

int binputlin(FILE * fp, char *lin)
{
    int newl;
    newl = (int) bingetlin(NULL);  /* retrieves newline info from bingetlin */

    while (!ferror(fp)) {
        switch (*lin) {
        case '\n':
            switch (newl) {
            case 0:        /* intended fall-through */
            case 1:
                fputc(*lin++, fp);
                break;

            case 2:
                fputc('\r', fp);
                fputc(*lin++, fp);
                break;

            case 3:
                fputc('\r', fp);
                lin++;
                break;
            }
            break;

        case '\0':
            goto lbl;
            break;

        default:
            fputc(*lin++, fp);
            break;
        }
    }

lbl:
    return newl;
}

int main(int argc, char **argv)
{
    size_t i;
    char *out, *lin;
    FILE *fin, *fout;

    switch (argc) {
    case 1:
        printf("Enter input filename:\n");
        if (!(out = bingetlin(stdin))) goto alocerr;

        if (!(fin = fopen(out, "rb"))) goto ioerr;
        free(out);

        break;

    case 2:
        if (!(fin = fopen(argv[1], "rb"))) goto ioerr;
        break;

    default:
        fprintf(stderr,
                "Incorrect usage!\nCorrect usage: vcfsplit <filename>\n");
        return 1;
        break;
    }

    while (!feof(fin) && !ferror(fin)) {
        if (!(fout = fopen("vcfsplit.tmp", "wb"))) goto ioerr;

        for (i = 0; i < 3 && (lin = bingetlin(fin)); ++i) {
            if (feof(fin) || !*lin) goto eofexit;

            /* fputs(lin, fout); */
            binputlin(fout, lin);

            /* fputc('\n', fout); */
            binputlin(fout, "\n");

            if (ferror(fout)) goto ioerr;
            free(lin);
        }

        /* extract filename */
        if (!(lin = bingetlin(fin))) goto ioerr;

        /* (-3) for "FN:"; (+4) for ".vcf" */
        if (!(out = malloc(strlen(lin) - 3 + 4 + 1))) goto alocerr;

        strcpy(out, lin + 3);
        strcat(out, ".vcf");

        /* fputs(lin, fout); */
        binputlin(fout, lin);

        /* fputc('\n', fout); */
        binputlin(fout, "\n");

        free(lin);
        /* end of extract filename */

        while ((lin = bingetlin(fin)) && strcmp(lin, "END:VCARD")) {
            if (lin[0] == 'P' && lin[1] == 'H' && lin[2] == 'O' &&
                lin[3] == 'T' && lin[4] == 'O') {

                free(lin);
                while ((lin = bingetlin(fin)) && *lin) free(lin);
                if (lin) free(lin);
            }
            else {
                /* fputs(lin, fout); */
                binputlin(fout, lin);

                /* fputc('\n', fout); */
                binputlin(fout, "\n");

                free(lin);
                if (ferror(fout)) goto ioerr;
            }
        }

        if (lin) free(lin);

        /* fputs("END:VCARD\n", fout); */
        binputlin(fout, "END:VCARD\n");

        fclose(fout);
        rename("vcfsplit.tmp", out);
        free(out);
    }

    fclose(fin);
    return 0;

eofexit:
    printf("Sucessfully Split!\n");
    if (fout) {
        fclose(fout);
        remove("vcfsplit.tmp");
    }
    free(lin);
    return 0;

ioerr:
    fprintf(stderr, "File I/O Error!\n");
    return 2;

alocerr:
    fprintf(stderr, "Memory Error!\n");
    return 3;
}
