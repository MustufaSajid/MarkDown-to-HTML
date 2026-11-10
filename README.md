# mdtohtml 

A Markdown to html converted written in C 
'mdtohtml' Process markdown text from a file and convert it into clean HTML 

## Features 
- **Block Elements:** Headings (`#` to `######`), Blockquotes (`>`), Unordered Lists (`-`, `*`), and Ordered Lists (`1.`, `2.`).
- **Inline Formatting:** Bold (`**`), Italic (`*`), Bold-Italic (`***`), and Inline Code (`` `code` ``).
- **Styling:** Optional embedded CSS injection via command-line flags.
- **Buffer Safety:** Strict boundary checks to prevent buffer overflows during parsing and string operations.


## Build

Build the program using GCC or Clang:
```bash
gcc -Wall -Wextra main.c -o mdtohtml
```
Or, if using a Makefile, simply run:
```bash
make
```
## Usage

Run the program using `./mdtohtml`.

To read Markdown from standard input and write HTML to standard output, use
```bash
./mdtohtml --css < input.md > output.html
```

## Scope & Limitations
This is a single pass md to html converter
### Supported Features 
- [x] Single level blockquotes,Heading and lists 
- [x] Inline formatting (**bold**, *italic*, `code`)
- [x] Optional embedded CSS (`--css` flag )


## Platform Compatiblity 
It is tested primarly on ***WINDOWS*** using GCC(Mingw-w64) via Command prompt and make 
> **Note:**  
> The included Makefile uses `del /f /q` for the clean operation. If compiling on Linux or macOS, replace `del /f /q` with `rm -f` in the Makefile.
## License 
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.