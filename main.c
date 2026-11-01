#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum LineFmt {
  BOLD,
  ITALIC,
};

int md_heading_to_html(char *str, int heading_no, char *out_buf,
                       size_t size_outbuf, size_t *ret_outsize) {
  int written = snprintf(out_buf, size_outbuf, "<h%d>%s</h%d>", heading_no, str,
                         heading_no);
  *ret_outsize = written;
  return written;
}
int md_to_para(char *str, char *out_buf, size_t size_outbuf,
               size_t *ret_outsize) {
  char *pOut = out_buf;
  char *p = str;
  memcpy(pOut, "<p>", 3);
  pOut += 3;

  while (*p) {
    if (*p == '*') {
      if (*(p + 1) == '*') { // **bold**
        char *start = p + 2;
        char *end = strstr(start, "**");
        if (*end) {
          memcpy(pOut, "<b>", 3);
          pOut += 3; // move past <b>
          while (start < end)
            *pOut++ = *start++;
          memcpy(pOut, "</b>", 4);
          pOut += 4; // move past </b>
          *pOut = '\0';
          p = end + 2; // move past the end **
          continue;
        }
      } else { // italic tag *itali*
        memcpy(pOut, "<em>", 4);
        pOut += 4;
        char *start = p + 1;
        char *end = strstr(start, "*");
        if (*end) {
          while (start != end) {
            *pOut++ = *start++;
          }
          memcpy(pOut, "</em>", 5);
          pOut += 5;
          *pOut = '\0';
          p = end + 1;
          continue;
        }
      }
    } else if (*p == '`') {
      memcpy(pOut, "<code>", 6);
      pOut += 6;
      char *start = p + 1;
      char *end = strstr(start, "`");
      if (*end) {
        while (start != end) {
          *pOut++ = *start++;
        }
      }
      memcpy(pOut, "</code>", 7);
      pOut += 7;
      *pOut = '\0';
      p = end + 1;
    }
    *pOut++ = *p++; // write normal character
  }
  memcpy(pOut, "</p>", 4);
  pOut += 4;
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
  if (argc != 3) {
    fprintf(stderr, "mdtohtml <filename> <out html filename>\n");
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
    fwrite(write, fmatStrSize, 1, stdout);
  }

  return EXIT_SUCCESS;
}
