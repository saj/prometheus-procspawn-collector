// Simple typed dynamic arrays.

#ifndef PPC_SLICE_H
#define PPC_SLICE_H

#include <stdlib.h>
#include <string.h>

#include "src/alloc.h"
#include "src/error.h"

#define PPC_SLICE_MINCAP 8

#define PPC_SLICE_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PPC_SLICE_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define PPC_SLICE_TYPES(T) \
  struct ppc_slice_T_##T { \
    size_t cap;            \
    size_t len;            \
    T      slots[];        \
  };                       \
  typedef struct ppc_slice_T_##T *ppc_slice_T_##T;

#define PPC_SLICE_FREE(T) \
  void ppc_slice_free_T_##T(ppc_slice_T_##T s) { ppc_free(s); }

#define PPC_SLICE_ALLOC(T)                                                              \
  ppc_slice_T_##T ppc_slice_alloc_T_##T(size_t n) {                                     \
    size_t          cap = PPC_SLICE_MAX(n, PPC_SLICE_MINCAP);                           \
    ppc_slice_T_##T p   = ppc_malloc(sizeof(struct ppc_slice_T_##T) + sizeof(T) * cap); \
    if (p == NULL) return NULL;                                                         \
    p->cap = cap;                                                                       \
    p->len = 0;                                                                         \
    return p;                                                                           \
  }

#define PPC_SLICE_GROW(T)                                   \
  ppc_slice_T_##T ppc_slice_grow_T_##T(ppc_slice_T_##T s) { \
    size_t oldcap = s->cap;                                 \
    size_t newcap = oldcap * 2;                             \
    if (newcap < oldcap) { /* overflow */                   \
      newcap = SIZE_MAX;                                    \
      if (newcap == oldcap) return NULL;                    \
    }                                                       \
    ppc_slice_T_##T news = ppc_slice_alloc_T_##T(newcap);   \
    if (news == NULL) return NULL;                          \
    size_t oldlen = s->len;                                 \
    if (oldlen > 0) {                                       \
      memcpy(news->slots, s->slots, sizeof(T) * oldlen);    \
      news->len = oldlen;                                   \
    }                                                       \
    ppc_slice_free_T_##T(s);                                \
    return news;                                            \
  }

#define PPC_SLICE_APPEND(T)                                        \
  ppc_slice_T_##T ppc_slice_append_T_##T(ppc_slice_T_##T s, T v) { \
    if (s->cap - s->len == 0) s = ppc_slice_grow_T_##T(s);         \
    if (s == NULL) return NULL;                                    \
    memcpy(&s->slots[s->len++], &v, sizeof(T));                    \
    return s;                                                      \
  }

#define PPC_SLICE(T)  \
  PPC_SLICE_TYPES(T); \
  PPC_SLICE_FREE(T);  \
  PPC_SLICE_ALLOC(T); \
  PPC_SLICE_GROW(T);  \
  PPC_SLICE_APPEND(T);

#endif // ifndef PPC_SLICE_H
