clean:
	rm -rf scanner lex.c
	rm -rf parser.c parser.h
	rm -rf compiler
	rm -rf quads.txt
	rm -rf target_to_bin.abc
	rm -rf avm
scanner:	
	flex --outfile=lex.c lex.l
parser:
	bison --yacc --defines --output=parser.c parser.y
compiler:
	gcc -g -o compiler lex.c parser.c
all:
	flex --outfile=lex.c lex.l
	bison --yacc --defines --output=parser.c parser.y
	gcc -g -o compiler lex.c parser.c symtable.c intermediate.c target.c 
	gcc -g -o avm decoder.c vm.c -lm

test:
	flex --outfile=lex.c lex.l
	bison --yacc --defines --output=parser.c parser.y
	gcc -g -o compiler lex.c parser.c symtable.c intermediate.c target.c 
	gcc -g -o avm decoder.c vm.c -lm
	./compiler test.txt
	./avm
val:
	flex --outfile=lex.c lex.l
	bison --yacc --defines --output=parser.c parser.y
	gcc -g -o compiler lex.c parser.c symtable.c intermediate.c target.c 
	gcc -g -o avm decoder.c vm.c -lm
	./compiler test.txt
	valgrind ./avm
