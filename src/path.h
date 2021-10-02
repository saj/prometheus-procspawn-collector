#ifndef PPC_PATH_H
#define PPC_PATH_H

#include "thirdparty/sds/sds.h"

// POSIX dirname(3) and basename(3) modify their input strings. :-(
sds pdirname(sds path, sds buf);
sds pbasename(sds path, sds buf);

sds tmp_file_name(sds path, sds buf);

#endif // ifndef PPC_PATH_H
