#include <errno.h>
#include <mictcp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

//
// Déclaration des types, constantes et macros
//

#define ENABLE_TCP_LOSS 1
#define MAX_UDP_SEGMENT_SIZE 1480
#define MICTCP_PORT 1337
#define VIDEO_FILE "../video/video.bin"

/**
 * Macro utilisée pour afficher le message d'erreur msg passé en paramètre
 * si la condition cond est validée, puis arrêter le programme.
 * Le message affiché contient des informations supplémentaires concernant
 * le fichier de provenance, la fonction et le numéro de ligne concerné.
 * Si errno est set, le message d'erreur associé est aussi affiché.
 */
#define ERROR_IF(cond,msg) \
    if (cond) { \
        if (errno != 0) { \
            fprintf(stderr, "%s:%d [%s()] -> %s (%s)\n", \
                    __FILE__, __LINE__, __func__, msg, strerror(errno)); \
        } else { \
            fprintf(stderr, "%s:%d [%s()] -> %s\n", \
                    __FILE__, __LINE__, __func__, msg); \
        } \
        exit(EXIT_FAILURE); \
    }

/**
 * Fonctions du programme
 */
enum gateway_function {
    UND_FCT,
    SOURCE,
    PUITS
};

/**
 * Protocoles pouvant être utilisé
 */
enum gateway_protocol {
    PROTO_TCP,
    PROTO_MICTCP
};

//
// Déclaration des fonctions locales
//

static void file_to_faketcp(char* filename, char *host, int port);
static void file_to_mictcp(char* filename);
static void mictcp_to_udp(char *host, int port);
static int read_rtp_packet(FILE *fd, struct timespec *timestamp, char *buffer, int buffer_size);
static struct timespec tsSubtract(struct timespec time1, struct timespec time2);
static void usage(void);

//
// Corps des fonctions publiques
//

int main(int argc, char** argv)
{
    enum gateway_protocol proto = PROTO_TCP;
    enum gateway_function func = UND_FCT;

    int ch;
    while ((ch = getopt(argc, argv, "t:sp")) != -1) {
        switch (ch) {
        case 't':
            if (strcmp(optarg, "mictcp") == 0) {
                proto = PROTO_MICTCP;
            } else if (strcmp(optarg, "tcp") == 0) {
                proto = PROTO_TCP;
            } else {
                printf("Unrecognized transport : %s\n", optarg);
                usage();
            }
            break;
        case 's':
            if (func == UND_FCT) {
                func = SOURCE;
            } else {
                usage();
            }
            break;
        case 'p':
            if (func == UND_FCT) {
                func = PUITS;
            } else {
                usage();
            }
            break;
        default:
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (func == UND_FCT || (func == PUITS && argc != 1) || (func == SOURCE && argc != 2)) {
        usage();
    }

    if (proto == PROTO_TCP) {
        if (func == SOURCE) {
            file_to_faketcp(VIDEO_FILE, argv[0], atoi(argv[1]));
        } else {
            printf("No gateway needed for puits using UDP\n");
        }
    } else {
        if (func == SOURCE) {
            file_to_mictcp(VIDEO_FILE);
        } else {
            mictcp_to_udp("127.0.0.1", atoi(argv[0]));
        }
    }
    return 0;
}

//
// Corps des fonctions privées
//

/**
 * Print usage and exit
 */
static void usage(void)
{
    printf("usage: gateway [-p|-s][-t tcp|mictcp] (<server>) <port>\n");
    exit(EXIT_FAILURE);
}

/**
 * Function that emulates TCP behavior while reading a file making it look like TCP was used.
 */
static void file_to_faketcp(char* filename, char *host, int port)
{
    /* Création du socket */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    ERROR_IF(sockfd == -1, "Socket error");

    /* Construction de l'adresse du socket distant */
    struct sockaddr_in s_addr = {0};
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    struct hostent* host_info = gethostbyname(host);
    ERROR_IF(host_info == NULL, "Error gethostbyname");
    ERROR_IF(host_info->h_addrtype != AF_INET, "gethostbyname bad h_addrtype");
    ERROR_IF(host_info->h_addr == NULL, "gethostbyname no addr");
    memcpy(&(s_addr.sin_addr), host_info->h_addr, host_info->h_length);

    /* Ouverture du fichier vidéo */
    FILE *filefd = fopen(filename, "rb");
    ERROR_IF(filefd == NULL, "Error fopen");

    uint count = 0;                             // compteur de paquets
    struct timespec current_time, last_time;    // stockage des timestamps
    char buffer[MAX_UDP_SEGMENT_SIZE];          // buffer de lecture/ecriture
    last_time.tv_sec = -1;
    last_time.tv_nsec = LONG_MAX;

    /* Lecture jusqu'à la fin du fichier vidéo */
    while (!feof(filefd)) {

        /* Lecture du paquet rtp */
        int nb_read = read_rtp_packet(filefd, &current_time, buffer, MAX_UDP_SEGMENT_SIZE);

        /* Attente avant la prochaine lecture */
        struct timespec delay = tsSubtract(current_time, last_time);
        nanosleep(&delay, NULL);

        /* Mise à jour du timestamp */
        last_time = current_time;

        if (ENABLE_TCP_LOSS) {
            /* On émule les pertes de paquets en délayant l'envoi de 2 secondes */
            if (count++ == 600) {
                printf("Simulating TCP loss\n");
                sleep(2);
                count = 0;
            }
        }

        /* Envoi du paquet rtp via faketcp */
        int nb_sent = sendto(sockfd, buffer, nb_read, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
        ERROR_IF(nb_sent == -1, "Error sendto");
    }

    /* Fermeture du socket et du fichier */
    close(sockfd);
    fclose(filefd);
}

/**
 * Function that reads a file and delivers to MICTCP.
 */
static void file_to_mictcp(char* filename)
{
    /* Création du socket MICTCP */
    int sockfd = mic_tcp_socket(CLIENT);
    if (sockfd == -1) {
        printf("ERROR creating the MICTCP socket\n");
    }

    /* On effectue la connexion */
    mic_tcp_sock_addr dest_addr;
    dest_addr.ip_addr = "localhost";
    dest_addr.ip_addr_size = strlen(dest_addr.ip_addr) + 1; // '\0'
    dest_addr.port = MICTCP_PORT;
    if (mic_tcp_connect(sockfd, dest_addr) == -1) {
        printf("ERROR connecting the MICTCP socket\n");
    }

    /* Ouverture du fichier vidéo */
    FILE *filefd = fopen(filename, "rb");
    ERROR_IF(filefd == NULL, "Error fopen");

    struct timespec current_time, last_time;    // stockage des timestamps
    char buffer[MAX_UDP_SEGMENT_SIZE];          // buffer de lecture/ecriture
    last_time.tv_sec = -1;
    last_time.tv_nsec = LONG_MAX;

    /* Lecture jusqu'à la fin du fichier vidéo */
    while (!feof(filefd)) {

        /* Lecture du paquet rtp */
        int nb_read = read_rtp_packet(filefd, &current_time, buffer, MAX_UDP_SEGMENT_SIZE);

        /* Attente avant la prochaine lecture */
        struct timespec delay = tsSubtract(current_time, last_time);
        nanosleep(&delay, NULL);

        /* Mise à jour du timestamp */
        last_time = current_time;

        /* Envoi du paquet rtp via mictcp */
        int nb_sent = mic_tcp_send(sockfd, buffer, nb_read);
        if (nb_sent < 0) {
            printf("ERROR on MICTCP send\n");
        }
    }

    /* Fermeture du socket et du fichier */
    if (mic_tcp_close(sockfd) == -1) {
        printf("ERROR on MICTCP close\n");
    }
    fclose(filefd);
}

/**
 * Function that listens on MICTCP and delivers to UDP.
 */
static void mictcp_to_udp(char *host, int port)
{
    /* Création du socket UDP */
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    ERROR_IF(udp_sockfd == -1, "Socket error");

    /* Construction de l'adresse du socket distant */
    struct sockaddr_in remote_s_addr = {0};
    remote_s_addr.sin_family = AF_INET;
    remote_s_addr.sin_port = htons(port);
    struct hostent* host_info = gethostbyname(host);
    ERROR_IF(host_info == NULL, "Error gethostbyname");
    ERROR_IF(host_info->h_addrtype != AF_INET, "gethostbyname bad h_addrtype");
    ERROR_IF(host_info->h_addr == NULL, "gethostbyname no addr");
    memcpy(&(remote_s_addr.sin_addr), host_info->h_addr, host_info->h_length);

    /* Création du socket MICTCP */
    int mictcp_sockfd = mic_tcp_socket(SERVER);
    if (mictcp_sockfd == -1) {
        printf("ERROR creating the MICTCP socket\n");
    }

    /* On bind le socket mictcp à une adresse locale */
    mic_tcp_sock_addr mt_local_addr;
    mt_local_addr.ip_addr = NULL;
    mt_local_addr.ip_addr_size = 0;
    mt_local_addr.port = MICTCP_PORT;
    if (mic_tcp_bind(mictcp_sockfd, mt_local_addr) == -1) {
        printf("ERROR on binding the MICTCP socket\n");
    }

    /* Acceptation d'une demande de connexion */
    mic_tcp_sock_addr mt_remote_addr;
    if (mic_tcp_accept(mictcp_sockfd, &mt_remote_addr) == -1) {
        printf("ERROR on accept on the MICTCP socket\n");
    }

    /* Lecture mictcp vers udp */
    char buff[MAX_UDP_SEGMENT_SIZE];    // buffer de lecture/ecriture
    while (1) {
        int nb_read = mic_tcp_recv(mictcp_sockfd, buff, MAX_UDP_SEGMENT_SIZE);
        if (nb_read <= 0) {
            if (nb_read < 0) {
                printf("ERROR on mic_recv on the MICTCP socket\n");
            }
            break;      // Fin de la transmission
        }

        int nb_sent = sendto(udp_sockfd, buff, nb_read, 0, (struct sockaddr*)&remote_s_addr, sizeof(remote_s_addr));
        ERROR_IF(nb_sent == -1, "Error sendto");
    }

    /* Fermeture des sockets */
    if (mic_tcp_close(mictcp_sockfd) == -1) {
        printf("ERROR on MICTCP close\n");
    }
    close(udp_sockfd);
}

/**
 * Read one rtp packet from the video file in the buffer, as well as
 * the corresponding timestamp
 * Return the number of bytes read
 */
static int read_rtp_packet(FILE *fd, struct timespec *timestamp, char *buffer, int buffer_size)
{
    /* Les champs du timestamp sont stockés sur 4 octets (héritage de la version 32 bits) */
    timestamp->tv_sec = 0;
    timestamp->tv_nsec = 0;
    fread(&(timestamp->tv_sec), 1, 4, fd);
    fread(&(timestamp->tv_nsec), 1, 4, fd);

    /* Taille du packet rdp */
    int packet_size = 0;
    fread(&packet_size, 1, sizeof(int), fd);

    /* Lecture du paquet rdp */
    ERROR_IF(packet_size > buffer_size, "Buffer is too small to store the packet");
    return fread(buffer, 1, packet_size, fd);
}

/**
 * Return (time1 - time2) when (time1 > time2), 0 otherwise
 */
static struct timespec tsSubtract(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    /* Subtract the second time from the first. */
    if ((time1.tv_sec < time2.tv_sec) || ((time1.tv_sec == time2.tv_sec) && (time1.tv_nsec <= time2.tv_nsec))) {
        /* TIME1 <= TIME2 */
        result.tv_sec = 0;
        result.tv_nsec = 0;
    } else { /* TIME1 > TIME2 */
        result.tv_sec = time1.tv_sec - time2.tv_sec;
        if (time1.tv_nsec < time2.tv_nsec) {
            result.tv_nsec = time1.tv_nsec + 1000000000L - time2.tv_nsec;
            result.tv_sec--; /* Borrow a second. */
        } else {
            result.tv_nsec = time1.tv_nsec - time2.tv_nsec;
        }
    }

    return (result);
}
