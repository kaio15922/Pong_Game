#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER

#include "cliente.h"
#include <winsock2.h>     // redes do Windows sempre primeiro
#include <raylib.h>       // gráficos do jogo
#include <stdio.h>

// garante que o compilador sabe onde achar a biblioteca de rede
#pragma comment(lib, "ws2_32.lib")

int main() 
{
    // CONFIGURAÇÃO DA REDE (WINSOCK2)
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) 
    {
        printf("Erro Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) 
    {
        printf("Erro Socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // configura o endereço de destino (onde o servidor está rodando)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // ativa o modo não-bloqueante no cliente também para a Raylib não travar
    unsigned long modo_nao_bloqueante = 1;
    ioctlsocket(client_socket, FIONBIO, &modo_nao_bloqueante);

    // CONFIGURAÇÃO DA INTERFACE
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Pong Multiplayer UDP");
    SetTargetFPS(60);

    // VARIÁVEIS DE CONTROLE DE TELA
    EstadoTela tela_atual = TELA_MENU;
    int meu_id = 0; 

    // VARIÁVEIS DO NETGRAPH (MÉTRICAS DE REDE)
    unsigned int ultima_sequencia = 0;
    int pacotes_perdidos_total = 0;
    int primeiro_pacote = 1;

    // definição geométrica dos botões de escolha no menu
    Rectangle botao_p1 = { (float)screenWidth / 2 - 180, 180, 160, 60 };
    Rectangle botao_p2 = { (float)screenWidth / 2 + 20, 180, 160, 60 };

    // variáveis locais para desenhar o jogo
    PacoteEstado estado_jogo = 
    {
        .bola_x = (float)screenWidth / 2,
        .bola_y = (float)screenHeight / 2,
        .jogador1_y = (float)screenHeight / 2 - 40,
        .jogador2_y = (float)screenHeight / 2 - 40,
        .score_p1 = 0,
        .score_p2 = 0,
        .sequencia = 0
    };

    PacoteInput meu_input;

    // LOOP PRINCIPAL DO JOGO
    while (!WindowShouldClose()) 
    {
        // LÓGICA DO MENU INICIAL
        if (tela_atual == TELA_MENU)
        {
            Vector2 mouse_pos = GetMousePosition();

            // clique no botão do jogador 1 (Esquerda)
            if (CheckCollisionPointRec(mouse_pos, botao_p1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                meu_id = 1;
                meu_input.id_jogador = meu_id;
                tela_atual = TELA_AGUARDANDO;
                SetWindowTitle(TextFormat("Pong Multiplayer - Jogador %d", meu_id));
            }

            // clique no botão do jogador 2 (Direita)
            if (CheckCollisionPointRec(mouse_pos, botao_p2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                meu_id = 2;
                meu_input.id_jogador = meu_id;
                tela_atual = TELA_AGUARDANDO;
                SetWindowTitle(TextFormat("Pong Multiplayer - Jogador %d", meu_id));
            }
        }
        
        // LÓGICA DE REDE E JOGO (RODA APENAS SE ESTIVER CONECTADO/AGUARDANDO)
        if (tela_atual == TELA_AGUARDANDO || tela_atual == TELA_JOGO)
        {
            // --- CAPTURA O TECLADO E ENVIA PARA O SERVIDOR ---
            int w_puro = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ? 1 : 0;
            int s_puro = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) ? 1 : 0;

            // criptografa os dados usando a chave secreta 0x5A (90 em decimal) antes de enviar
            meu_input.tecla_W = w_puro ^ 0x5A;
            meu_input.tecla_S = s_puro ^ 0x5A;

            // dispara os botões apertados via UDP para o servidor
            sendto(client_socket, (char*)&meu_input, sizeof(PacoteInput), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

            // ESCUTA SE O SERVIDOR MANDOU O ESTADO ATUALIZADO
            PacoteEstado estado_recebido;
            while (recvfrom(client_socket, (char*)&estado_recebido, sizeof(PacoteEstado), 0, (struct sockaddr *)&server_addr, &server_addr_len) > 0) 
            {
                // LÓGICA DO NETGRAPH: CALCULA PERDA DE PACOTES
                if (primeiro_pacote)
                {
                    ultima_sequencia = estado_recebido.sequencia;
                    primeiro_pacote = 0;
                }
                else
                {
                    // Se o número que chegou for maior que o esperado + 1, houve um salto (perda)
                    if (estado_recebido.sequencia > ultima_sequencia + 1)
                    {
                        pacotes_perdidos_total += (estado_recebido.sequencia - ultima_sequencia - 1);
                    }
                    
                    // Só atualiza a sequência se o pacote recebido for mais recente (evita pacotes fora de ordem)
                    if (estado_recebido.sequencia > ultima_sequencia)
                    {
                        ultima_sequencia = estado_recebido.sequencia;
                    }
                }

                estado_jogo = estado_recebido;
                
                // transiciona para a tela de jogo assim que a bola sai do centro
                if (estado_jogo.bola_x != (float)screenWidth / 2 || estado_jogo.bola_y != (float)screenHeight / 2)
                {
                    tela_atual = TELA_JOGO;
                }
            }
        }

        // DESENHA A INTERFACES DEPENDENDO DO ESTADO DA TELA
        BeginDrawing();
            ClearBackground(BLACK);

            if (tela_atual == TELA_MENU)
            {
                DrawText("PONG MULTIPLAYER", screenWidth / 2 - MeasureText("PONG MULTIPLAYER", 40) / 2, 60, 40, WHITE);
                DrawText("Escolha seu lado para conectar:", screenWidth / 2 - MeasureText("Escolha seu lado para conectar:", 20) / 2, 130, 20, GRAY);
                
                // desenho do botão jogador 1 (muda de colisão no hover do mouse)
                DrawRectangleRec(botao_p1, CheckCollisionPointRec(GetMousePosition(), botao_p1) ? LIGHTGRAY : RAYWHITE);
                DrawText("JOGADOR 1\n(Esquerda)", botao_p1.x + 15, botao_p1.y + 12, 20, BLACK);

                // desenho do botão jogador 2
                DrawRectangleRec(botao_p2, CheckCollisionPointRec(GetMousePosition(), botao_p2) ? LIGHTGRAY : RAYWHITE);
                DrawText("JOGADOR 2\n(Direita)", botao_p2.x + 15, botao_p2.y + 12, 20, BLACK);
            }
            else if (tela_atual == TELA_AGUARDANDO)
            {
                DrawText(TextFormat("Voce entrou como Jogador %d", meu_id), screenWidth / 2 - MeasureText(TextFormat("Voce entrou como Jogador %d", meu_id), 24) / 2, 150, 24, RAYWHITE);
                DrawText("Aguardando o oponente se conectar...", screenWidth / 2 - MeasureText("Aguardando o oponente se conectar...", 20) / 2, 220, 20, GOLD);
            }
            else if (tela_atual == TELA_JOGO)
            {
                // linha central
                DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, GRAY);

                // desenha paleta 1 (Esquerda)
                DrawRectangleRec((Rectangle){ 20, estado_jogo.jogador1_y, 15, 80 }, RAYWHITE);

                // desenha paleta 2 (Direita)
                DrawRectangleRec((Rectangle){ (float)screenWidth - 20 - 15, estado_jogo.jogador2_y, 15, 80 }, RAYWHITE);

                // desenha a bola
                DrawCircle(estado_jogo.bola_x, estado_jogo.bola_y, 8, RAYWHITE);

                // desenha o placar
                DrawText(TextFormat("%d", estado_jogo.score_p1), screenWidth / 4, 20, 40, WHITE);
                DrawText(TextFormat("%d", estado_jogo.score_p2), 3 * screenWidth / 4, 20, 40, WHITE);

                // --- PAINEL NETGRAPH (MÉTRICAS NO CANTO DA TELA) ---
                DrawRectangleRec((Rectangle){ 10, 10, 180, 55 }, ColorAlpha(DARKGRAY, 0.6f)); 
                DrawText(TextFormat("FPS: %d", GetFPS()), 15, 15, 12, GREEN);
                DrawText(TextFormat("Pacote ID: %u", ultima_sequencia), 15, 30, 12, RAYWHITE);
                DrawText(TextFormat("Pacotes Perdidos: %d", pacotes_perdidos_total), 15, 45, 12, pacotes_perdidos_total > 0 ? RED : GREEN);
            }

        EndDrawing();
    }

    // limpeza ao fechar
    CloseWindow();
    closesocket(client_socket);
    WSACleanup();
    return 0;
}