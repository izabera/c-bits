#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// assert() that always executes p
#ifndef NDEBUG
#define check(p) do {                                      \
  if (!(p)) {                                              \
    fprintf(stderr, "error: %s:%d\n", __func__, __LINE__); \
    exit(1);                                               \
  }                                                        \
} while (0)
#else
#define check(p) (p)
#endif

// from https://en.wikipedia.org/wiki/Damerau%E2%80%93Levenshtein_distance
#if 0
static public function damerauLevenshteinDistance(a:Array, b:Array, C:uint):uint
{
    // "infinite" distance is just the max possible distance
    var INF:uint = a.length + b.length;

    // make and initialize the character array indices
    var DA:Array = new Array(C);
    for (var k:uint = 0; k < C; ++k) DA[k]=0;

    // make the distance matrix H[-1..a.length][-1..b.length]
    var H:matrix = new matrix(a.length+2,b.length+2);

    // initialize the left and top edges of H
    H[-1][-1] = INF;
    for (var i:uint = 0; i <= a.length; ++i)
    {
        H[i][-1] = INF;
        H[i][ 0] = i;
    }
    for (var j:uint = 0; j <= b.length; ++j)
    {
        H[-1][j] = INF;
        H[ 0][j] = j;
    }

    // fill in the distance matrix H
    // look at each character in a
    for (var i:uint = 1; i <= a.length; ++i)
    {
        var DB:uint = 0;
        // look at each character in b
        for (var j:uint = 1; j <= b.length; ++j)
        {
            var i1:uint = DA[b[j-1]];
            var j1:uint = DB;
            var cost:uint;
            if (a[i-1] == b[j-1])
               {
                 cost = 0;
                 DB   = j;
               }
            else
               cost = 1;
            H[i][j] = Math.min(    H[i-1 ][j-1 ] + cost,  // substitution
                                   H[i   ][j-1 ] + 1,     // insertion
                                   H[i-1 ][j   ] + 1,     // deletion
                                   H[i1-1][j1-1] + (i-i1-1) + 1 + (j-j1-1));
        }
        DA[a[i-1]] = i;
    }
    return H[a.length][b.length];
}
#endif

typedef struct {
  const char *data;
  size_t len;
} Word;

// the actual dictionary
struct {
  size_t size, count;
  Word *words;
} dictionary, dict_by_size;

unsigned int common_prefix(const char *a, const char *b) {
  unsigned int i = 0;
  while (*a && *b && *a++ == *b++) i++;
  return i;
}

bool isknown(Word *word) {
  unsigned int max = dictionary.count - 1, min = 0, pos = max / 2, len = 0, oldlen = 0;
  /* binary search */
  while ((len = common_prefix(word->data, dictionary.words[pos].data) < word->len ||       // while it's not the same word
          word->len < dictionary.words[pos].len)
         && len >= oldlen                                                       // and we're making progresses
         && pos != max && pos != min ) {                                        // and there's still room for improvement
    oldlen = len;
    if (strcmp(word->data, dictionary.words[pos].data) > 0) min = pos;
    else max = pos;
    pos = (max + min) / 2;
  }
  return !strcmp(word->data, dictionary.words[pos].data);
}


void add_word(Word *word) {
  check(dictionary.words[dictionary.count++].data = strndup(word->data, word->len));
  dictionary.words[dictionary.count].len = word->len;
  check(dict_by_size.words[dict_by_size.count++].data = strndup(word->data, word->len));
  dictionary.words[dictionary.count].len = word->len;
  if (dictionary.count == dictionary.size) {
    dictionary.size *= 2;
    check(dictionary.words = realloc(dictionary.words, sizeof(Word) * dictionary.size));
    dict_by_size.size *= 2;
    check(dict_by_size.words = realloc(dict_by_size.words, sizeof(Word) * dict_by_size.size));
  }
}


bool issimilar(Word *a, Word *b) {
  if (a->len < b->len - 1 || a->len > b->len + 1) return false;
  while (a->data && b->data && *(a->data) == *(b->data)) a->data++, b->data++;  // skip common part
  if (*(a->data)) {
    if (*(b->data)) {
      if (a->data[0] == b->data[1] && b->data[0] == a->data[1] && !strcmp(a->data+2, b->data+2)) return true;   // transposition
      if (!strcmp(a->data+1, b->data+1)) return true;                                   // one character mismatch
    }
    if (!strcmp(a->data+1, b->data)) return true;                                       // extra character
  }
  if (*(b->data) && !strcmp(a->data, b->data+1)) return true;                                   // missing character
  return false;
}

void correct(Word *word) {
  printf("= %s =\n", word->data);
  for (unsigned int i = 0; i < dictionary.count; i++)
    if (issimilar(word, &dictionary.words[i]))
      puts(dictionary.words[i].data);
}

void loaddict() {
  FILE *dict;
  char *dictfile;
  if ((dictfile = getenv("DICT")) == NULL) dictfile = "/usr/share/dict/words";
  check(dict = fopen(dictfile, "r")); // <- must be sorted!
  ssize_t read;
  size_t len = 0;
  char *line = NULL;
  dictionary.count = 0;
  dictionary.size = 1 << 16;
  Word tmp;
  check(dictionary.words = malloc(sizeof(Word) * dictionary.size));
  check(dict_by_size.words = malloc(sizeof(Word) * dict_by_size.size));
  while ((read = getline(&line, &len, dict)) != -1) {
    if (read == 1) continue;
    line[tmp.len = read - 1] = '\0';
    tmp.data = line;
    add_word(&tmp);
  }
  fclose(dict);
  free(line);
}

int main() {
  loaddict();

  for (unsigned int i = 0; i < dictionary.count; i++) puts(dictionary.words[i].data);

#if 0
  char *delim = " !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}";
  ssize_t read;
  size_t len = 0;
  char *line = NULL;
  Word tmp;
  while ((read = getline(&line, &len, stdin)) != -1) {
    if (read == 1) continue;
    line[read-1] = '\0';
    tmp.data = strtok(line, delim);
    while (tmp.data) {
      tmp.len = strlen(tmp.data);
      if (!isknown(&tmp)) puts(tmp.data);
      /*if (!isknown(tmp)) correct(tmp);*/
      tmp.data = strtok(NULL, delim);
    }
  }

  free(line);
#endif
  return 0;
}
