NAME = programa
LIBS = sha256.c dicionario.c


copilar: main.c ${LIBS}
	gcc -o $(NAME) main.c $(LIBS) -I.

run: 
	@echo "Executando o programa..."
	make copilar
	@echo "---------------------"
	./$(NAME)

	
clean: 
	@echo "Removendo o executável..."
	rm -f $(NAME)
