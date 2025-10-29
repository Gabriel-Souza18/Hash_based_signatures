# Makefile principal do projeto SHA256 C/CPP

# Diretorios
SHA256_DIR = SHA256
LAMPORT_DIR = LamportOTS
WOTS_DIR = WOTS
TESTE_DIR = TesteHash

# Target padrao: compilar tudo
all: sha256 lamport wots

# Compilar biblioteca SHA256
sha256:
	@echo "================================"
	@echo "Compilando biblioteca SHA256..."
	@echo "================================"
	$(MAKE) -C $(SHA256_DIR)
	@echo ""

# Compilar Lamport OTS
lamport: sha256
	@echo "================================"
	@echo "Compilando Lamport OTS..."
	@echo "================================"
	$(MAKE) -C $(LAMPORT_DIR)
	@echo ""

# Compilar WOTS
wots: sha256
	@echo "================================"
	@echo "Compilando WOTS..."
	@echo "================================"
	$(MAKE) -C $(WOTS_DIR)
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
	$(MAKE) -C $(LAMPORT_DIR) clean
	$(MAKE) -C $(WOTS_DIR) clean
	$(MAKE) -C $(TESTE_DIR) clean
	@echo ""
	@echo "Removendo arquivos de resultados e txt..."
	rm -f resultados_*.csv
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
	@echo "  make           - Compila biblioteca SHA256, Lamport e WOTS"
	@echo "  make all       - Mesmo que 'make'"
	@echo "  make sha256    - Compila apenas a biblioteca SHA256"
	@echo "  make lamport   - Compila apenas Lamport OTS"
	@echo "  make wots      - Compila apenas WOTS"
	@echo "  make teste     - Compila TesteHash"
	@echo "  make test      - Compila e executa testes automatizados"
	@echo "  make clean     - Remove todos os arquivos compilados"
	@echo "  make rebuild   - Limpa e recompila tudo"
	@echo "  make help      - Mostra esta mensagem"
	@echo "================================"

.PHONY: all sha256 lamport wots teste test clean rebuild help
