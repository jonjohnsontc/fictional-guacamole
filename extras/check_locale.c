#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

Comparing the usage of strcmp and strcoll. strcoll should take advantage of locale
to handle UTF-8 character sorting, but running it against a few different examples
hasn't delivered the results I would've expected. It also appears to be 
non-deterministic on my WSL Debian machine

EDIT: Turns out that I wasn't properly assigning the correct pointer values as args
to both comparison functions. Now that I am, things seem to work correctly.

*/

static inline int collate(const void *ptr_a, const void *ptr_b) {
  return strcoll(*(char **)ptr_a, *(char **)ptr_b);
}

static inline int cmp(const void *ptr_a, const void *ptr_b) {
  return strcmp(*(char **)ptr_a, *(char **)ptr_b);
}

int main(void) {
  char *res = setlocale(LC_COLLATE, "en_US.UTF-8");
  if (res == NULL) {
    printf("Could not set locale\n");
  } else
    printf("locale: %s\n", res);

  char *words[6] = {"Zürich",  "Ürümqi",   "Wrocław",
                    "Oklahoma", "Pittsburgh", "Bouaké"};
  char *letters[6] = {"Z", "Ü", "W", "O", "P", "B"};
  qsort(words, 6, sizeof(char *), cmp);
  qsort(letters, 6, sizeof(char *), cmp);
  printf("strcmp order: \n");
  printf("words: ");
  for (int i = 0; i < 6; i++)
    printf("%s ", words[i]);
  printf("\n");
  printf("letters: ");
  for (int i = 0; i < 6; i++)
    printf("%s ", letters[i]);
  printf("\n");
  
  qsort(words, 6, sizeof(char *), collate);
  qsort(letters, 6, sizeof(char *), collate);
  printf("strcoll order: ");
  for (int i = 0; i < 6; i++)
    printf("%s ", words[i]);
  printf("\n");
  printf("letters: ");
  for (int i = 0; i < 6; i++)
    printf("%s ", letters[i]);
  printf("\n");

}