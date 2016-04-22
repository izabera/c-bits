#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <locale.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#define LEN(a) (sizeof(a) / sizeof(*(a)))

#define pdie(s) do{                                              \
    fprintf(stderr, "%s:%s:%d: ", __FILE__, __func__, __LINE__); \
    perror(s);                                                   \
    exit(1);                                                     \
}while(0)

#define die(fmt, ...) do{                                        \
    fprintf(stderr, "%s:%s:%d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, fmt, __VA_ARGS__);                           \
    exit(1);                                                     \
}while(0)

#define VECDEF(name, push, type)                                      \
typedef struct {                                                      \
    type *data;                                                       \
    size_t size;                                                      \
    size_t cap;                                                       \
} name;                                                               \
static void push(name *v, type t)                                     \
{                                                                     \
    if (v->size == v->cap) {                                          \
        v->cap = v->cap * 2 + 1;                                      \
        if (!(v->data = realloc(v->data, v->cap * sizeof(*v->data)))) \
            pdie("realloc");                                          \
    }                                                                 \
    v->data[v->size++] = t;                                           \
}

VECDEF(Wordlist, pushword, wchar_t *)
VECDEF(Charlist, pushchar, wchar_t)

static int check    (wchar_t *ws, int depth);
static int replace  (wchar_t *ws, int depth);
static int transpose(wchar_t *ws, int depth);
static int delete   (wchar_t *ws, int depth);
static int insert   (wchar_t *ws, int depth);

static long int estrtol(char *s);
static wchar_t *embstowcs(char *s);
static void readdict(void);
static void freedict(void);
static int cmp(const void *a, const void *b);
static int isword(wchar_t *ws);
static void spell(char *s);

static size_t nresults;
static int maxdepth;
static Wordlist dict, words;
static Charlist chars;
static char *dictfile = "/usr/share/dict/words";
static int (*errors[])(wchar_t *ws, int depth) = {
    transpose,
    replace,
    delete,
    insert,
};

/* return 1 to stop searching, return 0 to continue */
static int check(wchar_t *ws, int depth)
{
    size_t i;

    if (!depth && isword(ws))
        printf("%ls\n", ws);
    if (nresults && words.size == nresults)
        return 1;
    if (!depth)
        return 0;
    for (i = 0; i < LEN(errors); i++)
        if (errors[i](ws, depth - 1))
            return 1;
    return 0;
}

/* try replacing each character with the characters from the dictionary file */
static int replace(wchar_t *ws, int depth)
{
    wchar_t t, *p, *c;

    for (p = ws; *p; p++) {
        t = *p;
        for (c = chars.data; c < chars.data + chars.size; c++) {
            if (*c == t)
                continue;
            *p = *c;
            if (check(ws, depth))
                return 1;
        }
        *p = t;
    }
    return 0;
}

/* try swapping adjacent characters */
static int transpose(wchar_t *ws, int depth)
{
    wchar_t t, *p;

    for (p = ws; *p && p[1]; p++) {
        if (*p == p[1])
            continue;
        t    = *p;
        *p   = p[1];
        p[1] = t;
        if (check(ws, depth))
            return 1;
        p[1] = *p;
        *p   = t;
    }
    return 0;
}

/* try removing each character */
static int delete(wchar_t *ws, int depth)
{
    wchar_t t, *p, c = *ws, *end = ws + wcslen(ws);

    memmove(ws, ws + 1, (end - ws) * sizeof(*ws));
    for (p = ws; p < end; p++) {
        if (check(ws, depth))
            return 1;
        t  = *p;
        *p = c;
        c  = t;
    }
    return 0;
}

/* try inserting characters from file between each character */
static int insert(wchar_t *ws, int depth)
{
    wchar_t *c, *p, buf[wcslen(ws) + 2];

    wcscpy(buf + 1, ws);
    buf[0] = 1;

    for (p = buf; *p; p++) {
        for (c = chars.data; c < chars.data + chars.size; c++) {
            *p = *c;
            if (check(buf, depth))
                return 1;
        }
        *p = p[1];
    }
    return 0;
}

static long int estrtol(char *s)
{
    char *end;
    long int n = strtol(s, &end, 10);

    if (n < 1 || !*s || *end)
        die("Invalid number: %s\n", s);
    return n;
}

static wchar_t *embstowcs(char *s)
{
    wchar_t *ws;
    size_t wslen = mbstowcs(NULL, s, 0);

    if (wslen == (size_t)-1)
        pdie("mbstowcs");
    if (!(ws = malloc((wslen + 1) * sizeof(*ws))))
        pdie("malloc");
    if (mbstowcs(ws, s, wslen + 1) != wslen)
        pdie("mbstowcs");

    return ws;
}

static void readdict(void)
{
    wchar_t *ws;
    ssize_t slen;
    char *line = NULL;
    size_t n = 0;
    FILE *f = fopen(dictfile, "r");

    if (!f)
        pdie("fopen");

    while ((slen = getline(&line, &n, f)) > 0) {
        line[slen - 1] = '\0';
        ws = embstowcs(line);
        pushword(&dict, ws);
        for (; *ws; ws++)
            if (!chars.size || !wmemchr(chars.data, *ws, chars.size))
                pushchar(&chars, *ws);
    }
    if (ferror(f))
        pdie("getline");
    fclose(f);
    free(line);

    qsort(dict.data, dict.size, sizeof(*dict.data), cmp);
}

static void freedict(void)
{
    wchar_t **wsp;

    for (wsp = dict.data; wsp < dict.data + dict.size; wsp++)
        free(*wsp);
    free(dict.data);
    free(chars.data);
}

static int cmp(const void *a, const void *b)
{
    int r;

    errno = 0;
    r = wcscoll(*(wchar_t **)a, *(wchar_t **)b);
    if (errno)
        pdie("wcscoll");
    return r;
}

static int isword(wchar_t *ws)
{
    wchar_t **p;

    if (!lfind(&ws, words.data, &words.size, sizeof(*words.data), cmp) &&
        (p = bsearch(&ws, dict.data, dict.size, sizeof(*dict.data), cmp))) {
        pushword(&words, *p);
        return 1;
    }
    return 0;
}

static void spell(char *s)
{
    wchar_t *ws;
    int d;

    printf("= %s =\n", s);
    ws = embstowcs(s);
    for (d = 0; (maxdepth && d <= maxdepth) || (!maxdepth && !words.size); d++)
        check(ws, d);
    printf("\n");

    free(ws);
    free(words.data);
    words = (Wordlist){ NULL, 0, 0 };
}

int main(int argc, char **argv)
{
    int c;

    setlocale(LC_ALL, "");

    while ((c = getopt(argc, argv, ":f:m:n:")) != -1) {
        switch (c) {
        case ':': die("Option -%c requires an operand\n", optopt);
        case '?': die("Unrecognized option: -%c\n", optopt);
        case 'f': dictfile = optarg;          break;
        case 'm': maxdepth = estrtol(optarg); break;
        case 'n': nresults = estrtol(optarg); break;
        }
    }
    if (optind == argc)
        return 0;

    readdict();
    for (argv += optind; *argv; argv++)
        spell(*argv);
    freedict();

    return 0;
}
