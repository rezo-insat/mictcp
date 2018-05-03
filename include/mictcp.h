#ifndef MICTCP_H
#define MICTCP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>


/*
 * Etats du protocole (les noms des états sont donnés à titre indicatif
 * et peuvent être modifiés)
 */
typedef enum protocol_state
{
    IDLE, CLOSED, SYN_SENT, SYN_RECEIVED, ESTABLISHED, CLOSING
} protocol_state;

/*
 * Mode de démarrage du protocole
 * NB : nécessaire à l’usage de la fonction initialize_components()
 */
typedef enum start_mode { CLIENT, SERVER } start_mode;

/*
 * Structure d’une adresse de socket
 */
typedef struct mic_tcp_sock_addr
{
    char * ip_addr;
    int ip_addr_size;
    unsigned short port;
} mic_tcp_sock_addr;

/*
 * Structure d'un socket
 */
typedef struct mic_tcp_sock
{
  int fd;  /* descripteur du socket */
  protocol_state state; /* état du protocole */
  mic_tcp_sock_addr addr; /* adresse du socket */
} mic_tcp_sock;

/*
 * Structure des données utiles d’un PDU MIC-TCP
 */
typedef struct mic_tcp_payload
{
  char* data; /* données applicatives */
  int size; /* taille des données */
} mic_tcp_payload;

/*
 * Structure de l'entête d'un PDU MIC-TCP
 */
typedef struct mic_tcp_header
{
  unsigned short source_port; /* numéro de port source */
  unsigned short dest_port; /* numéro de port de destination */
  unsigned int seq_num; /* numéro de séquence */
  unsigned int ack_num; /* numéro d'acquittement */
  unsigned char syn; /* flag SYN (valeur 1 si activé et 0 si non) */
  unsigned char ack; /* flag ACK (valeur 1 si activé et 0 si non) */
  unsigned char fin; /* flag FIN (valeur 1 si activé et 0 si non) */
} mic_tcp_header;

/*
 * Structure d'un PDU MIC-TCP
 */
typedef struct mic_tcp_pdu
{
  mic_tcp_header header ; /* entête du PDU */
  mic_tcp_payload payload; /* charge utile du PDU */
} mic_tcp_pdu;

typedef struct app_buffer
{
    mic_tcp_payload packet;
    struct app_buffer* next;
    unsigned short id;
} app_buffer;


/****************************
 * Fonctions de l'interface *
 ****************************/
int mic_tcp_socket(start_mode sm);
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr);
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr);
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr);
int mic_tcp_send (int socket, char* mesg, int mesg_size);
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size);
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr);
int mic_tcp_close(int socket);

#endif
