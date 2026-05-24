import os
import matplotlib.pyplot as plt
from datetime import datetime

class Resultado:
    def __init__(self):
        self.tempos_LOTS = {
            'secret_keys': [],
            'public_keys': [],
            'assinatura': [],
            'total': []
        }
        self.tempos_WOTS = {
            'secret_keys': [],
            'public_keys': [],
            'masks': [],
            'assinatura': [],
            'total': []
        }
        self.tempos_HORS = {
            'secret_keys': [],
            'assinatura': [],
            'total': []
        }
        self.tamanho_LOTS = {
            'secret_keys': 0,
            'public_keys': 0,
            'assinatura': 0
        }
        self.tamanho_WOTS = {
            'secret_keys': 0,
            'public_keys': 0,
            'assinatura': 0
        }
        self.tamanho_HORS = {
            'secret_keys': 0,
            'public_keys': 0,
            'assinatura': 0
        }
        self.quantHashs_LOTS = []
        self.quantHashs_WOTS = []
        self.quantHashs_HORS = []
        self.timestamp = ""
        
    def calcular_medias(self):
        """Calcula as médias dos tempos"""
        medias = {
            'LOTS': {},
            'WOTS': {},
            'HORS': {}
        }
        
        # Médias LOTS
        if self.tempos_LOTS['total']:
            medias['LOTS']['secret_keys'] = sum(self.tempos_LOTS['secret_keys']) / len(self.tempos_LOTS['secret_keys'])
            medias['LOTS']['public_keys'] = sum(self.tempos_LOTS['public_keys']) / len(self.tempos_LOTS['public_keys'])
            medias['LOTS']['assinatura'] = sum(self.tempos_LOTS['assinatura']) / len(self.tempos_LOTS['assinatura'])
            medias['LOTS']['total'] = sum(self.tempos_LOTS['total']) / len(self.tempos_LOTS['total'])
        
        # Médias WOTS
        if self.tempos_WOTS['total']:
            medias['WOTS']['secret_keys'] = sum(self.tempos_WOTS['secret_keys']) / len(self.tempos_WOTS['secret_keys'])
            medias['WOTS']['public_keys'] = sum(self.tempos_WOTS['public_keys']) / len(self.tempos_WOTS['public_keys'])
            medias['WOTS']['masks'] = sum(self.tempos_WOTS['masks']) / len(self.tempos_WOTS['masks'])
            medias['WOTS']['assinatura'] = sum(self.tempos_WOTS['assinatura']) / len(self.tempos_WOTS['assinatura'])
            medias['WOTS']['total'] = sum(self.tempos_WOTS['total']) / len(self.tempos_WOTS['total'])
        
        # Médias HORS
        if self.tempos_HORS['total']:
            medias['HORS']['secret_keys'] = sum(self.tempos_HORS['secret_keys']) / len(self.tempos_HORS['secret_keys'])
            medias['HORS']['assinatura'] = sum(self.tempos_HORS['assinatura']) / len(self.tempos_HORS['assinatura'])
            medias['HORS']['total'] = sum(self.tempos_HORS['total']) / len(self.tempos_HORS['total'])
        
        return medias

def selecionar_resultados():
    """Lista arquivos CSV e permite ao usuário escolher um"""
    print("\n" + "="*60)
    print("  ANÁLISE DE RESULTADOS - ASSINATURAS DIGITAIS")
    print("="*60 + "\n")
    
    # Lista todos os arquivos CSV na pasta
    arquivos_csv = [f for f in os.listdir('.') if f.startswith('resultados_') and f.endswith('.csv')]
    
    if not arquivos_csv:
        print("Nenhum arquivo de resultados encontrado!")
        return None
    
    # Ordena por data (mais recente primeiro)
    arquivos_csv.sort(reverse=True)
    
    print("Arquivos de resultados disponíveis:\n")
    for i, arquivo in enumerate(arquivos_csv, 1):
        # Extrai o timestamp do nome do arquivo
        timestamp_str = arquivo.replace('resultados_', '').replace('.csv', '')
        try:
            dt = datetime.strptime(timestamp_str, '%Y%m%d_%H%M%S')
            data_formatada = dt.strftime('%d/%m/%Y às %H:%M:%S')
        except:
            data_formatada = timestamp_str
        
        print(f"  [{i}] {arquivo}")
        print(f"      Data: {data_formatada}\n")
    
    # Solicita escolha do usuário
    while True:
        try:
            escolha = input("Escolha o número do arquivo (ou 'q' para sair): ").strip()
            
            if escolha.lower() == 'q':
                return None
            
            escolha_num = int(escolha)
            if 1 <= escolha_num <= len(arquivos_csv):
                arquivo_escolhido = arquivos_csv[escolha_num - 1]
                print(f"\nArquivo selecionado: {arquivo_escolhido}\n")
                return arquivo_escolhido
            else:
                print(f"Por favor, escolha um número entre 1 e {len(arquivos_csv)}")
        except ValueError:
            print("Entrada inválida! Digite um número ou 'q' para sair.")

def ler_resultados(arquivo_csv):
    """Lê o arquivo CSV e popula o objeto Resultado"""
    resultado = Resultado()
    
    # Extrai timestamp do nome do arquivo
    resultado.timestamp = arquivo_csv.replace('resultados_', '').replace('.csv', '')
    
    try:
        with open(arquivo_csv, 'r') as f:
            linhas = f.readlines()
        
        # Pula o cabeçalho
        for linha in linhas[1:]:
            linha = linha.strip()
            if not linha:
                continue
            
            partes = linha.split(',')
            if len(partes) < 10:
                continue
            
            algoritmo = partes[0]
            tempo_sk = float(partes[2])
            tempo_pk = float(partes[3])
            tempo_masks = float(partes[4])
            tempo_assinatura = float(partes[5])
            hashes = int(partes[6])
            tamanho_sk = int(partes[7])
            tamanho_pk = int(partes[8])
            tamanho_assinatura = int(partes[9])
            
            if algoritmo == 'Lamport' or algoritmo == 'LOTS':
                resultado.tempos_LOTS['secret_keys'].append(tempo_sk)
                resultado.tempos_LOTS['public_keys'].append(tempo_pk)
                resultado.tempos_LOTS['assinatura'].append(tempo_assinatura)
                resultado.tempos_LOTS['total'].append(tempo_sk + tempo_pk + tempo_assinatura)
                resultado.quantHashs_LOTS.append(hashes)
                resultado.tamanho_LOTS['secret_keys'] = tamanho_sk
                resultado.tamanho_LOTS['public_keys'] = tamanho_pk
                resultado.tamanho_LOTS['assinatura'] = tamanho_assinatura
                
            elif algoritmo == 'WOTS':
                resultado.tempos_WOTS['secret_keys'].append(tempo_sk)
                resultado.tempos_WOTS['public_keys'].append(tempo_pk)
                resultado.tempos_WOTS['masks'].append(tempo_masks)
                resultado.tempos_WOTS['assinatura'].append(tempo_assinatura)
                resultado.tempos_WOTS['total'].append(tempo_sk + tempo_pk + tempo_masks + tempo_assinatura)
                resultado.quantHashs_WOTS.append(hashes)
                resultado.tamanho_WOTS['secret_keys'] = tamanho_sk
                resultado.tamanho_WOTS['public_keys'] = tamanho_pk
                resultado.tamanho_WOTS['assinatura'] = tamanho_assinatura
            
            elif algoritmo == 'HORS':
                resultado.tempos_HORS['secret_keys'].append(tempo_sk)
                resultado.tempos_HORS['assinatura'].append(tempo_assinatura)
                resultado.tempos_HORS['total'].append(tempo_sk + tempo_assinatura)
                resultado.quantHashs_HORS.append(hashes)
                resultado.tamanho_HORS['secret_keys'] = tamanho_sk
                resultado.tamanho_HORS['public_keys'] = tamanho_pk
                resultado.tamanho_HORS['assinatura'] = tamanho_assinatura
        
        print(f"Dados carregados: {len(resultado.tempos_LOTS['total'])} testes LOTS, {len(resultado.tempos_WOTS['total'])} testes WOTS, {len(resultado.tempos_HORS['total'])} testes HORS\n")
        
        return resultado
        
    except Exception as e:
        print(f"Erro ao ler arquivo: {e}")
        return None

def criar_graficos(resultado):
    """Cria gráficos comparativos entre LOTS, WOTS e HORS"""
    
    # Calcula médias
    medias = resultado.calcular_medias()
    
    # Formata timestamp para título
    try:
        dt = datetime.strptime(resultado.timestamp, '%Y%m%d_%H%M%S')
        titulo_data = dt.strftime('%d/%m/%Y às %H:%M:%S')
    except:
        titulo_data = resultado.timestamp
    
    # Cria figura com 3 subplots
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(22, 6))
    fig.suptitle(f'Comparação LOTS vs WOTS vs HORS - Resultados de {titulo_data}', 
                 fontsize=16, fontweight='bold')
    
    # ========== GRÁFICO 1: Comparação de Tempos ==========
    categorias = ['Secret Keys', 'Public Keys', 'Assinatura', 'Total']
    
    tempos_lots = [
        medias['LOTS'].get('secret_keys', 0) * 1000,
        medias['LOTS'].get('public_keys', 0) * 1000,
        medias['LOTS'].get('assinatura', 0) * 1000,
        medias['LOTS'].get('total', 0) * 1000
    ]
    
    tempos_wots = [
        medias['WOTS'].get('secret_keys', 0) * 1000,
        medias['WOTS'].get('public_keys', 0) * 1000,
        medias['WOTS'].get('assinatura', 0) * 1000,
        medias['WOTS'].get('total', 0) * 1000
    ]
    
    tempos_hors = [
        medias['HORS'].get('secret_keys', 0) * 1000,
        0,  # HORS não calcula PK separadamente
        medias['HORS'].get('assinatura', 0) * 1000,
        medias['HORS'].get('total', 0) * 1000
    ]
    
    x = range(len(categorias))
    largura = 0.25
    
    bars1 = ax1.bar([i - largura for i in x], tempos_lots, largura, 
                    label='LOTS', color='#3498db', alpha=0.8)
    bars2 = ax1.bar([i for i in x], tempos_wots, largura, 
                    label='WOTS', color='#e74c3c', alpha=0.8)
    bars3 = ax1.bar([i + largura for i in x], tempos_hors, largura, 
                    label='HORS', color='#2ecc71', alpha=0.8)
    
    ax1.set_xlabel('Operações', fontweight='bold')
    ax1.set_ylabel('Tempo (ms)', fontweight='bold')
    ax1.set_title('Comparação de Tempos de Execução', fontweight='bold', pad=15)
    ax1.set_xticks(x)
    ax1.set_xticklabels(categorias, rotation=15, ha='right')
    ax1.legend()
    ax1.grid(axis='y', alpha=0.3, linestyle='--')
    
    for bars in [bars1, bars2, bars3]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax1.text(bar.get_x() + bar.get_width()/2., height,
                        f'{height:.2f}',
                        ha='center', va='bottom', fontsize=7)
    
    # ========== GRÁFICO 2: Comparação de Tamanhos ==========
    categorias_tam = ['Secret Keys', 'Public Keys', 'Assinatura']
    
    tamanhos_lots = [
        resultado.tamanho_LOTS['secret_keys'] / 1024,
        resultado.tamanho_LOTS['public_keys'] / 1024,
        resultado.tamanho_LOTS['assinatura'] / 1024
    ]
    
    tamanhos_wots = [
        resultado.tamanho_WOTS['secret_keys'] / 1024,
        resultado.tamanho_WOTS['public_keys'] / 1024,
        resultado.tamanho_WOTS['assinatura'] / 1024
    ]
    
    tamanhos_hors = [
        resultado.tamanho_HORS['secret_keys'] / 1024,
        resultado.tamanho_HORS['public_keys'] / 1024,
        resultado.tamanho_HORS['assinatura'] / 1024
    ]
    
    x2 = range(len(categorias_tam))
    
    bars4 = ax2.bar([i - largura for i in x2], tamanhos_lots, largura, 
                    label='LOTS', color='#3498db', alpha=0.8)
    bars5 = ax2.bar([i for i in x2], tamanhos_wots, largura, 
                    label='WOTS', color='#e74c3c', alpha=0.8)
    bars6 = ax2.bar([i + largura for i in x2], tamanhos_hors, largura, 
                    label='HORS', color='#2ecc71', alpha=0.8)
    
    ax2.set_xlabel('Componentes', fontweight='bold')
    ax2.set_ylabel('Tamanho (KB)', fontweight='bold')
    ax2.set_title('Comparação de Tamanhos', fontweight='bold', pad=15)
    ax2.set_xticks(x2)
    ax2.set_xticklabels(categorias_tam, rotation=15, ha='right')
    ax2.legend()
    ax2.grid(axis='y', alpha=0.3, linestyle='--')
    
    for bars in [bars4, bars5, bars6]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax2.text(bar.get_x() + bar.get_width()/2., height,
                        f'{height:.1f}',
                        ha='center', va='bottom', fontsize=7)
    
    # ========== GRÁFICO 3: Comparação de Número de Hashes ==========
    categorias_hash = ['Assinatura']
    
    hashes_sig_lots = sum(resultado.quantHashs_LOTS) / len(resultado.quantHashs_LOTS) if resultado.quantHashs_LOTS else 0
    hashes_sig_wots = sum(resultado.quantHashs_WOTS) / len(resultado.quantHashs_WOTS) if resultado.quantHashs_WOTS else 0
    hashes_sig_hors = sum(resultado.quantHashs_HORS) / len(resultado.quantHashs_HORS) if resultado.quantHashs_HORS else 0
    
    hashes_lots = [hashes_sig_lots]
    hashes_wots = [hashes_sig_wots]
    hashes_hors = [hashes_sig_hors]
    
    x3 = range(len(categorias_hash))
    
    bars7 = ax3.bar([i - largura for i in x3], hashes_lots, largura, 
                    label='LOTS', color='#3498db', alpha=0.8)
    bars8 = ax3.bar([i for i in x3], hashes_wots, largura, 
                    label='WOTS', color='#e74c3c', alpha=0.8)
    bars9 = ax3.bar([i + largura for i in x3], hashes_hors, largura, 
                    label='HORS', color='#2ecc71', alpha=0.8)
    
    ax3.set_xlabel('Operações', fontweight='bold')
    ax3.set_ylabel('Número de Hashes SHA256', fontweight='bold')
    ax3.set_title('Comparação de Quantidade de Hashes', fontweight='bold', pad=15)
    ax3.set_xticks(x3)
    ax3.set_xticklabels(categorias_hash)
    ax3.legend()
    ax3.grid(axis='y', alpha=0.3, linestyle='--')
    
    for bars in [bars7, bars8, bars9]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax3.text(bar.get_x() + bar.get_width()/2., height,
                        f'{int(height)}',
                        ha='center', va='bottom', fontsize=8)
    
    # Ajusta layout
    plt.tight_layout()
    
    # Salva o gráfico
    nome_arquivo = f'grafico_{resultado.timestamp}.png'
    plt.savefig(nome_arquivo, dpi=300, bbox_inches='tight')
    print(f"Gráfico salvo: {nome_arquivo}")
    
    # Exibe informações adicionais
    print("\n" + "="*70)
    print("  RESUMO DOS RESULTADOS")
    print("="*70)
    print(f"\nLOTS (Lamport OTS):")
    print(f"   Tempo médio total: {medias['LOTS'].get('total', 0)*1000:.2f} ms")
    print(f"   Tamanho total: {sum(resultado.tamanho_LOTS.values())/1024:.1f} KB")
    if resultado.quantHashs_LOTS:
        print(f"   Hashes por assinatura: {resultado.quantHashs_LOTS[0]}")
    
    print(f"\nWOTS (Winternitz OTS):")
    print(f"   Tempo médio total: {medias['WOTS'].get('total', 0)*1000:.2f} ms")
    print(f"   Tamanho total: {sum(resultado.tamanho_WOTS.values())/1024:.1f} KB")
    if resultado.quantHashs_WOTS:
        print(f"   Hashes por assinatura: {resultado.quantHashs_WOTS[0]}")
    
    print(f"\nHORS (Hash to Obtain Random Subset):")
    print(f"   Tempo médio total: {medias['HORS'].get('total', 0)*1000:.2f} ms")
    print(f"   Tamanho total: {sum(resultado.tamanho_HORS.values())/1024:.1f} KB")
    if resultado.quantHashs_HORS:
        print(f"   Hashes por assinatura: {int(resultado.quantHashs_HORS[0])}")
    
    print("\n" + "="*70)
    print("="*70)
    

def main():
    """Função principal"""
    # Seleciona o arquivo
    arquivo = selecionar_resultados()
    if not arquivo:
        print("Programa encerrado.")
        return
    
    # Lê os resultados
    resultado = ler_resultados(arquivo)
    if not resultado:
        print("Erro ao processar resultados.")
        return
    
    # Cria os gráficos
    criar_graficos(resultado)
    
    # Exibe o gráfico
    plt.show()

if __name__ == "__main__":
    main()