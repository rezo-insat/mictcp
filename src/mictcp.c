// V3 avec vrai pourcentage 

#include <mictcp.h>
#include <api/mictcp_core.h>

#define TIMEOUT 10
#define TIMEOUT2 100
#define SIZE 100
#define LOST 10
#define LOSTRATE "15"

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
    mic_tcp_sock sock;
    mic_tcp_pdu pdu;
    int PE,PA = 0;
    float lostpdu = 0.0;
    float lostrate = 15.0;
    
    int pduemis =0;
    int fenetre[SIZE]; 
    void init (int* tab,int size){
        for(int i=0;i<size;i++){
            tab[i]=0;
        }
    }
    float rate(int* tab, int size){
        float s =0.0;
        for(int i=0; i<size;i++){
            s += (float)tab[i];
        }
        return ((s/(float)SIZE)*100.0); 
    }
    pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond= PTHREAD_COND_INITIALIZER;
 
    
int mic_tcp_socket(start_mode sm)
{
    //ajouter return -1 en cas d'erreur
    int result=-1;
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    result = initialize_components(sm); /* Appel obligatoire */
    sock.fd=result;
    set_loss_rate(LOST);
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
    //printf("Je vais dormir\n"); 
    pthread_cond_wait(&cond,&mutex);
<<<<<<< HEAD
    //printf("Je suis réveillé\n");
    sock.state = CONNECTED; 
=======
    printf("Je suis réveillé\n");*/
>>>>>>> leo
    
    //retourne 0 si sock.state = established, -1 sinon
    if (sock.fd == socket){
        sock.state == CONNECTED;
        return 0;
    } else {
        return -1;
    }
    
    
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    //Encpsulation SYN 
    
    mic_tcp_pdu pdusyn;
    mic_tcp_pdu ack;

    int sent= -1;
    int max = 300;
    int count = 0;
    int test=0;
    
    pdusyn.header.seq_num = PE; // Création du header et setup PA et PE
    pdusyn.header.ack_num = PA;
    pdusyn.header.syn= 1; 
    pdusyn.header.ack=0;
    pdusyn.header.fin=0;
    pdusyn.payload.data= LOSTRATE;
    pdusyn.payload.size = 0; 
    
    pdu.header.ack = 0;
    pdu.header.syn = 0;
    printf("Test1\n");
    while(pdu.header.syn != 1 && pdu.header.ack !=1){ 
        
        if(count == max){return(-1);}
        if (count != 0)
            printf("Paquet message perdu \n");
        count++;
        printf("Test2\n");
        if((sent=IP_send(pdusyn,addr))==-1){
            printf("Erreur d'envoi du pdu\n");
            exit(1);
        }
        printf("Test2\n");
        test=(IP_recv(&pdu,&sock.addr,TIMEOUT2) ==-1);

        
        while(count < max && test){
            count++;
            if(count == max){return(-1);}
            if(IP_send(pdusyn,sock.addr)==-1){
                printf("Erreur d'envoi du pdu");
                return(1);
            }
            test=(IP_recv(&pdu,&sock.addr,TIMEOUT2) ==-1);
        }
        printf("Count : %d\n",count);
        if(count == max)
            exit(-1);
    }
    printf("Test2\n");
    ack.header.ack = 1;
    ack.header.ack_num= PA; 
    ack.header.seq_num=PE; 
    ack.header.syn=0;
    ack.header.fin=0; 

    ack.payload.data="";
    ack.payload.size=0; 

    if(IP_send(ack,sock.addr)==-1){
        printf("Erreur d'envoi du pdu");
        exit(1);
<<<<<<< HEAD
    }
=======
    }*/

    //se servir de addr et sock.addr + refaire condition pour avoir 0 ou -1 et c'est tout ??

>>>>>>> leo
    printf("Connected\n");
    init(fenetre,SIZE);
    sock.state=CONNECTED;
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des  données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
<<<<<<< HEAD
    int sent= -1;
=======
    
    //construction pdu
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;
    pdu.header.seq_num= PE;
     
    //payload
    pdu.payload.data = mesg;
    pdu.payload.size=mesg_size;

    mic_tcp_pdu mem = pdu; 
    //printf("%d\n",pdu.header.seq_num);

    //envoi du pdu
    if((sent=IP_send(pdu,sock.addr))==-1){
        printf("Erreur d'envoi du pdu");
        exit(1);
    }
    PE = (PE +1) %2;
>>>>>>> leo
    
    if(sock.state==CONNECTED){    
        int max = 20;
        int count = 0;
        int test;
        
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
        test = (IP_recv(&pdu,&sock.addr,TIMEOUT) ==-1);
        
        if (test){  
            fenetre[pduemis%SIZE]=1;
            printf("Lost Pdu : %f\n",lostpdu);
        }
        else{
            fenetre[pduemis%SIZE]=0;   
        }
        float truc =rate(fenetre,SIZE);
        printf("Lost rate= %f \n",truc);
        
        if(truc>= lostrate && test){
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
    //prendre en compte le cas d'erreur
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
    //faire cas -1 / 0
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
<<<<<<< HEAD
    if (sock.state == CONNECTED){
        printf("Paquet recu \n");
        mic_tcp_pdu ack; 
       
        if(pdu.header.seq_num == PA){
            app_buffer_put(pdu.payload);
            PA = (PA+1) %2;
        }else{
            printf("Paquet Ack perdu\n");
        }
        printf("avant send\n");
        ack.header.ack=1;
        ack.header.ack_num=PA;
        ack.payload.data="";
        ack.payload.size=0;
        if (IP_send(ack,addr)==-1)
        {
            printf("Erreur d'envoi du pdu");
            exit(1);
        }
        printf("Ack envoyé \n");
    }else{
=======
    printf("Paquet recu \n");
    printf("ACK NUM %d\n",pdu.header.seq_num);

    mic_tcp_pdu ack; 
    
    if(pdu.header.seq_num == PA){
        app_buffer_put(pdu.payload);
        PA = (PA+1) %2;
    }else{
        printf("Paquet Ack perdu\n");
    }

    //construction pdu ack
    ack.header.ack=1;
    ack.header.ack_num=PA;
    ack.header.fin=0;
    ack.header.syn=0;
    //init source port, dest port ??

    ack.payload.data =""; 
    ack.payload.size=0;

    if (IP_send(ack,addr)==-1)
    {
        printf("Erreur d'envoi du pdu");
        exit(1);
    }
    //printf("Ack envoyé \n");
        /*
        printf("Je me connecte\n");
>>>>>>> leo
        int test=0;
        int sent= -1;
        int max = 20;
        int count = 0;  
        mic_tcp_pdu paqu;  
        while(paqu.header.syn != 1){
            test=(IP_recv(&paqu,&sock.addr,TIMEOUT) ==-1); 
            if(test){}
                //printf("En attente\n");
        }
        mic_tcp_pdu sack;
        
        sack.header.seq_num = PE; // Création du header et setup PA et PE
        sack.header.ack_num = PA;
        sack.header.syn= 1; 
        sack.header.ack=1;
        sack.header.fin=0;
        
        sack.payload.data = pdu.payload.data;
        sack.payload.size = sizeof(pdu.payload.data); 

        while(paqu.header.ack != 1){
                
            if(count == max){exit(-1);}
            count++;
            if (count == 0)
                printf("Paquet message perdu \n");
            if((sent=IP_send(sack,sock.addr))==-1){
                printf("Erreur d'envoi du pdu");
                exit(1);
            }
            test=(IP_recv(&paqu,&sock.addr,TIMEOUT2) ==-1);  
            while(count < max && test){
                count++;
                if(count == max){exit(-1);}
                if(IP_send(sack,sock.addr)==-1){
                    printf("Erreur d'envoi du pdu");
                    exit(1);
                }
                test=(IP_recv(&paqu,&sock.addr,TIMEOUT2) ==-1);
            }

            printf("Count : %d\n",count);
            if(count == max)
                exit(-1);
        }
        printf("Connected\n");
        pthread_cond_broadcast(&cond);
        sock.state = CONNECTED;

    }
    
}
