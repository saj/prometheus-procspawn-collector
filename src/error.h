#ifndef PPC_ERROR_H
#define PPC_ERROR_H

#include <stdarg.h>

extern const void *const PPC_OVERFLOW;

void vperr(int errnum, const char *fmt, va_list ap);
void perr(int errnum, const char *fmt, ...);
void vdie(const char *fmt, va_list ap);
void die(const char *fmt, ...);
void vdie_errnum(int errnum, const char *fmt, va_list ap);
void die_errnum(int errnum, const char *fmt, ...);

#endif // ifndef PPC_ERROR_H
