#pragma once

#define PORT 8888
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

// struct que o cliente envia para o servidor
typedef struct 
{
    int id_jogador;    // 1 para o jogador da esquerda, 2 para o da direita
    int tecla_W;       // 1 se estiver apertando W, 0 se solto
    int tecla_S;       // 1 se estiver apertando S, 0 se solto
} PacoteInput;

// struct que o servidor devolve para os clientes
typedef struct 
{
    float bola_x, bola_y;
    float jogador1_y;
    float jogador2_y;
    int score_p1;
    int score_p2;
    unsigned int sequencia;
} PacoteEstado;