#include <mictcp.h>
#include <api/mictcp_core.h>

int mic_tcp_socket(start_mode sm) 
// Permet de créer un socket entre l’application et MIC-TCP
// Retourne le descripteur du socket ou bien -1 en cas d'erreur
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   initialize_components(sm); // Appel obligatoire
   return -1;
}

int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
// Permet d’attribuer une adresse à un socket. Retourne 0 si succès, et -1 en cas d’échec
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
}

int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
// Met l’application en état d'acceptation d’une requête de connexion entrante
// Retourne 0 si succès, -1 si erreur
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return -1;
}

int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
// Permet de réclamer l’établissement d’une connexion
// Retourne 0 si la connexion est établie, et -1 en cas d’échec
{  
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return -1;
}

int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
// Permet de réclamer l’envoi d’une donnée applicative
// Retourne la taille des données envoyées, et -1 en cas d'erreur
// Dans le cas de la vidéo, seul la source va envoyer au puits
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    return -1; 
}


int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
// Permet à l’application réceptrice de réclamer la récupération d’une donnée 
// stockée dans les buffers de réception du socket
// Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
// NB : cette fonction fait appel à la fonction app_buffer_get() 
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    return -1;
}

int mic_tcp_close (int socket)
// Permet de réclamer la destruction d’un socket. 
// Engendre la fermeture de la connexion suivant le modèle de TCP. 
// Retourne 0 si tout se passe bien et -1 en cas d'erreur
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return -1;
}


void process_received_PDU(mic_tcp_pdu pdu)
// Gère le traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
// et d'acquittement, etc.) puis insère les données utiles du PDU dans le buffer 
// de réception du socket. Cette fonction utilise la fonction app_buffer_add().   
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
}
