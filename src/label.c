#include "src/label.h"

#include <stdlib.h>
#include <string.h>

#include "src/alloc.h"
#include "src/slice.h"

typedef struct ppc_label label;

PPC_SLICE(label);

static struct ppc_label *ppc_labelset_bsearch(ppc_labelset ls, struct ppc_label l);
static void              ppc_labelset_qsort(ppc_labelset ls);
static int               ppc_labelset_compar(const void *, const void *);

static bool isalun(int c);
static bool isalnumun(int c);

struct ppc_label
ppc_label_new(const char *restrict name, const char *restrict value) {
  return (struct ppc_label){
      .name  = sdsnew(name),
      .value = ppc_label_escape_value(value, sdsnewcap(strlen(value))),
  };
}

struct ppc_label
ppc_label_split(const char *s) {
  const char *sep = strchr(s, '=');
  if (sep == NULL) {
    return (struct ppc_label){
        .name  = sdsnew(s),
        .value = sdsempty(),
    };
  }
  const char *value = sep + 1;
  return (struct ppc_label){
      .name  = sdsnewlen(s, sep - s),
      .value = ppc_label_escape_value(value, sdsnewcap(strlen(value))),
  };
}

void
ppc_label_free(struct ppc_label l) {
  sdsfree(l.name);
  sdsfree(l.value);
}

bool
ppc_label_is_valid_name(const char *name) {
  if (name == NULL) return false;
  if (name[0] == '\0') return false;
  if (name[0] == '_') {
    if (name[1] == '\0') return true; // useless, but (apparently) valid
    if (name[1] == '_') return false; // reserved by prom
  } else if (!isalun(name[0])) {
    return false;
  }
  for (const char *c = name + 1; *c; c++) {
    if (!isalnumun(*c)) return false;
  }
  return true;
}

bool
ppc_label_is_valid_value(const char *value) {
  return true;
}

sds
ppc_label_escape_value(const char *value, sds buf) {
  for (const char *c = value; *c; c++) {
    switch (*c) {
    case '\\':
      buf = sdscatlen(buf, "\\\\", 2);
      continue;
    case '"':
      buf = sdscatlen(buf, "\\\"", 2);
      continue;
    case '\n':
      buf = sdscatlen(buf, "\\n", 2);
      continue;
    }
    buf = sdscatlen(buf, c, 1);
  }
  return buf;
}

ppc_labelset
ppc_labelset_alloc() {
  return ppc_slice_alloc_T_label(0);
}

void
ppc_labelset_free(ppc_labelset ls) {
  ppc_slice_free_T_label(ls);
}

void
ppc_labelset_free_all(ppc_labelset ls) {
  for (size_t i = 0; i < ls->len; i++) { ppc_label_free(ls->slots[i]); }
  ppc_labelset_free(ls);
}

ppc_labelset
ppc_labelset_append(ppc_labelset ls, struct ppc_label l) {
  struct ppc_label *found = ppc_labelset_bsearch(ls, l);
  if (found) {
    found->value = l.value;
    return ls;
  }
  ls = ppc_slice_append_T_label(ls, l);
  ppc_labelset_qsort(ls);
  return ls;
}

sds
ppc_labelset_format(ppc_labelset ls, sds buf) {
  sdsclear(buf);
  buf = sdscatlen(buf, "{", 1);
  for (size_t i = 0; i < ls->len; i++) {
    if (i) buf = sdscatlen(buf, ",", 1);
    buf = sdscatsds(buf, ls->slots[i].name);
    buf = sdscatlen(buf, "=\"", 2);
    buf = sdscatsds(buf, ls->slots[i].value);
    buf = sdscatlen(buf, "\"", 1);
  }
  buf = sdscatlen(buf, "}", 1);
  return buf;
}

static struct ppc_label *
ppc_labelset_bsearch(ppc_labelset ls, struct ppc_label l) {
  return (struct ppc_label *)bsearch(&l, ls->slots, ls->len, sizeof(struct ppc_label),
                                     ppc_labelset_compar);
}

static void
ppc_labelset_qsort(ppc_labelset ls) {
  qsort(ls->slots, ls->len, sizeof(struct ppc_label), ppc_labelset_compar);
}

static int
ppc_labelset_compar(const void *a, const void *b) {
  return strcmp(((struct ppc_label *)a)->name, ((struct ppc_label *)b)->name);
}

static bool
isalun(int c) {
  if (c >= 'a' && c <= 'z') return true;
  if (c == '_') return true;
  if (c >= 'A' && c <= 'Z') return true;
  return false;
}

static bool
isalnumun(int c) {
  if (c >= 'a' && c <= 'z') return true;
  if (c == '_') return true;
  if (c >= '0' && c <= '9') return true;
  if (c >= 'A' && c <= 'Z') return true;
  return false;
}
