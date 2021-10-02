#include "src/error.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/config.h"

const void *const PPC_OVERFLOW = (void *)"overflow";

void
vperr(int errnum, const char *fmt, va_list ap) {
  fprintf(stderr, PPC_PROG_NAME ": ");
  vfprintf(stderr, fmt, ap);

  if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
    const char *errstr = strerror(errnum);
    if (errstr == (char *)EINVAL) {
      fprintf(stderr, " unknown error: %d\n", errnum);
    } else {
      fprintf(stderr, " %s\n", errstr);
    }
  } else {
    fputc('\n', stderr);
  }
}

void
perr(int errnum, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vperr(errnum, fmt, ap);
  va_end(ap);
}

void
vdie(const char *fmt, va_list ap) {
  vperr(errno, fmt, ap);
  exit(EXIT_FAILURE);
}

void
die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vdie(fmt, ap);
  va_end(ap);
}

void
vdie_errnum(int errnum, const char *fmt, va_list ap) {
  vperr(errnum, fmt, ap);
  exit(EXIT_FAILURE);
}

void
die_errnum(int errnum, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vdie_errnum(errnum, fmt, ap);
  va_end(ap);
}
