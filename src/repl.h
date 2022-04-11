#ifndef CANDOR_REPL_H
#define CANDOR_REPL_H

#include <candor.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

#define BUF_SIZE 2048

static char buffer[BUF_SIZE];

static char* readline(const char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, BUF_SIZE, stdin);
  char* cpy = malloc(BUF_SIZE);
  strcopy(cpy, buffer);
  return cpy;
}

static void add_history(const char* line) {}
static void rl_uninitialize() {}

#else
#include <editline.h>
#endif

void run_repl(void);

#endif
