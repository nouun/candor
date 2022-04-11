#include "config.h"
#include "repl.h"

#include <signal.h>

static void handle_shutdown(void);
static void handle_interrupt(int);

int main(int argc, char** argv) {
  candor_init();

  signal(SIGINT, handle_interrupt);
  atexit(handle_shutdown);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      cval* res = candor_load_file(argv[i]);
      if (res->type == CVAL_ERR) { cval_println(res); }
      cval_del(res);
    }

    return 0;
  }

  printf("candor v%s\n", CANDOR_VERSION);
  run_repl();

  return 0;
}

static void handle_shutdown(void) {
  candor_deinit();
  rl_uninitialize();
}

static void handle_interrupt(int sig) {
  signal(sig, SIG_IGN);

  printf("Are you sure you want to exit? [y/N] ");
  char c = getchar();
  if (c == 'y' || c == 'Y') exit(0);
  else
    signal(sig, handle_interrupt);

  getchar();
}
