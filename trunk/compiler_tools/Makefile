CFLAGS:=-Wall -Iinclude
CC:=gcc
RM:=rm -rf
BIN:=bin

.PHONY: all assembler translator parser clean
all: assembler translator parser
assembler:
	$(CC) assembler/assembler.c $(CFLAGS) -o $(BIN)/assembler
translator:
	$(CC) translator/translator.c $(CFLAGS) -o $(BIN)/translator
parser:
	$(CC) parser/parser.c $(CFLAGS) -o $(BIN)/parser

clean:
	$(RM) $(BIN)/assembler $(BIN)/translator $(BIN)/parser
