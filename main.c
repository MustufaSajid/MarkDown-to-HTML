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
 *  License: MIT (see license for full text )
 */
#include "func.h"
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
    } else if (isdigit(*pCurr) && *(pCurr + 1) == ' ') {
      char *q = pCurr;
      while (isdigit((unsigned char)*q))
        q++;
      if (*q == '.' && *(q + 1) == ' ') {
        pCurr = q + 2;
      }
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
  write_html_footer(fout);
  fclose(file);
  fclose(fout);

  return EXIT_SUCCESS;
}
