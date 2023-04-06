#include <mictcp.h>
#include <api/mictcp_core.h>
#define NBR_SOCKETS 1024
#define TIMEOUT_DEFAUT 1
// FONCTIONS
typedef struct
{
	mic_tcp_sock socket;
	mic_tcp_sock_addr dist_addr;
	char NoSeqLoc;	// = -1;
	char NoSeqDist; // = -1;
} enhanced_socket;

static int socket_desc = 0;
static enhanced_socket tab_sockets[NBR_SOCKETS];
int timeout = TIMEOUT_DEFAUT;

int valid_socket(int socket);
int bound_socket(int socket);
int same_addr(mic_tcp_sock_addr *addr1, mic_tcp_sock_addr *addr2);
void display_mic_tcp_pdu(mic_tcp_pdu pdu, char* prefix);
void display_enhanced_socket(enhanced_socket sock, char* prefix);
void display_mic_tcp_sock_addr(mic_tcp_sock_addr addr, char* prefix);
/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
	printf("[MIC-TCP] Appel de la fonction: ");
	printf(__FUNCTION__);
	printf("\n");

	enhanced_socket *socket = &tab_sockets[socket_desc];

	socket->socket.fd = socket_desc++;
	socket->socket.state = IDLE;
	// not bound yet
    display_enhanced_socket(*socket, "état du socket");
	int result = initialize_components(sm); /* Appel obligatoire */
	if (result < 0)
	{
		return -1;
	}

	return socket->socket.fd;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
	printf("[MIC-TCP] Appel de la fonction: ");
	printf(__FUNCTION__);
	printf("\n");

	if (valid_socket(socket) && tab_sockets[socket].socket.state == IDLE)
	{
		tab_sockets[socket].socket.addr = addr;
		tab_sockets[socket].socket.state = WAITING;
        display_enhanced_socket(tab_sockets[socket],"");

		return 0;
	}

	return -1;
}

/*
 * Met le socket en état d'acceptation de connexion
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr *addr)
{
	printf("[MIC-TCP] Appel de la fonction: ");
	printf(__FUNCTION__);
	printf("\n");

	if (valid_socket(socket) && tab_sockets[socket].socket.state == WAITING)
	{
		tab_sockets[socket].socket.state = ACCEPTING;
		return 0;
	}
	return -1;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */


int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
	// if(!bound_socket(socket)){
	// 	printf("Socket not bound, cannot connect to specified address\n");
	// 	exit(1);
	// }
	tab_sockets[socket].dist_addr = addr;
	// tab_sockets[socket].NoSeqDist = 0;
	tab_sockets[socket].NoSeqLoc = 0;
	// create pdu syn
	mic_tcp_pdu pdu;
	pdu.header.source_port = tab_sockets[socket].socket.addr.port;
	pdu.header.dest_port = tab_sockets[socket].dist_addr.port;
	pdu.header.seq_num = tab_sockets[socket].NoSeqLoc;
	pdu.header.ack_num = -1;
	pdu.header.syn = 1;
	pdu.header.ack = 0;
	pdu.header.fin = 0;


	pdu.payload.data = NULL;
	pdu.payload.size = 0;
    display_enhanced_socket(tab_sockets[socket], "état du socket");
    display_mic_tcp_pdu(pdu, "envoi du pdu :");
    printf("À l'adresse :\n Remote Address IP: %.*s\n", addr.ip_addr_size, addr.ip_addr);
	printf("Remote Address Port: %hu\n", addr.port);

    IP_send(pdu, tab_sockets[socket].dist_addr);
	tab_sockets[socket].socket.state = SYN_SENT;
	while (1)
	{
		sleep(timeout);
        printf("timeout\n");
        printf("Socket state : %d, SYN SENT = %d\n",tab_sockets[socket].socket.state,SYN_SENT );
		if (tab_sockets[socket].socket.state == SYN_SENT)
		{
			IP_send(pdu, tab_sockets[socket].dist_addr);
		}
		else if (tab_sockets[socket].socket.state == ESTABLISHED)
		{
			break;
		}
	}

	return 0;
}

/*
 * Permet de réclamer l’NULL
*/

int mic_tcp_send(int mic_sock, char *mesg, int mesg_size)
{

	if (tab_sockets[mic_sock].socket.state != ESTABLISHED)
	{
		printf("non non non, t’es pas connecté \n");
		exit(1);
	}
	// create pdu
	mic_tcp_pdu pdu;
	pdu.header.source_port = tab_sockets[mic_sock].socket.addr.port;
	pdu.header.dest_port = tab_sockets[mic_sock].dist_addr.port;
	pdu.header.seq_num = tab_sockets[mic_sock].NoSeqLoc;
	pdu.header.ack_num = -1;
	pdu.header.syn = 0;
	pdu.header.ack = 0;
	pdu.header.fin = 0;

	pdu.payload.data = mesg;
	pdu.payload.size = mesg_size;

	IP_send(pdu, tab_sockets[mic_sock].dist_addr);
	tab_sockets[mic_sock].NoSeqLoc = (tab_sockets[mic_sock].NoSeqLoc + 1) % 2;

	tab_sockets[mic_sock].socket.state = WAITING;
	int sent_size = IP_send(pdu, tab_sockets[mic_sock].dist_addr);

	while (1)
	{
		sleep(timeout);
		if (tab_sockets[mic_sock].socket.state == WAITING)
		{
			sent_size = IP_send(pdu, tab_sockets[mic_sock].dist_addr);
		}
		else if (tab_sockets[mic_sock].socket.state == ESTABLISHED)
		{
			break;
		}
	}
	return sent_size;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv(int socket, char *mesg, int max_mesg_size)
{
	printf("[MIC-TCP] Appel de la fonction: ");
	printf(__FUNCTION__);
	printf("\n");

	if (!valid_socket(socket))
	{
		return -1;
	}

	int delivered_size;
	mic_tcp_payload *payload = malloc(sizeof(mic_tcp_payload));
	payload->data = mesg;
	payload->size = max_mesg_size;
    printf("procède au app_buffer_get\n");
	delivered_size = app_buffer_get(*payload);
	printf("payload récupéré\n");
	return delivered_size;
}

/*
 * Permet de traiter un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis d'insérer les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put(). Elle est appelée par initialize_components()
 */

void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr) //addr = adresse distante
{

	int mic_sock;  

    display_mic_tcp_pdu(pdu, "pdu reçu");

	for (mic_sock = 0; (mic_sock < socket_desc) && !(tab_sockets[mic_sock].socket.addr.port==pdu.header.dest_port); mic_sock++)
		; // trouver le socket destinataire
	if (mic_sock == socket_desc)
	{ // si aucun socket trouvé, alors retourne une erreur
		printf("socket non trouvé\n");
	}

	
	if (pdu.header.syn == 1 && tab_sockets[mic_sock].socket.state == ACCEPTING) // si PDU SYN
	{	
		printf("PDU SYN recu\n");
		tab_sockets[mic_sock].socket.state = SYN_RECEIVED;
		mic_tcp_pdu pdu_r;
		tab_sockets[mic_sock].dist_addr = addr;
		pdu_r.header.source_port = tab_sockets[mic_sock].socket.addr.port;
		pdu_r.header.dest_port = tab_sockets[mic_sock].dist_addr.port;
		pdu_r.header.seq_num = -1;
		pdu_r.header.ack_num = tab_sockets[mic_sock].NoSeqDist;
		pdu_r.header.syn = 1;
		pdu_r.header.ack = 1;
		pdu_r.header.fin = 0;

		pdu_r.payload.size = 0;
		pdu_r.payload.data = NULL;
        
        display_mic_tcp_sock_addr(tab_sockets[mic_sock].dist_addr, "à l'adresse:");
		IP_send(pdu_r, tab_sockets[mic_sock].dist_addr);
		tab_sockets[mic_sock].NoSeqLoc = (tab_sockets[mic_sock].NoSeqLoc + 1) % 2;
		tab_sockets[mic_sock].NoSeqDist = pdu.header.seq_num;

		tab_sockets[mic_sock].socket.state = SYN_RECEIVED;

		display_mic_tcp_pdu(pdu_r, "Pdu SYN ACK : \n");
		while (1)
		{

			printf("PDU SYN ACK envoyé\n");
			sleep(timeout);
			if (tab_sockets[mic_sock].socket.state == SYN_RECEIVED)
			{
				IP_send(pdu, tab_sockets[mic_sock].dist_addr);
			}
			else if (tab_sockets[mic_sock].socket.state == ESTABLISHED)
			{
				break;
			}
		}
	}
	else if (pdu.header.ack == 1 && pdu.header.syn == 1 && tab_sockets[mic_sock].socket.state == SYN_SENT)
	{ // si SYN ACK reçu envoyer ACK

		printf("PDU SYN ACK recu\n");
		tab_sockets[mic_sock].socket.state = ESTABLISHED;
		tab_sockets[mic_sock].NoSeqDist = pdu.header.seq_num;
		mic_tcp_pdu pdu_r;

		pdu_r.header.source_port = tab_sockets[mic_sock].socket.addr.port;
		pdu_r.header.dest_port = tab_sockets[mic_sock].dist_addr.port;
		pdu_r.header.seq_num = -1;
		pdu_r.header.ack_num = tab_sockets[mic_sock].NoSeqDist;
		pdu_r.header.syn = 0;
		pdu_r.header.ack = 1;
		pdu_r.header.fin = 0;

		pdu_r.payload.size = 0;
		pdu_r.payload.data = NULL;

		IP_send(pdu_r, tab_sockets[mic_sock].dist_addr);

		printf("PDU ACK de connexion envoyé\n");
	}
	else if (pdu.header.ack == 1 && pdu.header.syn == 1 && tab_sockets[mic_sock].socket.state == ESTABLISHED)
	{ // si SYN ACK reçu à nouveau renvoyer ACK

		printf("PDU SYN ACK recu a nouveau\n");
		mic_tcp_pdu pdu_r;

		pdu_r.header.source_port = tab_sockets[mic_sock].socket.addr.port;
		pdu_r.header.dest_port = tab_sockets[mic_sock].dist_addr.port;
		pdu_r.header.seq_num = -1;
		pdu_r.header.ack_num = tab_sockets[mic_sock].NoSeqDist;
		pdu_r.header.syn = 0;
		pdu_r.header.ack = 1;
		pdu_r.header.fin = 0;

		pdu_r.payload.size = 0;
		pdu_r.payload.data = NULL;

		IP_send(pdu_r, tab_sockets[mic_sock].dist_addr);
		
	}
	else if(pdu.header.ack == 0 && pdu.header.seq_num == tab_sockets[mic_sock].NoSeqLoc && tab_sockets[mic_sock].socket.state == ESTABLISHED){
		printf("PDU dat recu \n");
		mic_tcp_pdu pdu_r;
        pdu_r.header.source_port = tab_sockets[mic_sock].socket.addr.port;
		pdu_r.header.dest_port = tab_sockets[mic_sock].dist_addr.port;
		pdu_r.header.seq_num = -1;
		pdu_r.header.ack_num = tab_sockets[mic_sock].NoSeqDist;
		pdu_r.header.syn = 0;
		pdu_r.header.ack = 1;
		pdu_r.header.fin = 0;
		tab_sockets[mic_sock].NoSeqDist = (tab_sockets[mic_sock].NoSeqDist) + 1 % 2;
		IP_send(pdu_r, tab_sockets[mic_sock].dist_addr);
		app_buffer_put(pdu.payload); // envoyer la data dans le buffer
		printf("data in the buffer\n");
	}
	else if (pdu.header.ack == 1 && tab_sockets[mic_sock].socket.state == WAITING && (tab_sockets[mic_sock].NoSeqLoc == pdu.header.ack_num))
	{
		// si ACK de bon numéro de séquence passer au n° de seq suivant
		tab_sockets[mic_sock].NoSeqLoc = (tab_sockets[mic_sock].NoSeqLoc) + 1 % 2;
		tab_sockets[mic_sock].socket.state = ESTABLISHED;
	}
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close(int socket)
{
	printf("[MIC-TCP] Appel de la fonction :  ");
	printf(__FUNCTION__);
	printf("\n");

	if (valid_socket(socket) && tab_sockets[socket].socket.state == ESTABLISHED)
	{
		tab_sockets[socket].socket.state = CLOSED;
		tab_sockets[socket].socket.fd = -1;
	}
	return -1;
}

/*---------------------------------------------*/
int valid_socket(int socket)
{
	if (socket > socket_desc - 1 || tab_sockets[socket].socket.fd == -1)
	{
		// printf("ah, ton socket il existe pas\n");
		return 0;
	}
	return 1;
}

int bound_socket(int socket)
{
	if (tab_sockets[socket].socket.state == IDLE || tab_sockets[socket].socket.state == CLOSING || tab_sockets[socket].socket.state == CLOSED )
	{
		// printf("socket not bound\n");
		return 0;
	}
	return 1;
}

int same_addr(mic_tcp_sock_addr *addr1, mic_tcp_sock_addr *addr2)
{
	if (addr1->port == addr2->port && addr1->ip_addr_size == addr2->ip_addr_size && !memcmp(addr1->ip_addr, addr2->ip_addr, addr1->ip_addr_size))
	{
		return 1;
	}
	return 0;
}

void display_mic_tcp_pdu(mic_tcp_pdu pdu, char* prefix) {
	printf("------------------------------------------\n%s\n", prefix);
	printf("Source Port: %hu\n", pdu.header.source_port);
	printf("Destination Port: %hu\n", pdu.header.dest_port);
	printf("Sequence Number: %u\n", pdu.header.seq_num);
	printf("Acknowledgment Number: %u\n", pdu.header.ack_num);
	printf("SYN Flag: %d\n", pdu.header.syn);
	printf("ACK Flag: %d\n", pdu.header.ack);
	printf("FIN Flag: %d\n", pdu.header.fin);
	printf("Payload Size: %d\n", pdu.payload.size);
	if (pdu.payload.data != NULL) {
	printf("Payload Data: %.*s\n", pdu.payload.size, pdu.payload.data);
	} else {
	printf("Payload Data is NULL\n");
	}
    printf("----------------------------------\n");
}


void display_enhanced_socket(enhanced_socket sock,char* prefix) {
	printf("----------------------------------\n");
	printf("%s\n", prefix);
	printf("Socket File Descriptor: %d\n", sock.socket.fd);
	
	const char* protocol_states[] = {"IDLE", "CLOSED", "SYN_SENT", "SYN_RECEIVED", "ESTABLISHED", "CLOSING", "ACCEPTING", "WAITING"};
	
	
	printf("Socket Protocol State: %s\n", protocol_states[sock.socket.state]);
	
	printf("Socket Address IP: %.*s\n", sock.socket.addr.ip_addr_size, sock.socket.addr.ip_addr);
	printf("Socket Address Port: %hu\n", sock.socket.addr.port);
	printf("Remote Address IP: %.*s\n", sock.dist_addr.ip_addr_size, sock.dist_addr.ip_addr);
	printf("Remote Address Port: %hu\n", sock.dist_addr.port);
	printf("Local Sequence Number: %d\n", sock.NoSeqLoc);
	printf("Remote Sequence Number: %d\n", sock.NoSeqDist);
	printf("----------------------------------\n");
}

void display_mic_tcp_sock_addr(mic_tcp_sock_addr addr, char* prefix) {
	printf("%s\n", prefix);
	printf("IP Address: %.*s\n", addr.ip_addr_size, addr.ip_addr);
  	printf("Port: %hu\n", addr.port);
}
