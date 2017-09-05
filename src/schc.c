#include <stdio.h>
#include <errno.h>

#include "lexer.h"

void usage();

int main(int argc, char *argv[]) {
  puts("Simple C Haskell Compiler");

  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  const char *input_filename = argv[1];

  FILE *input = fopen(input_filename, "r");
  if (input == NULL) {
    fprintf(stderr, "Could not open file '%s': ", input_filename);
    perror("");
    return 1;
  }

  yyin = input;
  yyout = stdout;

  int token;
  while((token = yylex()) != 0) {
    printf("%-20s %s\n", strtoken(token), yytext);
  }

  fclose(input);

  return 0;
}

void usage(const char *name) {
  fprintf(stderr, "Usage: %s <input file>\n", name);
}