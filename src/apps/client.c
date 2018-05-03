#include <mictcp.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 1000

int main()
{

    int sockfd = 0;
    char chaine[MAX_SIZE];
    mic_tcp_sock_addr addr;
    addr.ip_addr = "127.0.0.1";
    addr.port = 1234;

    if ((sockfd = mic_tcp_socket(CLIENT)) == -1)
    {
        printf("[TSOCK] Erreur a la creation du socket MICTCP!\n");
        return 1;
    }
    else
    {
        printf("[TSOCK] Creation du socket MICTCP: OK\n");
    }

    if (mic_tcp_connect(sockfd, addr) == -1)
    {
        printf("[TSOCK] Erreur a la connexion du socket MICTCP!\n");
        return 1;
    }
    else
    {
        printf("[TSOCK] Connexion du socket MICTCP: OK\n");
    }

    memset(chaine, 0, MAX_SIZE);

    printf("[TSOCK] Entrez vos message a envoyer, CTRL+D pour quitter\n");
    while(fgets(chaine, MAX_SIZE , stdin) != NULL) {
        int sent_size = mic_tcp_send(sockfd, chaine, strlen(chaine)+1);
        printf("[TSOCK] Appel de mic_send avec un message de taille : %lu\n", strlen(chaine)+1);
        printf("[TSOCK] Appel de mic_send valeur de retour : %d\n", sent_size);
    }

    mic_tcp_close(sockfd);

    return 0;
}
