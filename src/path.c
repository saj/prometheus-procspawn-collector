#include "src/path.h"

#include <libgen.h>
#include <string.h>

sds
pdirname(sds path, sds buf) {
  sds path2 = sdsdup(path);
  buf       = sdscat(buf, dirname(path2));
  sdsfree(path2);
  return buf;
}

sds
pbasename(sds path, sds buf) {
  sds path2 = sdsdup(path);
  buf       = sdscat(buf, basename(path2));
  sdsfree(path2);
  return buf;
}

sds
tmp_file_name(sds path, sds buf) {
  if (!sdslen(path)) return buf;
  sds dn = pdirname(path, sdsnewcap(sdslen(path)));
  sds bn = pbasename(path, sdsnewcap(sdslen(path)));
  if (!strcmp(dn, "/")) {
    if (!strcmp(bn, "/")) return sdscatlen(buf, "/", 1);
    buf = sdscatlen(buf, "/", 1);
    goto skipdir;
  }
  if (!strcmp(dn, ".")) {
    if (!strcmp(bn, ".")) return sdscatlen(buf, ".", 1);
    if (!strcmp(bn, "..")) return sdscatlen(buf, "..", 2);
    goto skipdir;
  }
  buf = sdscatsds(buf, dn);
  buf = sdscatlen(buf, "/", 1);
skipdir:
  if (bn[0] != '.') buf = sdscatlen(buf, ".", 1);
  buf = sdscatsds(buf, bn);
  buf = sdscatlen(buf, "~tmp", 4);
  sdsfree(dn);
  sdsfree(bn);
  return buf;
}
