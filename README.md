# Pong_Game
Projeto Final da disciplina de Redes de Computadores - CI/UFPB

# 🏓 Pong Multiplayer - UDP Socket Game

Este projeto consiste em um jogo clássico **Pong Multiplayer** em tempo real, desenvolvido na linguagem **C** utilizando a biblioteca gráfica **Raylib** para a interface do usuário e a API **Winsock2** para a comunicação de rede em baixo nível. 

O projeto foi construído para a disciplina de Redes de Computadores, focando na implementação prática de protocolos da Camada de Aplicação sobre o protocolo de transporte **UDP**.

---

## 🚀 Funcionalidades Implementadas

* **Arquitetura Cliente-Servidor Autoritativa:** O servidor (`server.c`) calcula de forma centralizada toda a física da bola, colisões, pontuações e posicionamento, eliminando desincronizações (desatrelado de interface gráfica).
* **Comunicação UDP Não-Bloqueante:** Configuração de sockets utilizando `ioctlsocket(FIONBIO)` no cliente e no servidor. Isso impede o congelamento da aplicação e garante uma taxa de atualização fluida a ~60Hz.
* **Controle de Concorrência e Validação de Vagas:** O servidor rastreia os endereços IP e portas de origem. Se um terceiro jogador tentar se conectar na vaga de um jogador ativo, seus pacotes são descartados para evitar interferência.
* **Menu Inicial Gráfico:** Interface limpa integrada na Raylib controlada por cliques de mouse, eliminando a necessidade de interações ou inputs pelo terminal de texto.
* **Painel NetGraph (Telemetria de Rede):** Exibição em tempo real do frame-rate (FPS), ID do último pacote recebido e contador de **Choke (Pacotes Perdidos)** baseado em números de sequência embutidos no cabeçalho da aplicação.
* **Ofuscação de Payload (Segurança):** Cifragem simétrica simples bit a bit através do operador `XOR (^)` com chave estática (`0x5A`) aplicada aos inputs antes do envio na rede, mitigando engenharia reversa rudimentar via ferramentas de captura de tráfego (como Wireshark).

---

## 📦 Estrutura do Protocolo (Camada de Aplicação)

Para a comunicação, foram definidas duas estruturas estritas de transmissão de dados empacotados:

### 1. Cliente para Servidor (`PacoteInput`)
Envia apenas o estado atual de controle do jogador.
```
typedef struct
{
    int id_jogador;    // Identificador (1 = Esquerda, 2 = Direita)
    int tecla_W;       // Estado mascarado por XOR do comando de subida
    int tecla_S;       // Estado mascarado por XOR do comando de descida
} PacoteInput;
```

### 2. Servidor para Cliente (PacoteEstado)
Replica a fotografia exata do cenário de jogo para renderização sincronizada.
```
typedef struct
{
    float bola_x, bola_y;    // Coordenadas cartesianas da bola
    float jogador1_y;       // Posição vertical da paleta esquerda
    float jogador2_y;       // Posição vertical da paleta direita
    int score_p1;           // Placar do jogador 1
    int score_p2;           // Placar do jogador 2
    unsigned int sequencia; // Número incremental para cálculo de perda de rede
} PacoteEstado;
```
---

## 🛠️ Como Compilar e Executar

### Pré-requisitos
- Compilador GCC (MinGW para Windows) configurado nas variáveis de ambiente.
- Biblioteca Raylib instalada no diretório padrão C:\raylib\raylib\src (ou ajuste as flags do compilador se o seu caminho for diferente).

### 1. Compilação do Servidor
Abra o terminal e execute o comando abaixo para vincular a biblioteca de rede do Windows (-lws2_32):
```
gcc server.c -o server.exe -lws2_32
```

### 2. Compilação do Cliente
Execute o comando unindo as diretivas de linkagem da Raylib e do Winsock2:
```
gcc client.c -o client.exe -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
```

### 3. Executando a Partida

- Inicie o servidor central no primeiro terminal:
```
./server.exe
```
- Abra uma instância do cliente no segundo terminal (ou em outra máquina na mesma sub-rede local direcionando o IP configurado no cliente.h):
```
./client.exe
```
- Abra a segunda instância do cliente para o jogador oponente. Assim que ambos selecionarem suas respectivas paletas no menu, a física do servidor será liberada e o jogo iniciará sincronizado.
