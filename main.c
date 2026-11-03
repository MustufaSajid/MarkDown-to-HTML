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
int md_to_para(char *str, char *out_buf, size_t size_outbuf,
               size_t *ret_outsize) {
  InlineState curr_state = STATE_NONE;
  bool blockquote = false;
  char *pOut = out_buf;
  char *p = str;
  if (*p == '>') {
    blockquote = true;
    memcpy(pOut, "<blockquote>", 12);
    p++;
    pOut += 12;
  } else {
    memcpy(pOut, "<p>", 3);
    pOut += 3;
  }

  while (*p) {
    if (*p == '*') {
      if (*(p + 1) == '*') { // **bold**
        if (*(p + 2) == '*') {
          char *start = p + 3;
          char *end = strstr(start, "***");
          if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
            curr_state = STATE_BOLD_ITALIC;
            memcpy(pOut, "<b><em>", 7);
            pOut += 7;
            while (start < end) {
              *pOut++ = *start++;
            }
            memcpy(pOut, "</em></b>", 9);
            pOut += 9;
            curr_state = STATE_NONE;
            p = end + 3;
            continue;
          }
        } else {
          char *start = p + 2;
          char *end = strstr(start, "**");
          if (end != NULL && *start != ' ' &&
              *(end - 1) !=
                  ' ') { // if space at start ** text ** treat plain text

            curr_state = STATE_BOLD;
            memcpy(pOut, "<b>", 3);
            pOut += 3; // move past <b>
            while (start < end)
              *pOut++ = *start++;
            memcpy(pOut, "</b>", 4);
            pOut += 4; // move past </b>
            curr_state = STATE_NONE;
            p = end + 2; // move past the end **
            continue;
          }
        }
      } else { // italic tag *italic*
        char *start = p + 1;
        char *end = strstr(start, "*");
        if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
          curr_state = STATE_ITALIC;
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
      }
    } else if (*p == '`') {
      curr_state = STATE_CODE;
      char *start = p + 1;
      char *end = strstr(start, "`");
      if (end != NULL && *start != ' ' && *(end - 1) != ' ') {
        curr_state = STATE_CODE;
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
      }
    }
    *pOut++ = *p++; // write normal character
  }
  if (blockquote) {
    memcpy(pOut, "</blockquote>", 13);
    pOut += 13;
  } else {
    memcpy(pOut, "</p>", 4);
    pOut += 4;
  }
  *pOut = '\0';
  *ret_outsize = pOut - out_buf;
  return 0;
}
int md_to_Linefmt(char *str, char *out_buf, size_t size_outbuf,
                  size_t *ret_outsize, char *typeofFmt) {
  int written = snprintf(out_buf, size_outbuf, "<p>%s</p>", str);
  *ret_outsize = written;
  return written;
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
    // Handling empty line
    if (state_heading) {
      md_heading_to_html(pCurr, hash_count, write, sizeof(write), &fmatStrSize);
    } else {
      md_to_para(pCurr, write, sizeof(write), &fmatStrSize);
    }

    fwrite(write, fmatStrSize, 1, fout);
    fputc('\n', fout);
    fwrite(write, fmatStrSize, 1, stdout);
  }
  write_html_footer(fout);
  fclose(file);
  fclose(fout);

  return EXIT_SUCCESS;
}
