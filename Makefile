CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison
TARGET = mypl

# Source files
SOURCES = Main.c src/ast/ast.c src/interpreter/interpreter..c lex.yy.c parser.tab.c
OBJECTS = $(SOURCES:.c=.o)

# Generated files
LEX_SOURCE = lex.yy.c
YACC_SOURCE = parser.tab.c
YACC_HEADER = parser.tab.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

lex.yy.c: src/parser/lexer.l parser.tab.h
	$(LEX) -o lex.yy.c src/parser/lexer.l

parser.tab.c parser.tab.h: src/parser/parser.y
	$(YACC) -d -o parser.tab.c src/parser/parser.y

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LEX_SOURCE) $(YACC_SOURCE) $(YACC_HEADER) $(TARGET)

.PHONY: all clean
