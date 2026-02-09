#ifndef AOM_FEATURES_BEST_CASE_H
#define AOM_FEATURES_BEST_CASE_H

#define WILLIAM_BEST_CASE_FILTER_READ 1
#define WILLIAM_BEST_CASE_FILTER_WRITE 0

void print_discard(int discard);

int read_discard();

void open_file_write(const char *fn);

void open_file_read(const char *fn);

void close_file();

#endif  // AOM_FEATURES_BEST_CASE_H
