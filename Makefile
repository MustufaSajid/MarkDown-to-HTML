CC = clang  # or gcc 
CFLAGS =-Wall -Wextra -g 
TARGET = a.exe 
all:
	$(CC) $(CFLAGS) $(wildcard *.c) -o $(TARGET)
clean:
	del /f /q  $(TARGET)
.PHONY: all clean
