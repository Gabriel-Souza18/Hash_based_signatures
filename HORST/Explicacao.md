### 1. Assinatura HORS (Hash to Obtain Random Subset)


**Passo a passo:**
1. **Hash da Mensagem:** Primeiro, calcula o hash da mensagem $m$, gerando um valor $a = Hash(m)$[cite: 432].
2. **Divisão do Hash:** O valor $a$ é dividido em $k$ pedaços (substrings) menores, chamados $a_1, ..., a_k$[cite: 433]. Cada um desses pedaços tem exatamente o tamanho de $\log_2 t$ bits[cite: 433].
3. **Conversão para Índices:** Cada um desses pedaços de bits $a_j$ é interpretado como um número inteiro $i_j$[cite: 434]. Esses inteiros determinam quais chaves secretas serão reveladas.
4. **Montagem da Assinatura:** A assinatura final $\sigma$ é simplesmente a coleção das chaves privadas que correspondem a esses índices: $\sigma = (sk_{i_1}, ..., sk_{i_k})$[cite: 435].

---

### 2. Assinatura HORST (HORS with Trees)

O HORST reaproveita a lógica de seleção de índices do HORS, mas como ele usa uma árvore de Merkle para enxugar a chave pública, a assinatura precisa fornecer provas de que os segredos revelados pertencem a essa árvore.

**Passo a passo:**
1. **Geração das Chaves Internas:** Diferente do HORS tradicional que guarda todas as chaves, o HORST geralmente computa a chave secreta interna no momento da assinatura a partir de uma semente (seed)[cite: 490].
2. **Divisão da Mensagem:** A mensagem $M$ (ou seu hash, dependendo de como é chamada pelas camadas superiores) é dividida em $k$ pedaços, chamados $M_1, ..., M_k$, cada um com tamanho de $\log t$ bits[cite: 491].
3. **Conversão para Índices:** Assim como no HORS, cada pedaço $M_i$ é lido e interpretado como um número inteiro[cite: 492].
4. **Seleção dos Segredos:** Cada inteiro $M_i$ é usado como um índice para localizar e selecionar um pedaço da chave secreta, $sk_{M_i}$[cite: 493].
5. **Montagem da Assinatura com Caminhos de Autenticação:** A assinatura $\sigma$ é composta por $k$ blocos[cite: 494]. A grande diferença do HORST é que cada bloco $\sigma_i$ contém duas coisas[cite: 494]:
   * O elemento da chave secreta selecionado ($sk_{M_i}$)[cite: 494].
   * O **Caminho de Autenticação** ($Auth_{M_i}$), que é a lista de nós irmãos necessários para calcular e conectar aquele segredo específico até a raiz da árvore de Merkle[cite: 494]. 

**Resumo da diferença:** No HORS, você envia apenas os segredos. No HORST, você envia os segredos **mais** o "mapa" (caminho de autenticação) para que o verificador consiga chegar até a raiz da árvore e validar a assinatura.