# Makefile principal do projeto SHA256 C/CPP

# Diretorios
SHA256_DIR = SHA256
LOTS_DIR = LOTS
WOTS_DIR = WOTS
MSS_DIR = MSS
TESTE_DIR = TesteHash

# Target padrao: compilar tudo
all: sha256 lamport wots mss

# Compilar biblioteca SHA256
sha256:
	@echo "================================"
	@echo "Compilando biblioteca SHA256..."
	@echo "================================"
	$(MAKE) -C $(SHA256_DIR)
	@echo ""

# Compilar LOTS
lamport: sha256
	@echo "================================"
	@echo "Compilando LOTS..."
	@echo "================================"
	$(MAKE) -C $(LOTS_DIR)
	@echo ""

# Compilar WOTS
wots: sha256
	@echo "================================"
	@echo "Compilando WOTS..."
	@echo "================================"
	$(MAKE) -C $(WOTS_DIR)
	@echo ""

# Compilar MSS
mss: sha256
	@echo "================================"
	@echo "Compilando MSS..."
	@echo "================================"
	$(MAKE) -C $(MSS_DIR)
	@echo ""

# Compilar TesteHash
teste: sha256
	@echo "================================"
	@echo "Compilando TesteHash..."
	@echo "================================"
	$(MAKE) -C $(TESTE_DIR)
	@echo ""

# Executar testes automatizados
test: all
	@echo "================================"
	@echo "Executando testes..."
	@echo "================================"
	./test_algorithms.sh

# Limpar tudo
clean:
	@echo "================================"
	@echo "Limpando projeto completo..."
	@echo "================================"
	$(MAKE) -C $(SHA256_DIR) clean
	$(MAKE) -C $(LOTS_DIR) clean
	$(MAKE) -C $(WOTS_DIR) clean
	$(MAKE) -C $(MSS_DIR) clean
	$(MAKE) -C $(TESTE_DIR) clean
	@echo ""
	@echo "Removendo arquivos txt..."

	rm -f *.txt
	@echo ""
	@echo "================================"
	@echo "Limpeza concluida!"
	@echo "================================"

# Recompilar tudo do zero
rebuild: clean all

# Ajuda
help:
	@echo "================================"
	@echo "Makefile - SHA256 C/CPP"
	@echo "================================"
	@echo "Targets disponiveis:"
	@echo "  make           - Compila biblioteca SHA256, Lamport, WOTS e MSS"
	@echo "  make all       - Mesmo que 'make'"
	@echo "  make sha256    - Compila apenas a biblioteca SHA256"
	@echo "  make lots  	- Compila apenas LOTS"
	@echo "  make wots      - Compila apenas WOTS"
	@echo "  make mss       - Compila apenas MSS"
	@echo "  make teste     - Compila TesteHash"
	@echo "  make test      - Compila e executa testes automatizados"
	@echo "  make clean     - Remove todos os arquivos compilados"
	@echo "  make rebuild   - Limpa e recompila tudo"
	@echo "  make help      - Mostra esta mensagem"
	@echo "================================"

.PHONY: all sha256 lots wots mss teste test clean rebuild help
