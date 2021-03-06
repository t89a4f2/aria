#define _MSG_NO_ERROR_MACRO
#include "error_msg.h"
#undef _MSG_NO_ERROR_MACRO
#include "arpch.h"
#include "util/util.h"
#include "ds/ds.h"

static char* get_line_in_file(File* srcfile, u64 line) {
	char* line_start = srcfile->contents;
	u64 line_idx = 1;
	while (line_idx != line) {
		while (*line_start != '\n') {
			if (*line_start == '\0') {
				return null;
			}
			else line_start++;
		}
		line_start++;
		line_idx++;
	}
	return line_start;
}

static void eprint_tab(void) {
	for (u64 t = 0; t < TAB_COUNT; t++) {
		fprintf(stderr, " ");
	}
}

static int last_bar_indent_offset = 7;

void msg(
        MsgType type,
        File* srcfile,
        u64 line,
        u64 column,
        u64 char_count,
        const char* fmt,
        va_list ap) {

    char* color = null;
    char* typeof_msg_str = null;
    switch (type) {
    case MSG_ERROR: color = ANSI_FRED; typeof_msg_str = "error"; break;
    case MSG_NOTE: color = ANSI_FBOLD; typeof_msg_str = "note"; break;
    default: assert(0); break;
    }

	va_list aq;
	va_copy(aq, ap);
	fprintf(
		stderr,
		ANSI_FBOLD "%s" ANSI_RESET ":"
        ANSI_FBOLD "%lu" ANSI_RESET ":"
        ANSI_FBOLD "%lu" ANSI_RESET ": %s%s"
        ANSI_RESET ": ",
		srcfile->fpath,
		line,
		column,
        color,
        typeof_msg_str);
	vfprintf(
		stderr,
		fmt,
		aq);
	fprintf(stderr, "\n");

	int bar_indent = fprintf(stderr, "%6lu | ", line) - 2;
	assert(bar_indent > 0);
    last_bar_indent_offset = bar_indent;

	char* line_start_store = get_line_in_file(srcfile, line);
	char* line_start = line_start_store;
    u64 error_char_count = 0;
    bool count_error_chars = false;

	while (*line_start != '\n' && *line_start != '\0') {
        u64 color_to_put_to = (u64)(line_start - line_start_store + 1);
        if (color_to_put_to == column) {
            fprintf(stderr, color);
            count_error_chars = true;
        }
        if (color_to_put_to == column + char_count) {
            fprintf(stderr, ANSI_RESET);
            count_error_chars = false;
        }
		if (*line_start == '\t') eprint_tab();
		else fprintf(stderr, "%c", *line_start);
		line_start++;

        if (count_error_chars) error_char_count++;
	}
	fprintf(stderr, ANSI_RESET "\n");

	for (int c = 0; c < bar_indent; c++) fprintf(stderr, " ");
	fprintf(stderr, "| ");

	char* beg_of_line = get_line_in_file(srcfile, line);
    if (column != 0) {
        for (u64 c = 0; c < column - 1; c++) {
            if (beg_of_line[c] == '\t') eprint_tab();
            else fprintf(stderr, " ");
        }
    }

	// TODO: check if this works when there are tabs in between
	// error markers
    fprintf(stderr, color);
	char* column_start = beg_of_line + column - 1;
	for (u64 c = 0; c < error_char_count; c++) {
		if (column_start[c] == '\t') fprintf(stderr, "^^^^");
		else fprintf(stderr, "^");
	}
    fprintf(stderr, ANSI_RESET);
	fprintf(stderr, "\n");

	va_end(aq);
}

void msg_token(
        MsgType type,
        Token* token,
        const char* fmt,
        ...) {
    va_list ap;
    va_start(ap, fmt);
    vmsg_token(
            type,
            token,
            fmt,
            ap);
    va_end(ap);
}

void vmsg_token(
        MsgType type,
        Token* token,
        const char* fmt,
        va_list ap) {
    va_list aq;
    va_copy(aq, ap);
    msg(
            type,
            token->srcfile,
            token->line,
            token->column,
            token->char_count,
            fmt,
            ap);
    va_end(aq);
}

void msg_data_type(
        MsgType type,
        DataType* dt,
        const char* fmt,
        ...) {
    va_list ap;
    va_start(ap, fmt);
    vmsg_data_type(
            type,
            dt,
            fmt,
            ap);
    va_end(ap);
}

void vmsg_data_type(
        MsgType type,
        DataType* dt,
        const char* fmt,
        va_list ap) {
    va_list aq;
    va_copy(aq, ap);
    msg(
            type,
            dt->identifier->srcfile,
            dt->identifier->line,
            dt->identifier->column,
            dt->identifier->char_count + dt->pointer_count,
            fmt,
            ap);
    va_end(aq);
}

void msg_expr(
        MsgType type,
        Expr* expr,
        const char* fmt,
        ...) {
    va_list ap;
    va_start(ap, fmt);
    vmsg_expr(
            type,
            expr,
            fmt,
            ap);
    va_end(ap);
}

void vmsg_expr(
        MsgType type,
        Expr* expr,
        const char* fmt,
        va_list ap) {
    va_list aq;
    va_copy(aq, ap);
    msg(
            type,
            expr->head->srcfile,
            expr->head->line,
            expr->head->column,
            (u64)(expr->tail->end - expr->head->start),
            fmt,
            ap);
    va_end(aq);
}

static void error_write_dt_to_stderr(DataType* dt) {
    fprintf(stderr, "%s", dt->identifier->lexeme);
    for (u8 p = 0; p < dt->pointer_count; p++) {
        fprintf(stderr, "%c", '*');
    }
}

static void eprint_last_err_msg_line_num_indent(void) {
	for (int c = 0; c < last_bar_indent_offset; c++) fprintf(stderr, " ");
	fprintf(stderr, "| ");
}

void error_info_expect_type(DataType* dt) {
    eprint_last_err_msg_line_num_indent();
    fprintf(stderr, ANSI_FBOLD "expect type" ANSI_RESET ": `");
    error_write_dt_to_stderr(dt);
    fprintf(stderr, "`\n");
}

void error_info_got_type(DataType* dt) {
    eprint_last_err_msg_line_num_indent();
    fprintf(stderr, ANSI_FBOLD "   got type" ANSI_RESET ": `");
    error_write_dt_to_stderr(dt);
    fprintf(stderr, "`\n");
}

void error_info_expect_u64(char* pre, u64 n) {
    eprint_last_err_msg_line_num_indent();
    fprintf(
            stderr,
            ANSI_FBOLD "expect %s" ANSI_RESET ": ",
            pre
    );
    fprintf(stderr, "%lu", n);
    fprintf(stderr, "\n");
}

void error_info_got_u64(char* pre, u64 n) {
    eprint_last_err_msg_line_num_indent();
    fprintf(
            stderr,
            ANSI_FBOLD "   got %s" ANSI_RESET ": ",
            pre
    );
    fprintf(stderr, "%lu", n);
    fprintf(stderr, "\n");
}

/* shared across source files (no unique source file path) */
static void _error_common(const char* fmt, va_list ap) {
    va_list aq;
    va_copy(aq, ap);

	fprintf(stderr, ANSI_FBOLD "aria" ANSI_RESET ": ");
	vfprintf(stderr, fmt, aq);
	fprintf(stderr, "\n");

    va_end(aq);
}

void error_common(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
    _error_common(fmt, ap);
	va_end(ap);
}

void fatal_error_common(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
    _error_common(fmt, ap);
	va_end(ap);
	exit(1);
}

