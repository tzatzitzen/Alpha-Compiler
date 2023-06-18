all: scanner parser comp avm

test: scanner parser comp avm
	./comp test.txt
	./avm

val: scanner parser comp avm
	./comp test.txt
	valgrind ./avm

scanner:	
	flex --outfile=compiler/lex.c compiler/lex.l

parser:
	bison --yacc --defines --output=compiler/parser.c compiler/parser.y

comp:
	gcc -g -o comp compiler/lex.c compiler/parser.c compiler/symtable.c compiler/intermediate.c compiler/target.c

avm:
	gcc -g -o avm virtual_machine/decoder.c virtual_machine/vm.c -lm

clean:
	rm -rf compiler/scanner lex.c
	rm -rf compiler/parser.c parser.h
	rm -rf comp
	rm -rf compiler/quads.txt
	rm -rf virtual_machine/target_to_bin.abc
	rm -rf avm
	rm -rf quads.txt
