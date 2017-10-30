scc: lex.yy.c y.tab.c y.tab.h predefine.h tree.c main.c block.h block.c symboltable.h symboltable.c helper.c tree.h typecheck.c typecheck.h codegen.h codegen.c
	gcc lex.yy.c y.tab.c y.tab.h predefine.h tree.c main.c block.h block.c symboltable.h symboltable.c helper.c tree.h typecheck.c typecheck.h codegen.h codegen.c -o scc

lex.yy.c: scan.l
	flex scan.l

y.tab.c y.tab.h: parser.y
	yacc -d parser.y

clean:
	-rm lex.* y.*
