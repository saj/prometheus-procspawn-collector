#ifndef PPC_SIGNAL_H
#define PPC_SIGNAL_H

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <unistd.h>

void sigset_add(sigset_t *set, int sigs[], size_t sz);
void sigprocmask_block_by_sigset(sigset_t *set);
void sigprocmask_overwrite_by_sigset(sigset_t *set);
void sigprocmask_block(int sigs[], size_t sz);
void sigprocmask_overwrite(int sigs[], size_t sz);
void sigprocmask_zero(void);

// Dummy handler used to change our disposition toward SIG_IGN'd signals.
static void inline handle_signal_dummy(int signo) {}

#endif // ifndef PPC_SIGNAL_H
