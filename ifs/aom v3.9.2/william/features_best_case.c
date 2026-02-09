#include "features_best_case.h"
#include <stdio.h>
#include <string.h>

static char filename[200];
FILE *file;

void print_discard(int discard) {
  fprintf(file, "%d", discard);
}

int read_discard() {
  int c = fgetc(file);
  if (c == '0') return 0;
  if (c == '1') return 1;
  return -1;
}

void open_file_write(const char *fn) {
  strcpy(filename, fn);
  strcat(filename, ".filter_log.txt");

  file = fopen(filename, "w");
}

void open_file_read(const char *fn) {
  strcpy(filename, fn);
  strcat(filename, ".filter_log.txt");

  file = fopen(filename, "r");
}

void close_file() {
  fclose(file);
}
