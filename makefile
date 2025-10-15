
DIR1 = TesteHash/
DIR2 = LamportOTS/
DIR3 = WOTS/

LIBS1 = sha256.c ${DIR1}dicionario.c
LIBS2= sha256.c ${DIR2}utils.c ${DIR2}keys.c
LIBS3= sha256.c ${DIR3}utils.c ${DIR3}keys.c

all: 
	make clean
	make wots
	make lamport

wots: ${DIR3}wots.c ${LIBS3}
	gcc -o wots ${DIR3}wots.c ${LIBS3} -I.
	@echo "Wots Compilado"

lamport: ${DIR2}lamport.c ${LIBS2}
	gcc -o lamport ${DIR2}lamport.c ${LIBS2} -I.
	@echo "Lots Compilado"
teste: ${DIR1}main.c ${LIBS1}
	gcc -o testeHash ${DIR1}main.c $(LIBS1) -I.



clean: 
	@echo "Removendo o executável..."
	rm -f lamport testeHash wots

	@echo "Removendo .txts"
	rm assinatura.txt mensagem.txt publicKeys.txt
	
	@echo "Feito!"