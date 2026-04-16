# CLAUDE.md - Instruções para Assistente IA

## 📋 Regras Gerais

1. **Sempre pergunte antes de modificar arquivos fora da pasta especificada**
   - Se o usuário não indicar uma pasta, pergunte: "Qual pasta posso modificar?"
   - Nunca faça alterações em múltiplas pastas sem confirmação explícita

2. **Respeite a estrutura do projeto**
   - `/SHA256` - Implementação SHA-256
   - `/WOTS` - Winternitz One-Time Signature
   - `/MSS` - Merkle Signature Scheme
   - `/LOTS` - Lamport One-Time Signature
   - `/HORS` - Hash to Obtain Random Subset

3. **Compilação e Testes**
   - Sempre compile após modificações
   - Valide com `make` ou `make test`
   - Se houver erro, corrija antes de entregar
   - Remova prints de debug desnecessários

4. **Padrões de Código**
   - Mantenha o estilo existente (nomes em português, tipos `unsigned char`, etc.)
   - Use `libsodium` para operações criptográficas
   - Comente mudanças significativas
   - Respeite os padrões dos makefiles

5. **Commits e Documentação**
   - Descreva claramente o que foi alterado
   - Explique o motivo da mudança
   - Use Markdown para respostas técnicas
   - Forneça exemplos de compilação/teste quando relevante

## 🔒 O Que NÃO Fazer

- ❌ Modificar arquivos fora da pasta indicada sem permissão
- ❌ Remover código funcional sem justificativa
- ❌ Adicionar dependências não aprovadas
- ❌ Deixar warnings de compilação sem resolver
- ❌ Quebrar compatibilidade com código existente

## ✅ O Que Fazer

- ✅ Perguntar qual pasta pode modificar se não estiver claro
- ✅ Validar compilação antes de entregar
- ✅ Manter consistência com o projeto
- ✅ Documentar mudanças significativas
- ✅ Otimizar quando possível (RAM, performance)

## 📂 Exemplo de Confirmação

**Usuário:** "Corrige os warnings"  
**IA:** "Em qual pasta? Posso modificar [`SHA256`](SHA256 ), [`WOTS`](WOTS ), [`MSS`](MSS ), [`LOTS`](LOTS ) ou [`HORS`](HORS )?"

**Usuário:** "LOTS"  
**IA:** "Certo, vou corrigir os warnings em [`LOTS`](LOTS ) apenas."

## 🛠️ Checklist Antes de Entregar

- [ ] Código compila sem erros
- [ ] Warnings tratados ou justificados
- [ ] Testes executam corretamente
- [ ] Documentação atualizada
- [ ] Mudanças claras e reversíveis
- [ ] Sem modificações fora da pasta indicada

---

**Última atualização:** 16 de abril de 2026  
**Projeto:** Hash-based Signatures (WOTS, MSS, LOTS, HORS)