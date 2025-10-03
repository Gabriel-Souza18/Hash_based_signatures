
DIR1 = TesteHash/
DIR2 = LamportOTS/
LIBS1 = sha256.c ${DIR1}dicionario.c
LIBS2= sha256.c ${DIR2}utils.c ${DIR2}keys.c

lamport: ${DIR2}lamport.c ${LIBS2}
	gcc -o lamport ${DIR2}lamport.c ${LIBS2} -I.

teste: ${DIR1}main.c ${LIBS1}
	gcc -o testeHash ${DIR1}main.c $(LIBS1) -I.



clean: 
	@echo "Removendo o executável..."
	rm -f lamport testeHash
	@echo "Feito!"