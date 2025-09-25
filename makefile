NAME = programa
LIBS = sha256.c dicionario.c


compilar: main.c ${LIBS}
	gcc -o $(NAME) main.c $(LIBS) -I.

run: 
	@echo "Executando o programa..."
	make compilar
	./$(NAME)

	
clean: 
	@echo "Removendo o executável..."
	rm -f $(NAME)
