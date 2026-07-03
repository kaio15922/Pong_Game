#include <raylib.h>

// tamanho da janela do jogo
const int screenWidth = 800;
const int screenHeight = 450;

// struct das paletas dos jogadores
typedef struct Paleta
{
    float x, y;
    float altura, largura;
    float velocidade;
}Paleta;

// struct da bola
typedef struct Bola 
{
    float x, y;
    float Velo_x, Velo_y;
    float raio;
}Bola;

int main()
{ 
    // abre a tela e diz o FPS
    InitWindow(screenWidth, screenHeight, "Pong");
    SetTargetFPS(60);
  
    // criando os jogadores
    Paleta jogador1;
    jogador1.x = 20;
    jogador1.y = (float)screenHeight / 2 - 40;
    jogador1.largura = 15;
    jogador1.altura = 80;
    jogador1.velocidade = 6.0f;

    Paleta jogador2;
    jogador2.x = (float)screenWidth - 20 - 15;
    jogador2.y = (float)screenHeight / 2 - 40;
    jogador2.largura = 15;
    jogador2.altura = 80;
    jogador2.velocidade = 6.0f;

    // criando a bola
    Bola bola;
    bola.x = (float)screenWidth / 2;
    bola.y = (float)screenHeight / 2;
    bola.raio = 8.0f;
    bola.Velo_x = 4.0f;
    bola.Velo_y = 4.0f;

    // pontuação dos jogadores
    int pontuacao1 = 0;
    int pontuacao2 = 0;
    
    // loop do jogo
    while(!WindowShouldClose())
    {
        // controle do Jogador 1 (W / S)
        if (IsKeyDown(KEY_W) && jogador1.y > 0) jogador1.y -= jogador1.velocidade;
        if (IsKeyDown(KEY_S) && jogador1.y < screenHeight - jogador1.altura) jogador1.y += jogador1.velocidade;

        // controle do Jogador 2 (Setas Teclado: UP / DOWN)
        if (IsKeyDown(KEY_UP) && jogador2.y > 0) jogador2.y -= jogador2.velocidade;
        if (IsKeyDown(KEY_DOWN) && jogador2.y < screenHeight - jogador2.altura) jogador2.y += jogador2.velocidade;

        // movimentação Automática da Bola
        bola.x += bola.Velo_x;
        bola.y += bola.Velo_y;

        // colisão da Bola com o Teto e o Chão (Inverte velocidade Y)
        if (bola.y - bola.raio <= 0 || bola.y + bola.raio >= screenHeight) 
        {
            bola.Velo_y *= -1;
        }

        // colisão simples da Bola com o Jogador 1 (Esquerda)
        if (bola.x - bola.raio <= jogador1.x + jogador1.largura && bola.y >= jogador1.y && bola.y <= jogador1.y + jogador1.altura) 
        {
            bola.Velo_x *= -1.1;
            bola.x = jogador1.x + jogador1.largura + bola.raio; // Evita que a bola fique presa dentro da paleta
        }

        // colisão simples da Bola com o Jogador 2 (Direita)
        if (bola.x + bola.raio >= jogador2.x && bola.y >= jogador2.y && bola.y <= jogador2.y + jogador2.altura) 
        {
            bola.Velo_x *= -1.1;
            bola.x = jogador2.x - bola.raio; // evita que a bola fique presa dentro da paleta
        }

        // sistema de Pontuação (se a bola passar das extremidades laterais)
        if (bola.x < 0) 
        {
            pontuacao2++; // ponto do Jogador 2
            // reseta a bola no centro
            bola.x = (float)screenWidth / 2;
            bola.y = (float)screenHeight / 2;
            bola.Velo_x = 4.0f;
            bola.Velo_y = 4.0f;
            bola.Velo_x *= -1;
        }
        else if (bola.x > screenWidth) 
        {
            pontuacao1++; // ponto do Jogador 1
            // reseta a bola no centro
            bola.x = (float)screenWidth / 2;
            bola.y = (float)screenHeight / 2;
            bola.Velo_x = 4.0f;
            bola.Velo_y = 4.0f;
            bola.Velo_x *= -1;
        }

        // pondo as coisa na tela
        BeginDrawing();
        ClearBackground(BLACK);

        // linha divisória central
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, GRAY);

        // desenha as Paletas
        DrawRectangleRec((Rectangle){ jogador1.x, jogador1.y, jogador1.largura, jogador1.altura }, RAYWHITE);
        DrawRectangleRec((Rectangle){ jogador2.x, jogador2.y, jogador2.largura, jogador2.altura }, RAYWHITE);

        // desenha a Bola
        DrawCircle(bola.x, bola.y, bola.raio, RAYWHITE);

        // desenha o plcar
        DrawText(TextFormat("%d", pontuacao1), screenWidth / 4, 20, 40, WHITE);
        DrawText(TextFormat("%d", pontuacao2), 3 * screenWidth / 4, 20, 40, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}