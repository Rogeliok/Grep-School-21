#ifndef SRC_GREP_S21_GREP_H_
#define SRC_GREP_S21_GREP_H_

#define _GNU_SOURSE

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e;
  int E;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
} flags;

int s21_grep(int argc, char* argv[]);

char** Init_flags_and_patterns(int argc, char* argv[], flags* flags,
                               int* patterns_count);

void pattrern_add(char** patterns, int* patterns_count);

char** Init_files(int argc, char* argv[], flags flags, int* files_count);

char** Get_pattern(int argc, char* argv[], char** patterns, int* patterns_count,
                   flags flags);

void Work_with_files(char** files, int files_count, char** patterns,
                     int patterns_count, flags flags);

int Is_kareta(FILE* file);

int Flags_C_L(char** files, int* flag_L, int i, int coincidence,
              int files_count, flags flags);

int Flag_E(flags flags, char* file_string, char* pattern);

void file_loop(FILE* file, char** patterns, char* file_string, char* cur_file,
               flags flags, int* match, int files_count, int* flag_L, int i,
               int* print_happens_flag, int patterns_count, int* coincidence,
               int string_number);

void Flag_O(char* file_string, char** patterns, int patterns_count, flags flags,
            char* cur_file, int files_count, int string_number,
            int* coincidence);

char* string(char* file_string, int temp);

char** Flag_F_init(int argc, char* argv[], char** patterns,
                   int* patterns_count);

void Flag_F(FILE* file, char* cur_file, char* file_string, flags flags,
            int string_number, int files_count);

#endif  // SRC_GREP_S21_GREP_H_
