#ifndef __FUNC__H__
#define __FUNC__H__

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_EMBEDDED_CSS                                                   \
  "\tbody { font-family: sans-serif; line-height: 1.6; max-width: 800px; "     \
  "margin: 40px auto; padding: 0 20px; }\n"                                    \
  "\tblockquote { border-left: 4px solid #007acc; margin: 0; padding-left: "   \
  "16px; color: #555; background-color: #f9f9f9; padding: 8px 16px; }\n"       \
  "\tcode { background: #f4f4f4; padding: 2px 6px; border-radius: 4px; "       \
  "font-family: monospace; }\n"

static const char *CB_UNCHECKED = "<input type=\"checkbox\"> ";
static const char *CB_CHECKED = "<input type=\"checkbox\" checked> ";

typedef enum {
  STATE_TEXT,
  STATE_ITALIC,
  STATE_BOLD,
  STATE_BOLD_ITALIC,
  STATE_CODE,
  STATE_LIST,
  STATE_NONE = 0,
} InlineState;

// FUNCTION DEFINATIONS
void build_html_header(char *title, bool css, FILE *out);
void write_html_footer(FILE *out);
int md_heading_to_html(char *str, int heading_no, char *out_buf,
                       size_t size_outbuf, size_t *ret_outsize);

int md_to_inline(const char *str, char *out_buf, size_t size_outbuf,
                 size_t *ret_outsize);
int md_to_inline(const char *str, char *out_buf, size_t size_outbuf,
                 size_t *ret_outsize);
int md_to_para(char *str, char *out_buf, size_t size_outbuf,
               size_t *ret_outsize);
int md_to_unordered_list(char *str, char *out_buf, size_t size_outbuf,
                         size_t *ret_outsize, bool *in_list);
int md_to_ordered_list(char *str, char *out_buf, size_t size_outbuf,
                       size_t *ret_outsize, bool *in_list);

#endif // !__FUNC__H__
