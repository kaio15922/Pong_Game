#include <stdio.h>
//#include <stdlib.h>
#include <winsock2.h> // A biblioteca que o Windows exige para mexer com rede
#include "server.h"

// linkar a biblioteca de rede ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int main() 
{
    WSADATA wsa;             // uma estrutura que o Windows usa para guardar as configurações da rede
    SOCKET server_socket;    // o "id" do socket na placa de rede
    struct sockaddr_in server_addr, client_addr; // structs para guardar o IP e a Porta do servidor e do cliente
    int client_addr_len = sizeof(client_addr);   // tamanho do struct do cliente (o Windows pede isso)

    // LIGAR A REDE DO WINDOWS
    printf("Ligando a rede do Windows...\n");
    // windows exige que dê esse WSAStartup antes de qualquer coisa de rede.
    // MAKEWORD(2,2) só diz que estamos usando a versão 2.2 da biblioteca deles.
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) 
    {
        printf("Moio pra ligar a rede. Erro: %d\n", WSAGetLastError());
        return 1;
    }

    // CRIAR O CANAL UDP (O SOCKET)
    // criando o socket
    // AF_INET = Vamos usar IPv4
    // SOCK_DGRAM = definindo que o protocolo de transporte será UDP.
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) 
    {
        printf("Moio pra criar o canal UDP. Erro: %d\n", WSAGetLastError());
        WSACleanup(); // desliga a rede do Windows antes de fechar o programa
        return 1;
    }
    printf("Canal UDP criado com sucesso.\n");

    // CONFIGURAR O ENDEREÇO DO SERVIDOR
    server_addr.sin_family = AF_INET; // fala que estamos no IPv4
    
    // INADDR_ANY = aceitar pacotes vindo de qualquer IP pro pc
    // (Seja jogando no localhost, pelo Wi-Fi ou pelo cabo de rede).
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    
    // htons converte o número da porta 8888 para o formato que as placas de rede entendam.
    server_addr.sin_port = htons(PORT);       

    // RESERVAR A PORTA NO SISTEMA (BIND)
    /*
    o bind serve pra dizer pro windows q a partir de agr, a porta 8888 pertence a ele e que tudo que for pacote e chegar
    via udp, manda pro codigo
    */
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) 
    {
        printf("Erro ao reservar a porta 8888. Erro: %d\n", WSAGetLastError());
        closesocket(server_socket); // fecha o canal para não travar a porta no PC
        WSACleanup();
        return 1;
    }
    printf("Porta reservada! Servidor escutando na porta %d...\n", PORT);

    // ATIVANDO O MODO NÃO-BLOQUEANTE
    unsigned long modo_nao_bloqueante = 1; // 1 ativa, 0 desativa
    // ioctlsocket avisa o Windows que este socket não deve travar o código esperando pacotes
    ioctlsocket(server_socket, FIONBIO, &modo_nao_bloqueante);

    // VARIÁVEIS DA FÍSICA DO PONG
    float p1_y = (float)SCREEN_HEIGHT / 2 - 40;
    float p2_y = (float)SCREEN_HEIGHT / 2 - 40;
    float p_largura = 15, p_altura = 80, p_velocidade = 6.0f;

    float b_x = (float)SCREEN_WIDTH / 2;
    float b_y = (float)SCREEN_HEIGHT / 2;
    float b_raio = 8;
    float b_velo_x = 4.0f, b_velo_y = 4.0f;

    int scoreP1 = 0, scoreP2 = 0;

    // structs para receber e enviar os dados pela rede
    PacoteInput input_recebido;
    PacoteEstado estado_envio;

    // guardas para salvar os endereços de rede dos dois jogadores que vão se conectar
    struct sockaddr_in addr_jogador1, addr_jogador2;
    int j1_conectado = 0, j2_conectado = 0;

    // loop Principal do Servidor
    while (1) {
        
        // cria uma estrutura local e temporária para o cliente deste frame específico.
        // isso impede que o endereço do Jogador 1 e do Jogador 2 se misturem na rede.
        struct sockaddr_in de_onde_veio_o_pacote;
        int tamanho_endereco = sizeof(de_onde_veio_o_pacote);

        // ESCUTAR A REDE (COM VALIDAÇÃO DE VAGA)
        while (recvfrom(server_socket, (char*)&input_recebido, sizeof(PacoteInput), 0, (struct sockaddr *)&de_onde_veio_o_pacote, &tamanho_endereco) > 0) 
        {
            
            if (input_recebido.id_jogador == 1) 
            {
                // se a vaga do P1 estiver vazia, OU se quem mandou o pacote for o PRÓPRIO P1 que já estava jogando
                if (!j1_conectado || (addr_jogador1.sin_addr.s_addr == de_onde_veio_o_pacote.sin_addr.s_addr && addr_jogador1.sin_port == de_onde_veio_o_pacote.sin_port)) 
                {
                    addr_jogador1 = de_onde_veio_o_pacote;
                    j1_conectado = 1;
                    
                    if (input_recebido.tecla_W && p1_y > 0) p1_y -= p_velocidade;
                    if (input_recebido.tecla_S && p1_y < SCREEN_HEIGHT - p_altura) p1_y += p_velocidade;
                } 
                else 
                {
                    // se o IP/Porta for diferente, significa que outra pessoa tentou roubar a vaga do P1.
                    // o servidor simplesmente ignora o input dessa pessoa e não atualiza o comando.
                    printf("[AVISO] Tentativa de dupla conexao no Jogador 1 rejeitada.\n");
                }
            } 
            else if (input_recebido.id_jogador == 2) 
            {
                // mesma checagem para o Jogador 2
                if (!j2_conectado || (addr_jogador2.sin_addr.s_addr == de_onde_veio_o_pacote.sin_addr.s_addr && addr_jogador2.sin_port == de_onde_veio_o_pacote.sin_port)) 
                {
                    addr_jogador2 = de_onde_veio_o_pacote;
                    j2_conectado = 1;
                    
                    if (input_recebido.tecla_W && p2_y > 0) p2_y -= p_velocidade;
                    if (input_recebido.tecla_S && p2_y < SCREEN_HEIGHT - p_altura) p2_y += p_velocidade;
                } 
                else
                {
                    printf("[AVISO] Tentativa de dupla conexao no Jogador 2 rejeitada.\n");
                }
            }
        }

        // FÍSICA SÓ FUNCIONA COM OS DOIS CONECTADOS
        if (j1_conectado && j2_conectado) 
        {
            // a bola só se move se os dois players já tiverem entrado no jogo
            b_x += b_velo_x;
            b_y += b_velo_y;

            // colisão com teto e chão
            if (b_y - b_raio <= 0 || b_y + b_raio >= SCREEN_HEIGHT) 
            {
                b_velo_y *= -1;
            }

            // colisão com a paleta 1 (Esquerda)
            if (b_x - b_raio <= 20 + p_largura && b_y >= p1_y && b_y <= p1_y + p_altura) 
            {
                b_velo_x *= -1.1;
                b_x = 20 + p_largura + b_raio;
            }

            // colisão com a paleta 2 (Direita)
            if (b_x + b_raio >= (SCREEN_WIDTH - 20 - p_largura) && b_y >= p2_y && b_y <= p2_y + p_altura) 
            {
                b_velo_x *= -1.1;
                b_x = (SCREEN_WIDTH - 20 - p_largura) - b_raio;
            }

            // sistema de pontuação e reset central
            if (b_x < 0) 
            {
                scoreP2++;
                b_x = (float)SCREEN_WIDTH / 2; b_y = (float)SCREEN_HEIGHT / 2;
                b_velo_x *= -1;
            } 
            else if (b_x > SCREEN_WIDTH) 
            {
                scoreP1++;
                b_x = (float)SCREEN_WIDTH / 2; b_y = (float)SCREEN_HEIGHT / 2;
                b_velo_x *= -1;
            }
        } 
        else 
        {
            // enquanto os dois não entrarem, força o placar em zero e a bola parada no meio
            scoreP1 = 0;
            scoreP2 = 0;
            b_x = (float)SCREEN_WIDTH / 2;
            b_y = (float)SCREEN_HEIGHT / 2;
        }

        // REPLICAR O ESTADO DO JOGO
        estado_envio.bola_x = b_x;
        estado_envio.bola_y = b_y;
        estado_envio.jogador1_y = p1_y;
        estado_envio.jogador2_y = p2_y;
        estado_envio.score_p1 = scoreP1;
        estado_envio.score_p2 = scoreP2;

        if (j1_conectado) 
        {
            sendto(server_socket, (char*)&estado_envio, sizeof(PacoteEstado), 0, (struct sockaddr *)&addr_jogador1, sizeof(addr_jogador1));
        }
        if (j2_conectado) 
        {
            sendto(server_socket, (char*)&estado_envio, sizeof(PacoteEstado), 0, (struct sockaddr *)&addr_jogador2, sizeof(addr_jogador2));
        }

        Sleep(1); 
    }

    // desliga e solta tudo
    printf("Fechando o servidor...\n");
    closesocket(server_socket); // libera a porta 8888 para outros programas poderem usar
    WSACleanup();               // desliga a biblioteca de rede do Windows
    return 0;
}