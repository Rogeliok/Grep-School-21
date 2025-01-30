#include "s21_grep.h"

int s21_grep(int argc, char* argv[]) {
  if (argc == 1) {
    fprintf(stderr,
            "usage: grep [-abcDEFGHhiiJLlmnOoqRSsUVvwxZ] [-A num] [-B num] "
            "[-C[num]]\n"
            "\t[-e pattern] [-f file] [--binary-files=value] [--color=when]\n"
            "\t[--context[=num]] [--directories=action] [--label] "
            "[--line-buffered]\n"
            "\t[--null] [pattern] [file ...]\n");
  } else {
    flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int patterns_count = 0, files_count = 0;
    char** patterns =
        Init_flags_and_patterns(argc, argv, &flags, &patterns_count);
    char** files = Init_files(argc, argv, flags, &files_count);
    Work_with_files(files, files_count, patterns, patterns_count, flags);
    int i = 0;
    for (i = 0; i < patterns_count; i++) free(patterns[i]);
    free(patterns);
    for (i = 0; i < files_count; i++) free(files[i]);
    free(files);
  }
  return 0;
}

char** Init_flags_and_patterns(int argc, char* argv[], flags* flags,
                               int* patterns_count) {
  char** patterns = NULL;
  int parser, longindex = 0;
  int patterns_count_old = 0;
  struct option const* long_options = {0};
  while ((parser = getopt_long(argc, argv, "e:ivclnhsf:o", long_options,
                               &longindex)) != -1) {
    switch (parser) {
      case 'e': {
        flags->e = 1;
        flags->E = REG_EXTENDED;
        (*patterns_count)++;
      } break;
      case 'i': {
        flags->i = REG_ICASE;
      } break;
      case 'v':
        flags->v = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      case 'h':
        flags->h = 1;
        break;
      case 's':
        flags->s = 1;
        break;
      case 'f':
        flags->f = 1;
        break;
      case 'o':
        flags->o = 1;
        break;
      default:
        fprintf(
            stderr,
            "usage: grep [-abcDEFGHhiiJLlmnOoqRSsUVvwxZ] [-A num] [-B num] "
            "[-C[num]]\n"
            "\t[-e pattern] [-f file] [--binary-files=value] [--color=when]\n"
            "\t[--context[=num]] [--directories=action] [--label] "
            "[--line-buffered]\n"
            "\t[--null] [pattern] [file ...]\n");
        break;
    }
    if (*patterns_count == patterns_count_old + 1) {
      int lenght_pattern = strlen(optarg);
      patterns = realloc(patterns, (*patterns_count + 1) * sizeof(char*));
      if (patterns != NULL) {
        patterns[patterns_count_old] =
            (char*)calloc(lenght_pattern + 1, sizeof(char));
        if (patterns != NULL) {
          strcpy(patterns[patterns_count_old], optarg);
          patterns_count_old++;
        }
      }
    }
  }
  if (argc > 2)
    patterns = Get_pattern(argc, argv, patterns, patterns_count, *flags);
  return patterns;
}

char** Get_pattern(int argc, char* argv[], char** patterns, int* patterns_count,
                   flags flags) {  // распознавание шаблона если нет флага е
  if (!flags.e && !flags.f) {
    flags.E = REG_EXTENDED;
    int lenght_pattern = strlen(argv[optind]);
    patterns = realloc(patterns, (*patterns_count + 1) * sizeof(char*));
    if (patterns != NULL) {
      patterns[*patterns_count] =
          (char*)calloc(lenght_pattern + 1, sizeof(char));
      if (patterns != NULL) {
        strcpy(patterns[*patterns_count], argv[optind]);
        (*patterns_count)++;
      }
    }
  }
  return (flags.f) ? Flag_F_init(argc, argv, patterns, patterns_count)
                   : patterns;
}

char** Flag_F_init(int argc, char* argv[], char** patterns,
                   int* patterns_count) {
  int i = 0;
  FILE* patterns_file = NULL;
  char* pattern_string = '\0';
  size_t lenght = 0;
  while (i < argc) {
    if (argv[i][0] == '-' && argv[i][1] == 'f') {
      patterns_file = fopen(argv[i + 1], "r");
      if (patterns_file == NULL) {
        fprintf(stderr, "grep: %s No such file or directory\n", argv[i + 1]);
      } else {
        while (getline(&pattern_string, &lenght, patterns_file) > 0) {
          patterns = realloc(patterns, (*patterns_count + 1) * sizeof(char*));
          if (patterns != NULL) {
            patterns[*patterns_count] = (char*)calloc(lenght, sizeof(char));
            if (patterns != NULL) {
              if (pattern_string[strlen(pattern_string) - 1] == '\n' &&
                  strlen(pattern_string) > 2)
                pattern_string[strlen(pattern_string) - 1] = '\0';
              strcpy(patterns[*patterns_count], pattern_string);
              (*patterns_count)++;
            }
          }
        }
        fclose(patterns_file);
      }
    }
    i++;
  }
  free(pattern_string);
  return patterns;
}

char** Init_files(int argc, char* argv[], flags flags, int* files_count) {
  char** files = NULL;
  int i = (flags.e) ? optind : optind + 1, j = 0;
  if (argc > 3 && flags.f)
    if (argv[i - 3][1] == 'f' && argv[i - 3][0] == '-') i = optind;
  while (i < argc) {
    files = realloc(files, (*files_count + 1) * sizeof(char*));
    if (files != NULL) {
      files[*files_count] = (char*)calloc(strlen(argv[i++]) + 1, sizeof(char));
      if (files != NULL) {
        for (j = 0; j < (int)strlen(argv[i - 1]); j++)
          files[*files_count][j] = argv[i - 1][j];
        (*files_count)++;
      }
    }
  }

  return files;
}

void Work_with_files(char** files, int files_count, char** patterns,
                     int patterns_count, flags flags) {
  FILE* file = NULL;
  int i = 0, string_number = 0, print_happens_flag = 1, coincidence = 0,
      match = 0;
  int* flag_L = (int*)calloc(files_count, sizeof(int));
  char* file_string = '\0';
  size_t lenght = 0;
  while (i < files_count) {
    if ((file = fopen(files[i++], "r")) == NULL && !flags.s)
      fprintf(stderr, "grep: %s No such file or directory\n", files[i - 1]);
    string_number = 1;
    if (file != NULL) {
      while (getline(&file_string, &lenght, file) > 0) {
        print_happens_flag = 1;
        file_loop(file, patterns, file_string, files[i - 1], flags, &match,
                  files_count, flag_L, i, &print_happens_flag, patterns_count,
                  &coincidence, string_number);
        match = 0;
        if (!flags.c && !flags.l && flags.o && !flags.v)
          Flag_O(file_string, patterns, patterns_count, flags, files[i - 1],
                 files_count, string_number, &coincidence);
        string_number++;
      }
      string_number = 0;
      if (flags.v) print_happens_flag = 0;
      if (Is_kareta(file) && print_happens_flag == 0 && !flags.l && !flags.c)
        printf("\n");
      Flags_C_L(files, flag_L, i - 1, coincidence, files_count, flags);
      coincidence = 0;  // для флага с
      fclose(file);
    }
  }
  free(flag_L);
  free(file_string);
}

void file_loop(FILE* file, char** patterns, char* file_string, char* cur_file,
               flags flags, int* match, int files_count, int* flag_L, int i,
               int* print_happens_flag, int patterns_count, int* coincidence,
               int string_number) {
  int j = 0;
  regex_t regex = {0};
  regmatch_t pmatch[1] = {0};
  while (j < patterns_count && *print_happens_flag) {
    if (regcomp(&regex, patterns[j++], flags.i) != 0) {
      fprintf(stderr, "error regix\n");
      fclose(file);
      regfree(&regex);
      exit(1);
    }
    if (regexec(&regex, file_string, 1, pmatch, 0) == 0 && !flags.v) {
      if (Flag_E(flags, file_string, patterns[j - 1])) {
        (*coincidence)++;
        if (!flags.h && files_count != 1 && !flags.c && !flags.l && !flags.o)
          printf("%s:", cur_file);
        if (!flags.c && !flags.l && !flags.o)
          (flags.n) ? printf("%d:%s", string_number, file_string)
                    : printf("%s", file_string);
        *print_happens_flag = 0;
      }
      flag_L[i - 1] = 1;
    } else if (regexec(&regex, file_string, 1, pmatch, 0) == REG_NOMATCH &&
               flags.v) {
      (*match)++;
      if (*match == patterns_count) {
        (*coincidence)++;
        Flag_F(file, cur_file, file_string, flags, string_number, files_count);
        flag_L[i - 1] = 1;
      }
    }
    regfree(&regex);
  }
}

void Flag_F(FILE* file, char* cur_file, char* file_string, flags flags,
            int string_number, int files_count) {
  long int pos = 0;
  pos = ftell(file);
  if (file_string[strlen(file_string) - 1] == '\n' ||
      (file_string[strlen(file_string) - 1] != '\n' && fgetc(file) == EOF)) {
    fseek(file, pos, SEEK_SET);
    if (!flags.h && files_count != 1 && !flags.c && !flags.l)
      printf("%s:", cur_file);
    if (!flags.c && !flags.l)
      (flags.n) ? printf("%d:%s", string_number, file_string)
                : printf("%s", file_string);
  }
}

int Is_kareta(FILE* file) {  // рассмотреть случай пустого файла
  char ch = '\0', ch_next = '\0';
  int flag = 0;
  fseek(file, 0, SEEK_SET);
  while ((ch_next = fgetc(file)) != EOF) ch = ch_next;
  flag = (ch == '\n') ? 0 : 1;
  return flag;
}

void Flag_O(char* file_string, char** patterns, int patterns_count, flags flags,
            char* cur_file, int files_count, int string_number,
            int* coincidence) {
  int i = 0, j = 0, print_happens_flag = 1;
  regex_t regex = {0};
  regmatch_t pmatch[1] = {0};
  int temp = 0;
  char* str = '\0';
  while (i < patterns_count) {  // перебор паттернов
    regcomp(&regex, patterns[i++], flags.i);
    str = string(file_string, temp);
    if (regexec(&regex, str, 1, pmatch, 0) == 0 &&
        Flag_E(flags, str, patterns[i - 1])) {
      (*coincidence)++;
      if (!flags.h && files_count != 1 && !flags.c && !flags.l &&
          print_happens_flag)
        printf("%s:", cur_file);
      if (flags.n && print_happens_flag) printf("%d:", string_number);
      print_happens_flag = 0;
      if (!flags.c && !flags.l) {
        while (regexec(&regex, str, 1, pmatch, 0) == 0) {
          for (j = pmatch[0].rm_so; j < pmatch[0].rm_eo; j++)
            printf("%c", str[j]);
          if (str[pmatch[0].rm_eo] != '\0') printf("\n");
          temp += pmatch[0].rm_eo;
          free(str);
          str = string(file_string, temp);
        }
      }
      free(str);
    } else
      free(str);
    regfree(&regex);
  }
}

char* string(char* file_string, int temp) {
  int i = 0;
  char* str = (char*)calloc((int)strlen(file_string) - temp + 1, sizeof(char));
  for (i = 0; i < (int)strlen(file_string) - temp; i++)
    str[i] = file_string[i + temp];
  return str;
}

int Flag_E(flags flags, char* file_string, char* pattern) {
  int temp = 1;
  if (flags.E && pattern[0] == '.' && file_string[0] == '\n') temp = 0;
  return temp;
}

int Flags_C_L(char** files, int* flag_L, int i, int coincidence,
              int files_count, flags flags) {
  if (flags.c && flags.l) {
    if (flags.h || files_count == 1)
      printf("%d\n", (flag_L[i] == 1) ? 1 : 0);
    else
      printf("%s:%d\n", files[i], (flag_L[i] == 1) ? 1 : 0);
  } else if (flags.c) {
    if (flags.h || files_count == 1)
      printf("%d\n", coincidence);
    else
      printf("%s:%d\n", files[i], coincidence);
  }
  if (flags.l && flag_L[i] == 1) printf("%s\n", files[i]);
  return 0;
}

int main(int argc, char* argv[]) { return s21_grep(argc, argv); }
