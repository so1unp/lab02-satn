/*
 * Copia un archivo en otro.
 */

#include <stdlib.h>     // exit()
#include <stdio.h>  //para fprintf, perror
#include <string.h> //para strcmp
#include <fcntl.h> // para open
#include <unistd.h> // para read/write/close


#define BUFFER_SIZE 256

static int copy_syscalls(const char *src, const char *dst);
static int copy_stdio(const char *src, const char *dst);

static void usage(const char *prog)
{
    fprintf(stderr, "Uso: %s [s|f] archivo-origen archivo-destino\n", prog);
    fprintf(stderr, "  s: usar open/read/write/close\n");
    fprintf(stderr, "  f: usar fopen/fread/fwrite/fclose\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        usage(argv[0]);
    }

    const char *mode = argv[1];
    const char *from = argv[2];
    const char *to   = argv[3];

    if (strcmp(mode, "s") != 0 && strcmp(mode, "f") != 0) {
        fprintf(stderr, "Modo inválido: '%s'\n", mode);
        usage(argv[0]);
    }

    if (strcmp(mode, "s") == 0) {
        if (copy_syscalls(from, to) < 0) {
            return EXIT_FAILURE;
        }
    } else {
        if (copy_stdio(from, to) < 0) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int copy_syscalls(const char *src, const char *dst)
{
    int in = open(src, O_RDONLY);
    if (in < 0) {
        perror("open src");
        return -1;
    }

    int out = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (out < 0) {
        perror("open dst");
        close(in);
        return -1;
    }

    char buf[BUFFER_SIZE];

    for (;;) {
        ssize_t nread = read(in, buf, sizeof buf);
        if (nread == 0) break;
        if (nread < 0) {
            perror("read");
            close(out);
            close(in);
            return -1;
        }

        size_t off = 0;
        while (off < (size_t)nread) {
            ssize_t nw = write(out, buf + off, (size_t)nread - off);
            if (nw < 0) {
                perror("write");
                close(out);
                close(in);
                return -1;
            }
            off += (size_t)nw;
        }
    }

    if (close(out) < 0) {
        perror("close destino");
        close(in);
        return -1;
    }

    if (close(in) < 0) {
        perror("close origen");
        return -1;
    }

    return 0;
}

static int copy_stdio(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (in == NULL) {
        perror("fopen src");
        return -1;
    }

    FILE *out = fopen(dst, "wbx");
    if (out == NULL) {
        perror("fopen dst");
        fclose(in);
        return -1;
    }

    char buf[BUFFER_SIZE];

    for (;;) {
        size_t nread = fread(buf, 1, sizeof buf, in);

        if (nread > 0) {
            size_t nwritten = fwrite(buf, 1, nread, out);
            if (nwritten != nread) {
                perror("fwrite");
                fclose(out);
                fclose(in);
                return -1;
            }
        }

        if (nread < sizeof buf) {
            if (ferror(in)) {
                perror("fread");
                fclose(out);
                fclose(in);
                return -1;
            }
            break;
        }
    }

    if (fclose(out) != 0) {
        perror("fclose dst");
        fclose(in);
        return -1;
    }

    if (fclose(in) != 0) {
        perror("fclose src");
        return -1;
    }

    return 0;
}

