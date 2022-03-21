// V3 avec vrai pourcentage 

#include <mictcp.h>
#include <api/mictcp_core.h>
#define TIMEOUT 100

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
    mic_tcp_sock sock;
    mic_tcp_pdu pdu;
    int PE,PA = 0;
    float lostpdu = 0;
    float lostrate = 10.0; 
    float pduemis =1;
    // lost sur 
int mic_tcp_socket(start_mode sm)
{
    int result=-1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   sock.fd=result;
   set_loss_rate(5);
   return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    if(sock.fd == socket){
        sock.addr=addr;
        sock.state= IDLE;
        return 0;
    }else {
        return -1;
    }     
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    sock.state=CONNECTED;
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    //Encpsulation SYN 
    /*
    mic_tcp_pdu syn;
    //syn.header.dest_port= ; 
    
    syn.header.seq_num = PE; // Création du header et setup PA et PE
    syn.header.ack_num = PA;
    syn.header.syn= 1; 
    syn.header.ack=0;
    syn.header.fin=0;
    
    syn.payload.data = ""; //Encapsulation du payload vide 
    syn.payload.size = 0;

    if(IP_send(pdu,sock.addr)==-1){
        printf("Erreur d'envoi de SYN");
        exit(1);
    }

    if(IP_recv(&pdu,&sock.addr,TIMEOUT)==-1){

    }

    */

    
    sock.state=CONNECTED;
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    int sent= 0;
    int max = 20;
    int count = 0;

    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
  
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;
    pdu.header.seq_num= PE;
     
    pdu.payload.data = mesg;
    pdu.payload.size=mesg_size;

    mic_tcp_pdu mem = pdu; 

    if((sent=IP_send(pdu,sock.addr))==-1){
        printf("Erreur d'envoi du pdu");
        exit(1);
    }
    PE = (PE +1) %2;
    pduemis++; 
    int test = (IP_recv(&pdu,&sock.addr,TIMEOUT) ==-1);
    
    if (test){  
        lostpdu++;
        printf("Lost Pdu : %f\n",lostpdu);
    }
    printf("Lost rate= %f \n",(lostpdu/pduemis)*100.0);
    
    if((lostpdu/pduemis)*100 > lostrate && test){
        while(count < max && test){
            count++; 
            printf("Count: %d \n", count);  
            if((sent=IP_send(pdu,sock.addr))==-1){
                printf("Erreur d'envoi du pdu");
                exit(-1);
            }
            test=(IP_recv(&pdu,&sock.addr,TIMEOUT) ==-1);  
        }
        if(count == max)
            exit(-1);
        count = 0;
        while(pdu.header.ack_num!=PE){
            count++;
            if(count == max){exit(-1);}
            printf("Paquet message perdu \n");
            
            if((sent=IP_send(pdu,sock.addr))==-1){
                printf("Erreur d'envoi du pdu");
                exit(1);
            }
            test=(IP_recv(&pdu,&sock.addr,TIMEOUT) ==-1);  
            while(count < max && test){
                count++;
                if(count == max){exit(-1);}
                if(IP_send(mem,sock.addr)==-1){
                    printf("Erreur d'envoi du pdu");
                    exit(1);
                }
                test=(IP_recv(&pdu,&sock.addr,TIMEOUT) ==-1);
            }
            printf("Count : %d\n",count);
            if(count == max)
                exit(-1);
        } 
    }

    return sent;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    
    pdu.payload.data=mesg;
    pdu.payload.size=max_mesg_size;
    return app_buffer_get(pdu.payload);
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    sock.state=CLOSED;
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    mic_tcp_pdu ack; 
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    if(pdu.header.seq_num == PA){
        app_buffer_put(pdu.payload);
        PA= (PA+1) %2;
    }else{
        printf("Paquet Ack perdu\n");
    }
   
    ack.header.ack=1;
    ack.header.ack_num=PA;
    if (IP_send(ack,addr)==-1)
    {
        printf("Erreur d'envoi du pdu");
        exit(1);
    }
    
    

}
