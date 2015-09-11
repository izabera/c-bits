#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
// add trivial error checking where needed

struct {
  size_t size, count;
  const char **words;
} dictionary;

unsigned int common_prefix(const char *a, const char *b) {
  unsigned int i = 0;
  while (*a && *b && *a++ == *b++) i++;
  return i;
}

bool isknown(const char *word) {
  unsigned int max = dictionary.count - 1, min = 0, pos = max / 2, len = 0, oldlen = 0;
  /* binary search */
  while ((len = common_prefix(word, dictionary.words[pos]) < strlen(word) ||    // while it's not the same word
          strlen(word) < strlen(dictionary.words[pos]))
         && len >= oldlen                                                       // and we're making progresses
         && pos != max && pos != min ) {                                        // and there's still room for improvement
    oldlen = len;
    if (strcmp(word, dictionary.words[pos]) > 0) min = pos;
    else max = pos;
    pos = (max + min) / 2;
  }
  return !strcmp(word, dictionary.words[pos]);
}

void add_word(const char *word) {
  dictionary.words[dictionary.count++] = strdup(word);
  if (dictionary.count == dictionary.size) {
    dictionary.size *= 2;
    dictionary.words = realloc(dictionary.words, sizeof(char *) * dictionary.size);
  }
}

bool issimilar(const char *a, const char *b) {
  if (strlen(a) < strlen(b) - 1 || strlen(a) > strlen(b) + 1) return false;
  while (*a && *b && *a == *b) a++, b++;  // skip common part
  if (*a) {
    if (*b) {
      if (a[0] == b[1] && b[0] == a[1] && !strcmp(a+2, b+2)) return true;   // transposition
      if (!strcmp(a+1, b+1)) return true;                                   // one character mismatch
    }
    if (!strcmp(a+1, b)) return true;                                       // extra character
  }
  if (*b && !strcmp(a, b+1)) return true;                                   // missing character
  return false;
}

void correct(const char *word) {
  printf("= %s =\n", word);
  for (unsigned int i = 0; i < dictionary.count; i++)
    if (issimilar(word, dictionary.words[i])) puts(dictionary.words[i]);
}

int main() {
  FILE *dict = fopen("/usr/share/dict/words", "r"); // <- must be sorted!
  ssize_t read;
  size_t len = 0;
  char *line = NULL;
  dictionary.count = 0;
  dictionary.size = 1 << 16;
  dictionary.words = malloc(sizeof(char *) * dictionary.size);
  while ((read = getline(&line, &len, dict)) != -1) {
    if (read == 1) continue;
    line[read-1] = '\0';
    add_word(line);
  }
  fclose(dict);

  char *delim = "\\1234567890'|!\"£$%&/()=?^[]{}<>,;.:-_", *word;
  while ((read = getline(&line, &len, stdin)) != -1) {
    if (read == 1) continue;
    line[read-1] = '\0';
    word = strtok(line, delim);
    while (word) {
      if (strlen(word) != 0 && !isknown(word)) correct(word);
      word = strtok(NULL, delim);
    }
  }

  free(line);
  return 0;
}
