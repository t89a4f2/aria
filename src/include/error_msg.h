#ifndef _ERROR_MSG_H
#define _ERROR_MSG_H

#include <token.h>
#include <arpch.h>

void error(
	File* srcfile,
	u64 line,
	u64 column,
	u64 char_count,
	const char* fmt,
	va_list ap);

void error_token(
        File* srcfile,
        Token* token,
        const char* fmt,
        ...);
void verror_token(
        File* srcfile,
        Token* token,
        const char* fmt,
        va_list ap);

void error_common(const char* fmt, ...);
void fatal_error_common(const char* fmt, ...);

#ifndef _ERROR_MSG_NO_ERROR_MACRO
#define error_token(token, fmt, ...) \
    self->error_state = ERROR_ERROR, \
    error_token(self->srcfile, token, fmt, ##__VA_ARGS__)
#endif

#endif /* _ERROR_MSG_H */
