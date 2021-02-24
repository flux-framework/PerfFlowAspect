CC := gcc
CFALGS := 
LEX := flex
LEX_FLAGS := --nodefault
YACC := bison
YACC_FLAGS := -t --report=none -d
AR := ar
OBJS := parser.tab.o \
	lex.yy.o \
	perfflow_parser.o
      
all: libperfflow_parser.so

parser.tab.c parser.tab.h: parser.y
	$(YACC) $(YACC_FLAGS) $<

lex.yy.c: lex.l parser.tab.h
	$(LEX) $(LEX_FLAGS) $<

%.o: %.c
	$(CC) $^ -c -fPIC -o $@

libperfflow_parser.so: $(OBJS)
	gcc -shared -Wl,--version-script=libperfflow_parser.map $^ -o $@

.PHONY: clean

clean:
	rm -f $(OBJS) lex.yy.c parser.tab.c parser.tab.h parser.output libperfflow_parser.so

