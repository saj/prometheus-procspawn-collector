#include "src/signal.h"

#include <errno.h>

#include "src/error.h"

void
sigset_add(sigset_t *set, int sigs[], size_t sz) {
  for (size_t i = 0; i < sz; i++) sigaddset(set, sigs[i]);
}

void
sigprocmask_block_by_sigset(sigset_t *set) {
  errno = 0;
  if (sigprocmask(SIG_BLOCK, set, NULL)) die("sigprocmask:");
}

void
sigprocmask_overwrite_by_sigset(sigset_t *set) {
  errno = 0;
  if (sigprocmask(SIG_SETMASK, set, NULL)) die("sigprocmask:");
}

void
sigprocmask_block(int sigs[], size_t sz) {
  sigset_t mask;
  sigemptyset(&mask);
  sigset_add(&mask, sigs, sz);
  sigprocmask_block_by_sigset(&mask);
}

void
sigprocmask_overwrite(int sigs[], size_t sz) {
  sigset_t mask;
  sigemptyset(&mask);
  sigset_add(&mask, sigs, sz);
  sigprocmask_overwrite_by_sigset(&mask);
}

void
sigprocmask_zero(void) {
  sigprocmask_overwrite(NULL, 0);
}
