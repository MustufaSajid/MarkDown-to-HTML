#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *DEFAULT_EMBEDDED_CSS =
    "\t\tbody { font-family: sans-serif; line-height: 1.6; max-width: 800px; "
    "margin: 40px auto; padding: 0 20px; }\n"
    "\t\tblockquote { border-left: 4px solid #007acc; margin: 0; padding-left: "
    "16px; color: #555; background-color: #f9f9f9; padding: 8px 16px; }\n"
    "\t\tcode { background: #f4f4f4; padding: 2px 6px; border-radius: 4px; "
    "font-family: monospace; }\n";

typedef enum {
  STATE_TEXT,
  STATE_ITALIC,
  STATE_BOLD,
  STATE_BOLD_ITALIC,
  STATE_CODE,
  STATE_LIST,
  STATE_NONE = 0,
} InlineState;

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
  int written = snprintf(out_buf, size_outbuf, "<h%d>%s</h%d>", heading_no, str,
                         heading_no);
  *ret_outsize = written;
  return written;
}
int md_to_inline(const char *str, char *out_buf, size_t size_outbuf,
                 size_t *ret_outsize) {
  InlineState curr_state = STATE_NONE;
  char *pOut = out_buf;
  const char *p = str;

  while (*p) {
    if (*p == '*' && *(p + 1) == '*' && *(p + 2) == '*') {
      const char *start = p + 3;
      char *end = strstr(start, "***");

      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        curr_state = STATE_BOLD_ITALIC;
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

          curr_state = STATE_NONE;
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
        curr_state = STATE_BOLD;
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

          curr_state = STATE_NONE;
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
        curr_state = STATE_ITALIC;
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

          curr_state = STATE_NONE;
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
        curr_state = STATE_CODE;
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

          curr_state = STATE_NONE;
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

  return (int)(pOut - out_buf);
}
int md_to_para(char *str, char *out_buf, size_t size_outbuf,
               size_t *ret_outsize) {
  char *pOut = out_buf;
  char *p = str;
  bool blockquote = false;

  if (*p == '>') {
    blockquote = true;
    memcpy(pOut, "<blockquote>", 12);
    p++;
    pOut += 12;
  } else {
    memcpy(pOut, "<p>", 3);
    pOut += 3;
  }

  size_t inline_written = 0;
  size_t remaining = size_outbuf - (size_t)(pOut - out_buf);

  md_to_inline(p, pOut, remaining, &inline_written);
  pOut += inline_written;

  if (blockquote) {
    memcpy(pOut, "</blockquote>", 13);
    pOut += 13;
  } else {
    memcpy(pOut, "</p>", 4);
    pOut += 4;
  }

  *pOut = '\0';
  if (ret_outsize) {
    *ret_outsize = (size_t)(pOut - out_buf);
  }
  return (int)(pOut - out_buf);
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
    if (ptr[1] == ' ') {
      // Unchecked box
      memcpy(pOut, "<input type=\"checkbox\"> ", 24);
      pOut += 33;
    } else {
      // Checked box
      memcpy(pOut, "<input type=\"checkbox\" checked > ", 32);
      pOut += 41;
    }
    ptr += 4; // skip [ ] [X]
  }
  size_t remaining = size_outbuf - (size_t)(pOut - out_buf);
  md_to_inline(ptr, pOut, remaining,
               &ret_size); // move past opening ul and li tag
  pOut += ret_size;        // move past the fmted paragraph
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
  md_to_inline(ptr, pOut, remaining,
               &ret_size); // move past opening ul and li tag
  pOut += ret_size;        // move past the fmted paragraph
  memcpy(pOut, "</li>\n", 6);
  pOut += 6;
  *pOut = '\0';
  if (ret_outsize) {
    *ret_outsize = (size_t)(pOut - out_buf);
  }
  return 0;
}
int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "mdtohtml <filename> <out_html filename> <title> <-css>\n");
    return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening markdown file ");
    return EXIT_FAILURE;
  }
  FILE *fout = fopen(argv[2], "w");
  if (fout == NULL) {
    fprintf(stderr, "Error opening html file ");
    return EXIT_FAILURE;
  }
  char *title = "Document";
  bool css = false;
  for (int i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--css") == 0) {
      css = true;
    } else {
      title = argv[i];
    }
  }

  build_html_header(title, css, fout);
  char buffer[2048];
  char write[2048];
  char *pCurr = NULL;
  bool is_list = false;
  bool is_ordList = false;

  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    bool state_para = false;
    bool state_heading = false;
    bool is_bold = false;
    int hash_count = 0;
    size_t fmatStrSize = 0;
    int pos = strcspn(buffer, "\r\n");
    buffer[pos] = '\0';
    if (strlen(buffer) == 0) {
      continue;
    }
    pCurr = buffer;
    while (*pCurr == '#') {
      state_heading = true;
      pCurr++;
      hash_count++;
    }
    if (state_heading && *pCurr == ' ') {
      pCurr++;
    }
    if (*pCurr == '-' && *(pCurr + 1) == ' ') {
      pCurr += 2;
      if (is_ordList) {
        fputs("</ol>", fout);
        is_ordList = false;
      }
      md_to_unordered_list(pCurr, write, sizeof(write), &fmatStrSize, &is_list);
      fwrite(write, fmatStrSize, 1, fout);
      fputc('\n', fout);
      fwrite(write, fmatStrSize, 1, stdout);
    } else if (isdigit(*pCurr) && *(pCurr + 1) == ' ') {
      pCurr += 2;
      if (is_list) {
        fputs("</ul>", fout);
        is_list = false;
      }
      md_to_ordered_list(pCurr, write, sizeof(write), &fmatStrSize,
                         &is_ordList);
      fwrite(write, fmatStrSize, 1, fout);
      fputc('\n', fout);
      fwrite(write, fmatStrSize, 1, stdout);
    } else {
      if (is_list) {
        fputs("</ul>\n", fout);
        is_list = false;
      }
      if (is_ordList) {
        fputs("</ol>\n", fout);
        is_ordList = false;
      }
      if (*pCurr == '\0') { // ignore empty
        continue;
      }
      if (state_heading) {
        md_heading_to_html(pCurr, hash_count, write, sizeof(write),
                           &fmatStrSize);
        fwrite(write, fmatStrSize, 1, fout);
        fputc('\n', fout);
        fwrite(write, fmatStrSize, 1, stdout);
        continue;
      } else {
        md_to_para(pCurr, write, sizeof(write), &fmatStrSize);
        fwrite(write, fmatStrSize, 1, fout);
        fputc('\n', fout);
        fwrite(write, fmatStrSize, 1, stdout);
        continue;
      }
    }
  }
  if (is_list) { // handling last line of file list
    fputs("</ul>", fout);
    is_list = false;
  }
  if (is_ordList) {
    fputs("</ol>", fout);
    is_ordList = false;
  }
  write_html_footer(fout);
  fclose(file);
  fclose(fout);

  return EXIT_SUCCESS;
}
