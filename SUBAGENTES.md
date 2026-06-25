# SUBAGENTES.md — Registro e Explicação dos Subagentes

## O que é um Subagente?

Um **subagente** é um processo de IA auxiliar iniciado pelo assistente principal para executar tarefas em paralelo ou tarefas que exigiriam muito contexto se feitas no mesmo espaço de conversa. Ele possui as mesmas capacidades do agente principal (ler arquivos, fazer buscas, analisar código), mas opera em uma conversa separada e reporta os resultados de volta.

### Quando o assistente usa um Subagente

| Situação | Motivo |
|---|---|
| Leitura de um PDF extenso | Evita consumir o contexto da conversa principal com conteúdo muito longo |
| Pesquisa paralela a outras tarefas | O agente principal continua trabalhando enquanto o subagente pesquisa |
| Tarefa de pesquisa isolada | Não polui o contexto principal com muitos passos de leitura |
| Análise de múltiplas fontes simultâneas | Vários subagentes podem ser lançados em paralelo |

### Ciclo de vida de um Subagente

```
Agente principal
    │
    ├──► invoke_subagent(prompt, tipo)
    │         │
    │         └──► Subagente executa a tarefa de forma assíncrona
    │                   │
    │                   └──► Envia resultado de volta ao agente principal
    │
    └──► Agente principal recebe a resposta e continua o trabalho
```

O agente principal **não precisa ficar esperando** — ele continua outras tarefas e o sistema o notifica automaticamente quando o subagente termina.

---

## Registro de Subagentes Usados Neste Projeto

### 1. Pesquisador do artigo SPHINCS (25/06/2026)

| Campo | Detalhe |
|---|---|
| **ID da Conversa** | `0ffb251b-5700-472c-a7ea-204de2068523` |
| **Tipo** | `research` (somente leitura) |
| **Objetivo** | Analisar o artigo SPHINCS (Bernstein et al., 2015) e responder perguntas sobre a definição e funcionamento do HORST |
| **Log** | `file:///home/gabriel/.gemini/antigravity/brain/0ffb251b-5700-472c-a7ea-204de2068523/.system_generated/logs/transcript.jsonl` |

**Resultados obtidos:**

- **HORST é stateless**: Não requer rastreamento de estado. A escolha de índices é determinada pseudoaleatoriamente a partir da mensagem via PRF, sem estado persistente.
- **Sem state tracking na árvore**: A árvore é construída de forma puramente funcional (*bottom-up*), sem análise ou rastreamento de estados dos nós durante a construção ou verificação.
- **Bitmasks no artigo original**: A árvore Merkle do HORST usa XOR com bitmasks antes do hash — `H((esq ⊕ Q_L) ∥ (dir ⊕ Q_R))` — para que a segurança dependa apenas de resistência à segunda pré-imagem. A implementação no repositório omite as bitmasks por simplificação.
- **Diferença HORS → HORST**: Apenas a árvore de Merkle. A chave pública cai de `t × n` bits (ex: ~2 MB para t=65536) para apenas `n = 32` bytes (a raiz). A assinatura cresce com os caminhos de autenticação.
- **Parâmetros do SPHINCS-256**: `t = 2¹⁶ = 65536`, `k = 32`, `τ = 16` (altura da árvore HORST), `h = 60` (hiper-árvore total), `d = 12` camadas.

---

## Como Interpretar um Subagente no Contexto do Projeto

Sempre que o assistente lançar um subagente durante o desenvolvimento deste projeto, uma nova entrada será adicionada na seção **"Registro de Subagentes"** acima contendo:

- A data e o objetivo
- O ID da conversa (para rastreabilidade)
- O resultado obtido

---

*Arquivo mantido automaticamente pelo assistente IA.*
*Última atualização: 25 de junho de 2026*
