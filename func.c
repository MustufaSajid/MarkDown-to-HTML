#include "func.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void build_html_header(char *title, bool css, FILE *out) {
  char buffer[2048];
  if (!css) {
    snprintf(buffer, sizeof(buffer),
             "<!DOCTYPE html>\n"
             "<html lang=\"en\">\n"
             "<head>\n"
             "\t<meta charset=\"UTF-8\">\n"
             "\t<title>%s</title>\n"
             "\t</head >\n "
             "<body>\n",
             (title != NULL && *title != '\0') ? title : "Document");
  } else {
    snprintf(buffer, sizeof(buffer),
             "<!DOCTYPE html>\n"
             "<html lang=\"en\">\n"
             "<head>\n"
             "\t<meta charset=\"UTF-8\">\n"
             "\t<title>%s</title>\n"
             "\t<style>\n%s\t</style>\n"
             "</head>\n"
             "<body>\n",
             title, DEFAULT_EMBEDDED_CSS);
  }
  fputs(buffer, out);
}
void write_html_footer(FILE *out) {
  fputs("</body>\n", out);
  fputs("</html>\n", out);
}
int md_heading_to_html(char *str, int heading_no, char *out_buf,
                       size_t size_outbuf, size_t *ret_outsize) {
  if (heading_no < 1 || heading_no > 6)
    return EXIT_FAILURE;
  int written = snprintf(out_buf, size_outbuf, "<h%d>%s</h%d>", heading_no, str,
                         heading_no);
  if (written < 0)
    return EXIT_FAILURE;

  if ((size_t)written >= size_outbuf) {
    if (ret_outsize)
      *ret_outsize = size_outbuf - 1; /* truncated */
    return EXIT_FAILURE;
  }
  if (ret_outsize)
    *ret_outsize = (size_t)written;
  return EXIT_SUCCESS;
}
int md_to_inline(const char *str, char *out_buf, size_t size_outbuf,
                 size_t *ret_outsize) {
  char *pOut = out_buf;
  const char *p = str;

  while (*p) {
    if (*p == '*' && *(p + 1) == '*' && *(p + 2) == '*') {
      const char *start = p + 3;
      char *end = strstr(start, "***");

      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        size_t space_used = pOut - out_buf;
        size_t remain = size_outbuf - space_used;

        if (remain >=
            (strlen("<b><em>") + strlen("</em></b>") + (end - start) + 1)) {
          memcpy(pOut, "<b><em>", strlen("<b><em>"));
          pOut += 7;

          while (start < end) {
            *pOut++ = *start++;
          }

          memcpy(pOut, "</em></b>", strlen("</em></b>"));
          pOut += 9;

          p = end + 3;
          continue;
        } else {
          fprintf(stderr, "Return buffer is smaller that size of string "
                          "conversion to html\n");
          return -1;
        }
      }
    } else if (*p == '*' && *(p + 1) == '*') {
      const char *start = p + 2; // bold
      char *end = strstr(start, "**");

      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        size_t space_used = pOut - out_buf;
        size_t remain = size_outbuf - space_used;

        if (remain >= (strlen("<b>") + strlen("</b>") + (end - start) + 1)) {
          memcpy(pOut, "<b>", 3);
          pOut += 3;

          while (start < end) {
            *pOut++ = *start++;
          }

          memcpy(pOut, "</b>", 4);
          pOut += 4;

          p = end + 2;
          continue;
        } else {
          fprintf(stderr, "Return buffer is smaller that size of string "
                          "conversion to html\n");
          return -1;
        }
      }
    } else if (*p == '*') { // *italic*
      const char *start = p + 1;
      char *end = strstr(start, "*");

      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        size_t space_used = pOut - out_buf;
        size_t remain = size_outbuf - space_used;

        if (remain >= (strlen("<em>") + strlen("</em>") + (end - start) + 1)) {
          memcpy(pOut, "<em>", 4);
          pOut += 4;

          while (start != end) {
            *pOut++ = *start++;
          }

          memcpy(pOut, "</em>", 5);
          pOut += 5;

          p = end + 1;
          continue;
        }

        fprintf(stderr, "Return buffer is smaller that size of string "
                        "conversion to html\n");
        return -1;
      }
    } else if (*p == '`') {
      const char *start = p + 1;
      char *end = strstr(start, "`");

      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        size_t space_used = pOut - out_buf;
        size_t remain = size_outbuf - space_used;

        if (remain >=
            (strlen("<code>") + strlen("</code>") + (end - start) + 1)) {
          memcpy(pOut, "<code>", 6);
          pOut += 6;

          while (start != end) {
            *pOut++ = *start++;
          }

          memcpy(pOut, "</code>", 7);
          pOut += 7;

          p = end + 1;
          continue;
        } else {
          fprintf(stderr, "Return buffer is smaller that size of string "
                          "conversion to html\n");
          return -1;
        }
      }
    }
    size_t space_used = pOut - out_buf;
    size_t remain = size_outbuf - space_used;
    if (remain >= 2) { // one for char and one for null terminator
      *pOut++ = *p++;  // write normal character
    } else {
      fprintf(stderr, "Space full in out buffer returning ......\n");
      return -1;
    }
  }

  *pOut = '\0';

  if (ret_outsize) {
    *ret_outsize = pOut - out_buf;
  }

  return EXIT_SUCCESS;
}
int md_to_para(char *str, char *out_buf, size_t size_outbuf,
               size_t *ret_outsize) {
  char *pOut = out_buf;
  char *p = str;
  bool blockquote = (*p == '>');
  const char *open = blockquote ? "<blockquote>" : "<p>";
  const char *close = blockquote ? "</blockquote>" : "</p>";
  size_t open_len = strlen(open);
  size_t close_len = strlen(close);
  if (size_outbuf < open_len + close_len + 1)
    return EXIT_FAILURE;
  if (blockquote) {
    p++;
    if (*p == ' ') {
      p++;
    }
  }
  memcpy(pOut, open, open_len);
  pOut += open_len;
  size_t inline_written = 0;
  size_t remaining = size_outbuf - open_len - close_len - 1;
  int ret_count = md_to_inline(p, pOut, remaining, &inline_written);
  if (ret_count < 0) {
    return -1;
  }
  pOut += inline_written;
  memcpy(pOut, close, close_len);
  pOut += close_len;

  *pOut = '\0';
  if (ret_outsize) {
    *ret_outsize = (size_t)(pOut - out_buf);
  }
  return 0;
}
int md_to_unordered_list(char *str, char *out_buf, size_t size_outbuf,
                         size_t *ret_outsize, bool *in_list) {
  char *pOut = out_buf;
  char *ptr = str;
  size_t ret_size = 0;

  if (*in_list == false) {
    memcpy(pOut, "<ul>\n", 5);
    pOut += 5;
    *in_list = true;
  }
  memcpy(pOut, "  <li>", 6);
  pOut += 6;
  if (ptr[0] == '[' && (ptr[1] == ' ' || ptr[1] == 'x' || ptr[1] == 'X') &&
      ptr[2] == ']' && ptr[3] == ' ') {
    const char *checkbox = (ptr[1] == ' ') ? CB_UNCHECKED : CB_CHECKED;
    size_t chckbx_len = strlen(checkbox);
    if (size_outbuf - (size_t)(pOut - out_buf) <
        chckbx_len + 1) // +1 for null term
      return -1;
    memcpy(pOut, checkbox, chckbx_len);
    pOut += chckbx_len; // move pout after chckbx
    ptr += 4;           // skip [ ] [X]
  }
  if (size_outbuf - (size_t)(pOut - out_buf) < 7)
    return -1;
  size_t remaining = (size_outbuf - (size_t)(pOut - out_buf) - 7);
  if (md_to_inline(ptr, pOut, remaining, &ret_size) < 0)
    return -1;
  pOut += ret_size; // move past the fmted paragraph
  memcpy(pOut, "</li>\n", 6);
  pOut += 6;
  *pOut = '\0';
  if (ret_outsize) {
    *ret_outsize = (size_t)(pOut - out_buf);
  }
  return 0;
}
int md_to_ordered_list(char *str, char *out_buf, size_t size_outbuf,
                       size_t *ret_outsize, bool *in_list) {
  char *pOut = out_buf;
  char *ptr = str;
  size_t ret_size = 0;

  if (*in_list == false) {
    memcpy(pOut, "<ol>\n", 5);
    pOut += 5;
    *in_list = true;
  }
  memcpy(pOut, "  <li>", 6);
  pOut += 6;
  size_t remaining = size_outbuf - (size_t)(pOut - out_buf);
  if (md_to_inline(ptr, pOut, remaining, &ret_size) < 0) // ← add check
    return -1;
  pOut += ret_size; // move past the fmted paragraph
  memcpy(pOut, "</li>\n", 6);
  pOut += 6;
  *pOut = '\0';
  if (ret_outsize) {
    *ret_outsize = (size_t)(pOut - out_buf);
  }
  return 0;
}
