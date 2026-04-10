# Makefile principal do projeto SHA256 C/CPP

# Diretorios
SHA256_DIR = SHA256
LOTS_DIR = LOTS
WOTS_DIR = WOTS
MSS_DIR = MSS
TESTE_DIR = TesteHash

# Arquivo de teste libsodium
SODIUM_TEST_SRC = libsodium_test.c
SODIUM_TEST_BIN = libsodium_test

# Target padrao: compilar tudo
all: sha256 lamport wots mss sodium

# Compilar biblioteca SHA256
sha256:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C $(SHA256_DIR)
	@echo ""

# Compilar LOTS
lamport: sha256
	@echo "Compilando LOTS..."
	$(MAKE) -C $(LOTS_DIR)
	@echo ""

# Compilar WOTS
wots: sha256
	@echo "Compilando WOTS..."
	$(MAKE) -C $(WOTS_DIR)
	@echo ""

# Compilar MSS
mss: sha256
	@echo "Compilando MSS..."
	$(MAKE) -C $(MSS_DIR)
	@echo ""

# Compilar TesteHash
teste: sha256
	@echo "Compilando TesteHash..."
	$(MAKE) -C $(TESTE_DIR)
	@echo ""

# Compilar teste do libsodium
sodium:
	@echo "Compilando teste libsodium..."
	gcc -Wall -Wextra -O2 $(SODIUM_TEST_SRC) -lsodium -o $(SODIUM_TEST_BIN)
	@echo ""

# Executar testes automatizados
test: all
	@echo "Executando testes..."
	./test_algorithms.sh

# Limpar tudo
clean:
	@echo "Limpando projeto completo..."
	$(MAKE) -C $(SHA256_DIR) clean
	$(MAKE) -C $(LOTS_DIR) clean
	$(MAKE) -C $(WOTS_DIR) clean
	$(MAKE) -C $(MSS_DIR) clean
	$(MAKE) -C $(TESTE_DIR) clean
	rm -f $(SODIUM_TEST_BIN)
	@echo ""
	@echo "Removendo arquivos txt..."

	rm -f *.txt
	@echo ""
	@echo "Limpeza concluida!"

cleanSaida:
	@echo "Limpando saidas..."
	rm -f *.csv
	rm -f *.png
# Recompilar tudo do zero
rebuild: clean all

# Ajuda
help:
	@echo "Makefile - SHA256 C/CPP"
	@echo "Targets disponiveis:"
	@echo "  make           - Compila biblioteca SHA256, Lamport, WOTS e MSS"
	@echo "  make all       - Mesmo que 'make'"
	@echo "  make sha256    - Compila apenas a biblioteca SHA256"
	@echo "  make lots  	- Compila apenas LOTS"
	@echo "  make wots      - Compila apenas WOTS"
	@echo "  make mss       - Compila apenas MSS"
	@echo "  make teste     - Compila TesteHash"
	@echo "  make sodium    - Compila o teste do libsodium"
	@echo "  make test      - Compila e executa testes automatizados"
	@echo "  make clean     - Remove todos os arquivos compilados"
	@echo "  make rebuild   - Limpa e recompila tudo"
	@echo "  make help      - Mostra esta mensagem"

.PHONY: all sha256 lots wots mss teste sodium test clean rebuild help
