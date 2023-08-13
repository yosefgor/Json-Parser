COMP_FLAGS=-g -Wall -Werror -fmax-errors=3 -std=c++17 -Wno-unused-function

test_obj: test_Object.cpp Object.hpp
	g++ test_Object.cpp -o $@ $(COMP_FLAGS)

all: clean
	bison -d parser.ypp
	flex scanner.lex
	g++ -o parser $(COMP_FLAGS) lex.yy.c parser.tab.cpp

clean:
	rm -f parser.tab.*pp parser lex.yy.c test_obj
.PHONY: all clean