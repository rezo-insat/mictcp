#include <mictcp.h>
#include <stdio.h>

#define MAX_SIZE 1000

int main()
{
    int sockfd;
    mic_tcp_sock_addr addr;
    mic_tcp_sock_addr remote_addr;
    char chaine[MAX_SIZE];

    addr.ip_addr = "127.0.0.1";
    addr.port = 1234;


    if ((sockfd = mic_tcp_socket(SERVER)) == -1)
    {
        printf("[TSOCK] Erreur a la creation du socket MICTCP!\n");
        return 1;
    }
    else
    {
        printf("[TSOCK] Creation du socket MICTCP: OK\n");
    }

    if (mic_tcp_bind(sockfd, addr) == -1)
    {
        printf("[TSOCK] Erreur lors du bind du socket MICTCP!\n");
        return 1;
    }
    else
    {
        printf("[TSOCK] Bind du socket MICTCP: OK\n");
    }

    if (mic_tcp_accept(sockfd, &remote_addr) == -1)
    {
        printf("[TSOCK] Erreur lors de l'accept sur le socket MICTCP!\n");
        return 1;
    }
    else
    {
        printf("[TSOCK] Accept sur le socket MICTCP: OK\n");
    }


    memset(chaine, 0, MAX_SIZE);

    printf("[TSOCK] Appuyez sur CTRL+C pour quitter ...\n");

    while(1) {
        int rcv_size = 0;
        printf("[TSOCK] Attente d'une donnee, appel de mic_recv ...\n");
        rcv_size = mic_tcp_recv(sockfd, chaine, MAX_SIZE);
        printf("[TSOCK] Reception d'un message de taille : %d\n", rcv_size);
        printf("[TSOCK] Message Recu : %s", chaine);
    }
    return 0;
}
