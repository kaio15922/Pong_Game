#pragma once

#define WIN32_LEAN_AND_MEAN
// Desativa funções conflitantes do Windows antes de incluir o Winsock e a Raylib
#define NOGDI             // Exclui recursos gráficos antigos do Windows
#define NOUSER            // Exclui funções de usuário do Windows (como ShowCursor)

#define SERVER_PORT 8888
#define SERVER_IP "127.0.0.1" // "127.0.0.1" significa "meu próprio PC" (Localhost)


typedef struct 
{
    int id_jogador;
    int tecla_W;
    int tecla_S;
} PacoteInput;

typedef struct 
{
    float bola_x, bola_y;
    float jogador1_y;
    float jogador2_y;
    int score_p1;
    int score_p2;
    unsigned int sequencia;
} PacoteEstado;

typedef enum { TELA_MENU, TELA_AGUARDANDO, TELA_JOGO } EstadoTela;