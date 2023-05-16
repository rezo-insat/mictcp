// /*
//  * Permet de créer un socket entre l’application et MIC-TCP
//  * Retourne le descripteur du socket ou bien -1 en cas d'erreur
//  */
// int mic_tcp_socket(start_mode sm) {
//    printf("[MIC-TCP] Appel de la fonction: ");  
//    printf(__FUNCTION__); printf("\n");
   
//    result = initialize_components(sm); /* Appel obligatoire */
//    return -1; }
// /*
//  * Permet d’attribuer une adresse à un socket.
//  * Retourne 0 si succès, et -1 en cas d’échec
//  */
// int mic_tcp_bind(int socket, mic_tcp_sock_addr addr) {
//    printf("[MIC-TCP] Appel de la fonction: ");  
//    printf(__FUNCTION__); printf("\n");
//    return -1; }
// /*
//  * Met le socket en état d'acceptation de connexion
//  * Retourne 0 si succès, -1 si erreur
//  */
// int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr) {
// 	printf("[MIC-TCP] Appel de la fonction: ");  
// 	printf(__FUNCTION__); printf("\n");
// 	return -1; }
// /*
//  * Permet de réclamer l’établissement d’une connexion
//  * Retourne 0 si la connexion est établie, et -1 en cas d’échec
//  */
// int mic_tcp_connect (int socket, mic_tcp_sock_addr addr) {
// printf("[MIC-TCP] Appel de la fonction: ");  
// printf(__FUNCTION__); printf("\n");
// return -1; }
// /*
//  * Permet de réclamer l’envoi d’une donnée applicative
//  * Retourne la taille des données envoyées, et -1 en cas d'erreur
//  */
// int mic_tcp_send (int mic_sock, char* mesg, int mesg_size) {
// 	printf("[MIC-TCP] Appel de la fonction: "); 
// 	printf(__FUNCTION__); printf("\n");
	
// 	int sent_size= -1;
// 	mic_tcp_PDU pdu;
// 	pdu.header = ...
// 	pdu.payload.data = mesg;
// 	pdu.payload.size = mesg_size;
	
// 	//IP_recv quand fiabilité implémentée
	
// 	sent_size = IP_send(pdu, sock_addr) 
// 	return sent_size;
// }

// /*
//  * Permet à l’application réceptrice de réclamer la récupération d’une donnée
//  * stockée dans les buffers de réception du socket
//  * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
//  * NB : cette fonction fait appel à la fonction app_buffer_get()
//  */
// int mic_tcp_recv (int socket, char* mesg, int max_mesg_size) {
// 	printf("[MIC-TCP] Appel de la fonction: "); 
// 	printf(__FUNCTION__); printf("\n");
	
// 	int delivered_size = -1;
// 	mic_tcp_payload *payload;
// 	payload.data = mesg;
// 	payload.size = max_mesg_size;
// 	app_buffer_get(payload);
	
// 	return delivered_size;
	
// }
// /*
//  * Permet de traiter un PDU MIC-TCP reçu (mise à jour des numéros de séquence
//  * et d'acquittement, etc.) puis d'insérer les données utiles du PDU dans
//  * le buffer de réception du socket. Cette fonction utilise la fonction
//  * app_buffer_put(). Elle est appelée par initialize_components()
//  */
// void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr) {
//     printf("[MIC-TCP] Appel de la fonction: "); 
// 	printf(__FUNCTION__); printf("\n");
	
// 	//éventuellement trouver les bons socket, buffer et app
// 	app_buffer_put(PDU.body);
	
// 	/* Utile quand on fera du reliable
// 	if(SeqPDU = SeqDist){ //packet valide
// 		SendAck(SeqDist);
// 		SeqDist++;
// 		IP_send(ack);
// 	}
// 	*/ 
	
// }
// /*
//  * Permet de réclamer la destruction d’un socket.
//  * Engendre la fermeture de la connexion suivant le modèle de TCP.
//  * Retourne 0 si tout se passe bien et -1 en cas d'erreur
//  */
// int mic_tcp_close (int socket) {
// 	printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
// 	return -1; }






// // STRUCTURES 







// /* États du protocole ( noms des états donnés à titre indicatif => peuvent être modifiés) */
// typedef enum protocol_state {
// IDLE, CONNECTED, ...
// } protocol_state;
// /* Mode de démarrage du protocole. NB : nécessaire à l’usage de la fonction initialize_components() */
// typedef enum start_mode { CLIENT, SERVER } start_mode;
// /* Structure d’une adresse de socket */
// typedef struct mic_tcp_sock_addr {
// 	char *ip_addr; // @ IP : à fournir sous forme décimale pointée : ex = “192.168.1.2”
// 	int ip_addr_size; // taille de l’adresse
// 	unsigned short port; // numéro de port
// } mic_tcp_sock_addr;
// /*
//  * Structure d'un socket
//  */
// typedef struct mic_tcp_sock {
//   int fd;  /* descripteur du socket */
//   protocol_state state; /* état du protocole */
//   mic_tcp_sock_addr addr; /* adresse du socket */
// } mic_tcp_sock;
// /*
//  * Structure d'un PDU MIC-TCP
//  */
// typedef struct mic_tcp_pdu {
//   mic_tcp_header header ; /* entête du PDU */
//   mic_tcp_payload payload; /* charge utile du PDU */
// } mic_tcp_pdu;
// /*
//  * Structure de l'entête d'un PDU MIC-TCP
//  */
// typedef struct mic_tcp_header
// {
//   unsigned short source_port; /* numéro de port source */
//   unsigned short dest_port; /* numéro de port de destination */
//   unsigned int seq_num; /* numéro de séquence */
//   unsigned int ack_num; /* numéro d'acquittement */
//   unsigned char syn; /* flag SYN (valeur 1 si activé et 0 si non) */
//   unsigned char ack; /* flag ACK (valeur 1 si activé et 0 si non) */
//   unsigned char fin; /* flag FIN (valeur 1 si activé et 0 si non) */
// } mic_tcp_header;
// /*
//  * Structure des données utiles d’un PDU MIC-TCP
//  */
// typedef struct mic_tcp_payload {
//   char* data; /* données applicatives */
//   int size; /* taille des données */
// } mic_tcp_payload;
