/*
 * PROGRAM : markdown-to-html Converter
 * FILE: main.c
 * Target : Windows (Mingw-w64 GCC )
 * DESCRIPTION: Parses markdown file and generates a clean html
 * DEPENDENCIES :
 *  func.h / func.c Contain function Prototype and Implementation
 * USGAE:
 *      ./mdtohtml <md file> <output html filename> <title> < --css >
 *      <title> : is optional argument
 *      <--css> : Is optional argument it just make html look better and it is
 *                hardcoded html
 * FEATURES HANDLED :
 *  Block elements : Heading(h1-h6),Blockquotes('>'),Lists(unordered and
 * ordered)
 *  Inline Parsing : Bold,Italic,Code,Block and Italic
 *  Ouput : Embed default hardcoded CSS
 *
 *  License: MIT (see License for full detail)
 */
#include "func.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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
  char *title = "Document"; // default if not title arg provided
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
  bool FencedBlock = false;

  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    bool state_heading = false;
    int hash_count = 0;
    size_t fmatStrSize = 0;
    int pos = strcspn(buffer, "\r\n");
    buffer[pos] = '\0';
    if (strlen(buffer) == 0 && FencedBlock) {
      fputc('\n', fout);
      continue;
    } else if (strlen(buffer) == 0) {
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
    if (state_heading && !FencedBlock) {
      if (is_list) {
        fputs("</ul>\n", fout);
        is_list = false;
      }
      if (is_ordList) {
        fputs("</ol>\n", fout);
        is_ordList = false;
      }
      if (md_heading_to_html(pCurr, hash_count, write, sizeof(write),
                             &fmatStrSize) != 0) {
        fclose(file);
        fclose(fout);
        return EXIT_FAILURE;
      }
      fwrite(write, fmatStrSize, 1, fout);
      fputc('\n', fout);
      fwrite(write, fmatStrSize, 1, stdout);
      continue;
    }
    if (FencedBlock) {
      if (strncmp(pCurr, "```", 3) == 0) {
        int written = snprintf(write, sizeof(write), "</code></pre>");
        fwrite(write, 1, written, fout);
        FencedBlock = false;
        continue;
      }
      // Write the current code line
      for (const char *p = buffer; *p != '\0'; p++) {
        switch (*p) {
        case '&':
          fputs("&amp;", fout);
          break;
        case '<':
          fputs("&lt;", fout);
          break;
        case '>':
          fputs("&gt;", fout);
          break;
        default:
          fputc(*p, fout);
          break;
        }
      }
      fputc('\n', fout);
      continue;
    }
    if (*pCurr == '-' && *(pCurr + 1) == ' ') {
      pCurr += 2;
      if (is_ordList) {
        fputs("</ol>\n", fout);
        is_ordList = false;
      }
      if (md_to_unordered_list(pCurr, write, sizeof(write), &fmatStrSize,
                               &is_list) != 0) {
        fclose(file);
        fclose(fout);
        return EXIT_FAILURE;
      }
      fwrite(write, fmatStrSize, 1, fout);
      fwrite(write, fmatStrSize, 1, stdout);
    } else if (isdigit(*pCurr) && *(pCurr + 1) == '.' && *(pCurr + 2) == ' ') {
      char *q = pCurr;
      while (isdigit((unsigned char)*q)) {
        q++;
      }
      if (*q == '.' && *(q + 1) == ' ') {
        pCurr = q + 2;
        if (is_list) {
          fputs("</ul>\n", fout);
          is_list = false;
        }
        if (md_to_ordered_list(pCurr, write, sizeof(write), &fmatStrSize,
                               &is_ordList) != 0) {
          fclose(file);
          fclose(fout);
          return EXIT_FAILURE;
        }
        fwrite(write, fmatStrSize, 1, fout);
        fwrite(write, fmatStrSize, 1, stdout);
      }
    } else if (strncmp(pCurr, "```", 3) == 0) {
      pCurr += 3;
      FencedBlock = true;
      // fprintf(stdout, "FENCED BLOCK DETECTED\n");
      char langType[26] = {0};
      char *space = pCurr;
      while (*space != ' ' && *space != '\0') {
        // printf("Current char: %c | ASCII: %d\n", *space, *space);
        space++;
      }
      strncpy(langType, pCurr, (space - pCurr));
      if (*space == ' ') {
        pCurr = space + 1;
      } else {
        pCurr = space;
      }
      char *end = pCurr;
      while (*end != '\0' && strncmp(end, "```", 3) != 0) {
        end++;
      }
      if (strncmp(end, "```", 3) == 0) {
        char code[1024];
        strncpy(code, pCurr, (end - pCurr));
        code[end - pCurr] = '\0';
        printf("%s\n", code);
        int written =
            snprintf(write, sizeof(write),
                     "<pre><code class=\"language-%s\"> %s </code></pre>",
                     langType, code);
        fwrite(write, written, 1, fout);
        FencedBlock = false;
        continue;
      } else {
        int written = snprintf(write, sizeof(write),
                               "<pre><code class=\"language-%s\">", langType);

        fwrite(write, 1, written, fout);

        continue;
      }
    } else {
      if (is_list) {
        fputs("</ul>\n", fout);
        is_list = false;
      }
      if (is_ordList) {
        fputs("</ol>\n", fout);
        is_ordList = false;
      }
      if (*pCurr == '\0' && FencedBlock) { // print newline in fenced bloc
        fputc('\n', fout);
        continue;
      } else if (*pCurr == '\0') { // else skip empty
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
    fputs("</ul>\n", fout);
    is_list = false;
  }
  if (is_ordList) {
    fputs("</ol>\n", fout);
    is_ordList = false;
  }
  if (FencedBlock) {
    {
      int written = snprintf(write, sizeof(write), "</code></pre>");
      fwrite(write, 1, written, fout);
      FencedBlock = false;
    }

    write_html_footer(fout);
    fclose(file);
    fclose(fout);

    return EXIT_SUCCESS;
  }
}
