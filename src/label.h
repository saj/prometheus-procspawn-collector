#ifndef PPC_LABEL_H
#define PPC_LABEL_H

#include <stdbool.h>

#include "thirdparty/sds/sds.h"

struct ppc_label {
  sds name;
  sds value;
};

// ppc_label_new() and ppc_label_split() return escaped values but do not
// otherwise validate their input.

struct ppc_label ppc_label_new(const char *restrict name, const char *restrict value);
struct ppc_label ppc_label_split(const char *s);
void             ppc_label_free(struct ppc_label l);

bool ppc_label_is_valid_name(const char *name);
bool ppc_label_is_valid_value(const char *value);
sds  ppc_label_escape_value(const char *value, sds buf);

typedef struct ppc_slice_T_label *ppc_labelset;

// Warning: ppc_labelset aliases sds buffers from the original ppc_label.
//
// * ppc_labelset_free() frees the outer ppc_labelset only, leaving the aliased
//   sds buffers -- within each ppc_label -- alone.
// * ppc_labelset_free_all() additionally walks all contained ppc_labels and
//   frees their sds buffers.

ppc_labelset ppc_labelset_alloc();
void         ppc_labelset_free(ppc_labelset ls);
void         ppc_labelset_free_all(ppc_labelset ls);
ppc_labelset ppc_labelset_append(ppc_labelset ls, struct ppc_label l);
sds          ppc_labelset_format(ppc_labelset ls, sds buf);

#endif // ifndef PPC_LABEL_H
