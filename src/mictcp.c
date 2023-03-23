#include <mictcp.h>
#include <api/mictcp_core.h>
#define NBR_SOCKETS 1024

// FONCTIONS


static int socket_desc = 0;
static mic_tcp_sock tab_sockets[NBR_SOCKETS];

int valid_socket(int socket);
int bound_socket(int socket);



/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm) {
   printf("[MIC-TCP] Appel de la fonction: ");  
   printf(__FUNCTION__); printf("\n");
   
   mic_tcp_sock *socket = &tab_sockets[socket_desc];
   
   
   socket->fd = socket_desc++;
   socket->state = IDLE;
  //not bound yet


   int result = initialize_components(sm); /* Appel obligatoire */
   if (result<0){
	return -1;
   }
   
   return socket->fd; 
}


/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr) {
   printf("[MIC-TCP] Appel de la fonction: ");  
   printf(__FUNCTION__); printf("\n");

	
	if(valid_socket(socket) && tab_sockets[socket].state ==IDLE){
   		tab_sockets[socket].addr = addr;
   		tab_sockets[socket].state = BOUND;
		
		return 0;
	}
   
   return -1; 
   }


/*
 * Met le socket en état d'acceptation de connexion
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr) {
	printf("[MIC-TCP] Appel de la fonction: ");  
	printf(__FUNCTION__); printf("\n");

	if(valid_socket(socket) && tab_sockets[socket].state == BOUND){
		tab_sockets[socket].state = ACCEPTING;
		return 0;
	}
	return -1; 
}


/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */

int mic_tcp_connect (int socket, mic_tcp_sock_addr addr) {
	printf("[MIC-TCP] Appel de la fonction: ");  
	printf(__FUNCTION__); printf("\n");

	
	if(valid_socket(socket) && tab_sockets[socket].state == IDLE){
		printf("%d\n", socket);
   		tab_sockets[socket].addr = addr;
		tab_sockets[socket].state = ESTABLISHED;
		return 0;
	}
   
   return -1;

}


/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */


int mic_tcp_send (int mic_sock, char* mesg, int mesg_size) {
	printf("[MIC-TCP] Appel de la fonction: "); 
	printf(__FUNCTION__); printf("\n");
	
	mic_tcp_sock socket=tab_sockets[mic_sock];
	mic_tcp_header header;

	// printf("%d\n", mic_sock);
	if (socket.state!=ESTABLISHED){
		printf("connection not established\n");
		return -1;
	}

	header.dest_port=socket.addr.port;
	
	int sent_size = -1;
	mic_tcp_pdu pdu;
	pdu.header = header;
	pdu.payload.data = mesg;
	pdu.payload.size = mesg_size;
	
	//IP_recv quand fiabilité implémentée
	
	sent_size = IP_send(pdu, socket.addr) ;
	return sent_size;
}


/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size) {
	printf("[MIC-TCP] Appel de la fonction: "); 
	printf(__FUNCTION__); printf("\n");
	
	if(!valid_socket(socket)){
		return -1;
	}

	int delivered_size;
	mic_tcp_payload *payload;
	payload->data= mesg;
	payload->size = max_mesg_size;
	delivered_size = app_buffer_get(*payload);

	return delivered_size;
	
}


/*
 * Permet de traiter un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis d'insérer les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put(). Elle est appelée par initialize_components()
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr) {
    printf("[MIC-TCP] Appel de la fonction: "); 
	printf(__FUNCTION__); printf("\n");
	
	/*
	mic_tcp_sock_addr addr_emetteur = addr //pour clarification
	//éventuellement trouver les bons socket, buffer et app
	for (int iint valid_socket(int socket) =0; i<socket_desc; i++){
		if(tab_sockets[i].state==ACCEPTING && tab_sockets[i].addr == addr) {break}
		else if (i==socket_desc-1){return -1}
	}
	*/

	app_buffer_put(pdu.payload);
	
	/* Utile quand on fera du reliable
	if(SeqPDU = SeqDist){ //packet valide
		SendAck(SeqDist);
		SeqDist++;
		IP_send(ack);
	}
	*/ 
	
}
/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket) {
	printf("[MIC-TCP] Appel de la fonction :  "); 
	printf(__FUNCTION__); printf("\n");

	if(valid_socket(socket) && tab_sockets[socket].state == ESTABLISHED){
		tab_sockets[socket].state = CLOSED;
		tab_sockets[socket].fd = -1;
	}
	return -1; 
}


/*---------------------------------------------*/
int valid_socket(int socket){
	if(socket > socket_desc-1 || tab_sockets[socket].fd == -1){
		//printf("ah, ton socket il existe pas\n");
		return 0;
	}
	return 1;
}

int bound_socket(int socket){
	if (tab_sockets[socket].addr.ip_addr_size ==-1){
		//printf("socket not bound\n");
		return 0;
	}
	return 1;
}






