#ifndef PPC_ALLOC_H
#define PPC_ALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ppc_malloc  safe_malloc
#define ppc_realloc safe_realloc
#define ppc_free    free

static const char *const oom = "out of memory\n";

static inline void *
safe_malloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    write(2, oom, strlen(oom));
    abort();
  }
  return p;
}

static inline void *
safe_realloc(void *ptr, size_t size) {
  void *p = realloc(ptr, size);
  if (!p) {
    write(2, oom, strlen(oom));
    abort();
  }
  return p;
}

#endif // ifndef PPC_ALLOC_H
