#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static struct termios orig_termios;
static int ttyfd = STDIN_FILENO;

void tty_raw() {
    struct termios raw;
    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    if (tcsetattr(ttyfd,TCSAFLUSH,&raw) < 0) {
        fprintf(stderr, "ERROR: Can't set raw mode.\n");
        exit(1);
    }
}

void tty_reset() {
    if (tcsetattr(ttyfd,TCSAFLUSH,&orig_termios) < 0) {
        fprintf(stderr, "ERROR: Failed to reset terminal (sorry!).\n");
    }
}





void echo_chr(unsigned char c) {
    if (c >= 32 && c < 127) {
        fprintf(stderr, "%c", c);
    } else {
        switch (c) {
            case '\a':
                fprintf(stderr, "\x1b[30;47m\\a\x1b[0m"); break;
            case '\b':
                fprintf(stderr, "\x1b[30;47m\\b\x1b[0m"); break;
            case 0x1b:
                fprintf(stderr, "\x1b[30;47m\\e\x1b[0m"); break;
            case '\f':
                fprintf(stderr, "\x1b[30;47m\\f\x1b[0m"); break;
            case '\n':
                fprintf(stderr, "\x1b[30;47m\\n\x1b[0m"); break;
            case '\r':
                fprintf(stderr, "\x1b[30;47m\\r\x1b[0m"); break;
            case '\t':
                fprintf(stderr, "\x1b[30;47m\\t\x1b[0m"); break;
            case '\v':
                fprintf(stderr, "\x1b[30;47m\\v\x1b[0m"); break;
            default:
                fprintf(stderr, "\x1b[30;47m%02x\x1b[0m", c);
        }
    }
    fflush(stderr);
}

void echo_buf(unsigned char* buf, int buf_size) {
    int ptr = 0;
    for (ptr = 0; ptr < buf_size; ptr++) {
        echo_chr(buf[ptr]);
    }
}
unsigned char read_hex() {
    unsigned char c1 = getc(stdin);
    unsigned char c2 = getc(stdin);
    if (c1 >= '0' && c1 <= '9') {
        c1 -= '0';
    } else if (c1 >= 'a' && c1 <= 'f') {
        c1 -= 'a';
        c1 += 0xa;
    } else {
        return 0;
    }

    if (c2 >= '0' && c2 <= '9') {
        c2 -= '0';
    } else if (c2 >= 'a' && c2 <= 'f') {
        c2 -= 'a';
        c2 += 0xa;
    } else {
        return 0;
    }

    return (c1*0x10)+(c2);
}

unsigned char* read_shell(int* count) {
    fprintf(stderr, "$ ");
    tty_reset();
    char* line;
    size_t len = 0;
    ssize_t size = getline(&line, &len, stdin);
    FILE* out = popen(line, "r");

    char c;
    int index = 0;
    int page = 1024;
    unsigned char* buf = malloc(page * sizeof(unsigned char));

    while ((c = getc(out)) != EOF) {
        buf[index] = (unsigned char) c;
        index++;
        if (index%page == 0) {
            page *= 2;
            unsigned char* tmp = realloc(buf, page);
            if (tmp != NULL) {
                buf = tmp;
            } else {
                fprintf(stderr, "WARN: out of space.\n");
                break;
            }
        }
    }
    *count = index;
    int status = pclose(out);
    fprintf(stderr, "[Exited: %d]\n", status);

    tty_raw();
    return buf;
}
unsigned char* read_backslashed(int* len) {
    unsigned char c;
    fprintf(stderr, "*\x1b[D");
    fflush(stderr);
    char k = getc(stdin);
    switch (k) {
        case 'a':
            c = '\a'; break;
        case 'b':
            c = '\b'; break;
        case 'e':
            c = 0x1b; break;
        case 'f':
            c = '\f'; break;
        case 'n':
            c = '\n'; break;
        case 'r':
            c = '\r'; break;
        case 't':
            c = '\t'; break;
        case 'v':
            c = '\v'; break;
        case '\\':
            c = '\\'; break;
        case 'x':
            fprintf(stderr, "**\x1b[2D");
            fflush(stderr);
            c = read_hex();
            fprintf(stderr, "\x1b[K"); break;
            fflush(stderr);
        case '!':
            fprintf(stderr, "\x1b[G\x1b[K");
            unsigned char* data = read_shell(len);
            fprintf(stderr, "\x1b[G\x1b[K");
            return data;
            break;
        default:
            c = k;
    }
    *len = 1;
    char* s = malloc(sizeof(unsigned char)*2);
    sprintf(s, "%c", c);
    echo_chr(c);
    return (unsigned char*)s;
}




int main(int argc, char** argv) {
    if (!isatty(ttyfd)) {
        fprintf(stderr, "ERROR: standard input is not a tty.\n");
        exit(1);
    }
    if (tcgetattr(ttyfd,&orig_termios) < 0) {
        fprintf(stderr, "ERROR: can't get tty settings.\n");
        exit(1);
    }
    if (atexit(tty_reset) != 0) {
        fprintf(stderr, "WARN: failed to set cleanup-on-exit.\n");
    }
    tty_raw();

    size_t buf_max = 1024*sizeof(unsigned char);
    unsigned char* buffer = malloc(buf_max);
    int cursor = 0;
    int buf_size = 0;
    while (1) {
        unsigned char c = getc(stdin);
        if (c == 0x3) {
            if (cursor == 0) {
                break;
            } else {
                fprintf(stderr, "\x1b[G\x1b[K");
                fflush(stderr);
                cursor = 0;
                buf_size = 0;
            }
        } else if (c == 0x4) {
            fprintf(stderr, "\n");
            fflush(stderr);
            fwrite(buffer, sizeof(unsigned char), buf_size, stdout);
            fflush(stdout);
            buf_size = 0;
            cursor = 0;
        } else if (c == 0x7f || c == 0x18) {
            if (buf_size > 0) {
                if (buffer[cursor-1] >= 32 && buffer[cursor-1] < 127) {
                    fprintf(stderr, "\x1b[D\x1b[K");
                } else {
                    fprintf(stderr, "\x1b[2D\x1b[K");
                }
                cursor--;
                buf_size--;
                fflush(stderr);
            } else {
                fprintf(stderr, "\a");
                fflush(stderr);
            }
        } else if (c == 0x0c) {
            fprintf(stderr, "\n\x1b[G\x1b[K");
            echo_buf(buffer, buf_size);
        } else {
            if (c == '\\') {
                int len = 0;
                unsigned char* s = read_backslashed(&len);
                int i = 0;
                for (i=0; i<len; i++) {
                    buffer[cursor] = s[i];
                    cursor++;
                    if (cursor >= buf_max) {
                        buf_max *= 2;
                        buffer = realloc(buffer, buf_max);
                    }
                    if (cursor > buf_size) {
                        buf_size = cursor;
                    }
                }
                free(s);
                fprintf(stderr, "\x1b[G\x1b[K");
                echo_buf(buffer, buf_size);
            } else {
                echo_chr(c);
                buffer[cursor] = c;
                cursor++;
                if (cursor >= buf_max) {
                    buf_max *= 2;
                    buffer = realloc(buffer, buf_max);
                }
                if (cursor > buf_size) {
                    buf_size = cursor;
                }
            }
        }
    }
    free(buffer);

    return 0;
}
