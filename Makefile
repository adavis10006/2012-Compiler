TARGET = parser
OBJECT = parser.output parser.tab.h parser.tab.c parser.tab.o lex.yy.c functions.o symboltable.o sem_alloc.o registers.o asm_generate.o
CC = gcc -g
LEX = flex
YACC = bison -v
YACCFLAG = -d
LIBS = -lfl

parser: parser.tab.o	symboltable.o sem_alloc.o functions.o registers.o asm_generate.o
	$(CC) -o $(TARGET) parser.tab.o symboltable.o sem_alloc.o functions.o registers.o asm_generate.o $(LIBS)

parser.tab.o: parser.tab.c lex.yy.c symboltable.c sem_alloc.c functions.c registers.c asm_generate.c
	$(CC) -c parser.tab.c

symboltable.o: symboltable.c
	$(CC) -c symboltable.c

sem_alloc.o: sem_alloc.c
	$(CC) -c sem_alloc.c

functions.o: functions.c
	$(CC) -c functions.c

registers.o: registers.c
	$(CC) -c registers.c

asm_generate.o: asm_generate.c
	$(CC) -c asm_generate.c

lex.yy.c: lexer.l
	$(LEX) lexer.l

parser.tab.c: parser.y
	$(YACC) $(YACCFLAG) parser.y

test: parser
	./$(TARGET) ./case/test1.c > ./ans/test1
	./$(TARGET) ./case/test2.c > ./ans/test2
	./$(TARGET) ./case/test3.c > ./ans/test3
	./$(TARGET) ./case/test4.c > ./ans/test4
	./$(TARGET) ./case/test5.c > ./ans/test5
	./$(TARGET) ./case/test6.c > ./ans/test6
	./$(TARGET) ./case/test7.c > ./ans/test7
	./$(TARGET) ./case/test8.c > ./ans/test8
	./$(TARGET) ./case/test9.c > ./ans/test9
	./$(TARGET) ./case/test10.c > ./ans/test10
	./$(TARGET) ./case/test11.c > ./ans/test11
	./$(TARGET) ./case/test12.c > ./ans/test12

clean:
	rm -f $(TARGET) $(OBJECT)
	rm -rf ./ans/*

