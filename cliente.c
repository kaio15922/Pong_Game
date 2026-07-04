#include "cliente.h"
#include <winsock2.h>     // redes do Windows sempre primeiro
#include <raylib.h>       // gráficos do jogo
#include <stdio.h>
//#include <stdlib.h>

// garante que o compilador sabe onde achar a biblioteca de rede
#pragma comment(lib, "ws2_32.lib")

int main() 
{
    // PERGUNTA INICIAL: DEFINE QUEM É ESTE JOGADOR
    int meu_id = 1;
    printf("Escolha o seu jogador (1 para Esquerda, 2 para Direita): ");
    scanf("%d", &meu_id);

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
    InitWindow(screenWidth, screenHeight, TextFormat("Pong Multiplayer - Jogador %d", meu_id));
    SetTargetFPS(60);

    // variáveis locais para desenhar o jogo
    PacoteEstado estado_jogo = 
    {
        .bola_x = (float)screenWidth / 2,
        .bola_y = (float)screenHeight / 2,
        .jogador1_y = (float)screenHeight / 2 - 40,
        .jogador2_y = (float)screenHeight / 2 - 40,
        .score_p1 = 0,
        .score_p2 = 0
    };

    PacoteInput meu_input;
    meu_input.id_jogador = meu_id;

    // LOOP PRINCIPAL DO JOGO
    while (!WindowShouldClose()) 
    {
        
        // CAPTURA O TECLADO E ENVIA PARA O SERVIDOR
        meu_input.tecla_W = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ? 1 : 0;
        meu_input.tecla_S = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) ? 1 : 0;

        // dispara os botões apertados via UDP para o servidor
        sendto(client_socket, (char*)&meu_input, sizeof(PacoteInput), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // ESCUTA SE O SERVIDOR MANDOU O ESTADO ATUALIZADO
        PacoteEstado estado_recebido;
        // tenta ler a rede. Se não chegou nada, passa direto sem atualizar a tela
        // o while garante que vamos ler TODOS os pacotes que estão na fila 
        // e só vamos ficar com o último, que é o mais atualizado de todos
        while (recvfrom(client_socket, (char*)&estado_recebido, sizeof(PacoteEstado), 0, (struct sockaddr *)&server_addr, &server_addr_len) > 0) 
        {
            estado_jogo = estado_recebido;
        }

        // DESENHA O JOGO BASEADO NO QUE ESTÁ NA STRUCT
        BeginDrawing();
            ClearBackground(BLACK);

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

        EndDrawing();
    }

    // limpeza ao fechar
    CloseWindow();
    closesocket(client_socket);
    WSACleanup();
    return 0;
}