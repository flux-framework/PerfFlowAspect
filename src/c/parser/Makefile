CXX := clang++
CXXFLAGS := -std=c++11
CFLAGS :=
LEX := flex
LEX_FLAGS := --nodefault
YACC := bison
YACC_FLAGS := -t --report=none -d
AR := ar
OBJS := perfflow_parser.o \
	lex.yy.o \
	parser.tab.o

all: libperfflow_parser.so

libperfflow_parser.so: $(OBJS)
	$(CXX) -shared -Wl,--version-script=libperfflow_parser.map $^ -o $@

parser.tab.c parser.tab.h: parser.y
	$(YACC) $(YACC_FLAGS) $<

lex.yy.c: lex.l parser.tab.h
	$(LEX) $(LEX_FLAGS) $<

perfflow_parser.o: perfflow_parser.cpp parser.tab.h
	$(CXX) $(CXXFLAGS) $< -c -fPIC -o $@

lex.yy.o: lex.yy.c
	$(CXX) -Wno-deprecated-register $< -c -fPIC -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) $< -c -fPIC -o $@


.PHONY: clean

clean:
	rm -f $(OBJS) lex.yy.c parser.tab.c parser.tab.h parser.output libperfflow_parser.so

