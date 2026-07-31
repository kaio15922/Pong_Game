#include <stdio.h>
#include <string.h>   // Mudança-Inicio: adicionado para tratamento de strings (strcmp) // Mudança-Fim
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
    unsigned int sequencia_servidor = 0;

    // LIGAR A REDE DO WINDOWS
    printf("Ligando a rede do Windows..\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) 
    {
        printf("Moio pra ligar a rede. Erro: %d\n", WSAGetLastError());
        return 1;
    }

    // CRIAR O CANAL UDP (O SOCKET)
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) 
    {
        printf("Moio pra criar o canal UDP. Erro: %d\n", WSAGetLastError());
        WSACleanup(); 
        return 1;
    }
    printf("Canal UDP criado com sucesso.\n");

    // CONFIGURAR O ENDEREÇO DO SERVIDOR
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = INADDR_ANY; // Escuta tanto conexões diretas quanto Broadcasts locais
    server_addr.sin_port = htons(PORT);       

    // RESERVAR A PORTA NO SISTEMA (BIND)
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) 
    {
        printf("Erro ao reservar a porta 8888. Erro: %d\n", WSAGetLastError());
        closesocket(server_socket); 
        WSACleanup();
        return 1;
    }
    printf("Porta reservada! Servidor escutando na porta %d...\n", PORT);

    // ATIVANDO O MODO NÃO-BLOQUEANTE
    unsigned long modo_nao_bloqueante = 1; 
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
        
        struct sockaddr_in de_onde_veio_o_pacote;
        int tamanho_endereco = sizeof(de_onde_veio_o_pacote);

        // ESCUTAR A REDE (COM VALIDAÇÃO DE VAGA)
        // Mudança-Inicio: Criamos um buffer maior (256 bytes) para caber tanto strings quanto as structs do jogo
        char buffer_rede[256]; 
        int bytes_recebidos;
        
        // Agora lemos para dentro do buffer_rede
        while ((bytes_recebidos = recvfrom(server_socket, buffer_rede, sizeof(buffer_rede), 0, (struct sockaddr *)&de_onde_veio_o_pacote, &tamanho_endereco)) > 0) 
        {
            // 1. Verifica se o pacote recebido é a string de Broadcast LAN
            if (bytes_recebidos >= 14 && strncmp(buffer_rede, "PONG_DISCOVERY", 14) == 0)
            {
                char sinal_resposta[] = "PONG_SERVER_ALIVE";
                // Responde diretamente para a máquina do cliente que está escutando
                sendto(server_socket, sinal_resposta, strlen(sinal_resposta), 0, (struct sockaddr *)&de_onde_veio_o_pacote, tamanho_endereco);
                continue; // Consome o pacote e ignora o resto da física
            }

            // 2. Se não for broadcast, verifica se tem o tamanho exato do nosso PacoteInput
            if (bytes_recebidos == sizeof(PacoteInput))
            {
                // Converte (cast) os bytes puros do buffer de volta para a estrutura do jogo
                PacoteInput* input_recebido = (PacoteInput*)buffer_rede;

                // Desocultação = Faz o XOR com a mesma chave para descriptografar os comandos
                int tecla_W_real = input_recebido->tecla_W ^ 0x5A;
                int tecla_S_real = input_recebido->tecla_S ^ 0x5A;
                
                if (input_recebido->id_jogador == 1) 
                {
                    if (!j1_conectado || (addr_jogador1.sin_addr.s_addr == de_onde_veio_o_pacote.sin_addr.s_addr && addr_jogador1.sin_port == de_onde_veio_o_pacote.sin_port)) 
                    {
                        addr_jogador1 = de_onde_veio_o_pacote;
                        j1_conectado = 1;
                        
                        if (tecla_W_real && p1_y > 0) p1_y -= p_velocidade;
                        if (tecla_S_real && p1_y < SCREEN_HEIGHT - p_altura) p1_y += p_velocidade;
                    } 
                    else 
                    {
                        printf("[AVISO] Tentativa de dupla conexao no Jogador 1 rejeitada.\n");
                    }
                } 
                else if (input_recebido->id_jogador == 2) 
                {
                    if (!j2_conectado || (addr_jogador2.sin_addr.s_addr == de_onde_veio_o_pacote.sin_addr.s_addr && addr_jogador2.sin_port == de_onde_veio_o_pacote.sin_port)) 
                    {
                        addr_jogador2 = de_onde_veio_o_pacote;
                        j2_conectado = 1;
                        
                        if (tecla_W_real && p2_y > 0) p2_y -= p_velocidade;
                        if (tecla_S_real && p2_y < SCREEN_HEIGHT - p_altura) p2_y += p_velocidade;
                    } 
                    else
                    {
                        printf("[AVISO] Tentativa de dupla conexao no Jogador 2 rejeitada.\n");
                    }
                }
            }
        }
        // Mudança-Fim
        
        // FÍSICA SÓ FUNCIONA COM OS DOIS CONECTADOS
        if (j1_conectado && j2_conectado) 
        {
            b_x += b_velo_x;
            b_y += b_velo_y;

            if (b_y - b_raio <= 0 || b_y + b_raio >= SCREEN_HEIGHT) 
            {
                b_velo_y *= -1;
            }

            if (b_x - b_raio <= 20 + p_largura && b_y >= p1_y && b_y <= p1_y + p_altura) 
            {
                b_velo_x *= -1.1;
                b_x = 20 + p_largura + b_raio;
            }

            if (b_x + b_raio >= (SCREEN_WIDTH - 20 - p_largura) && b_y >= p2_y && b_y <= p2_y + p_altura) 
            {
                b_velo_x *= -1.1;
                b_x = (SCREEN_WIDTH - 20 - p_largura) - b_raio;
            }

            if (b_x < 0) 
            {
                scoreP2++;
                b_x = (float)SCREEN_WIDTH / 2; b_y = (float)SCREEN_HEIGHT / 2;
                b_velo_x = 4.0f;
                b_velo_x *= -1;
            } 
            else if (b_x > SCREEN_WIDTH) 
            {
                scoreP1++;
                b_x = (float)SCREEN_WIDTH / 2; b_y = (float)SCREEN_HEIGHT / 2;
                b_velo_x = 4.0f;
                b_velo_x *= -1;
            }
        } 
        else 
        {
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

        estado_envio.sequencia = sequencia_servidor;
        sequencia_servidor++;

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

    printf("Fechando o servidor...\n");
    closesocket(server_socket); 
    WSACleanup();              
    return 0;
}