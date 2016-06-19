#include <api/mictcp_core.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <strings.h>

/*****************
 * API Variables *
 *****************/
int first_free = 0;
int initialized = -1;
int sys_socket;
unsigned short API_CS_Port = 8524;
unsigned short API_SC_Port = 8525;
float reliability = 100;
mic_tcp_sock current_socket;
int exite_status = -1;
pthread_t listen_th;
pthread_mutex_t lock;
unsigned int global_id = 0;
unsigned short  loss_rate = 0;
float jump = 0;
float range = 0;
int count = 0;
int reverse = 1;

struct sockaddr_in local_addr, remote_addr, tmp_addr;
socklen_t local_size, remote_size; 
socklen_t tmp_addr_size = sizeof(struct sockaddr);

app_buffer* app_buffer_first = NULL;
app_buffer* app_buffer_last = NULL;
unsigned int app_buffer_size = 0;
unsigned int app_buffer_count = 0;

/*************************
 * Fonctions Utilitaires *
 *************************/
int initialize_components(start_mode mode)
{
    int bnd;
    struct hostent * hp;
    if(initialized != -1) return initialized;    
    if((sys_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) return -1;
    else initialized = 1;
    
    if((mode == SERVER) & (initialized != -1))
    {        
        memset((char *) &local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(API_CS_Port);
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bnd = bind(sys_socket, (struct sockaddr *) &local_addr, sizeof(local_addr));
         
        if (bnd == -1)
        {
            initialized = -1;
        }
        else
        {
            memset((char *) &remote_addr, 0, sizeof(local_addr));
            remote_addr.sin_family = AF_INET;
            remote_addr.sin_port = htons(API_SC_Port);
            hp = gethostbyname("localhost");
            memcpy (&(remote_addr.sin_addr.s_addr), hp->h_addr, hp->h_length);
            remote_size = sizeof(remote_addr);
            initialized = 1;
        } 
        
        
    }
    else
    {
        if(initialized != -1)
        {
            memset((char *) &remote_addr, 0, sizeof(local_addr));
            remote_addr.sin_family = AF_INET;
            remote_addr.sin_port = htons(API_CS_Port);
            hp = gethostbyname("localhost");
            memcpy (&(remote_addr.sin_addr.s_addr), hp->h_addr, hp->h_length);
            remote_size = sizeof(remote_addr);
            
            memset((char *) &local_addr, 0, sizeof(local_addr));
            local_addr.sin_family = AF_INET;
            local_addr.sin_port = htons(API_SC_Port);
            local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            bnd = bind(sys_socket, (struct sockaddr *) &local_addr, sizeof(local_addr));
        }
    }
    
    if((initialized == 1) && (mode == SERVER))
    {
        pthread_create (&listen_th, NULL, listening, "1");
    }
    
    return initialized;
}
    


int IP_send(mic_tcp_pdu pk, mic_tcp_sock_addr addr)
{
    if(initialized == -1) return -1;
    if(loss_rate == 0)
    {
        mic_tcp_payload tmp = get_full_stream(pk);        
        int sent_size =  full_send(tmp);
        
        free (tmp.data);
        
        return sent_size;             
    }
    else return partial_send(get_full_stream(pk));
}

int IP_recv(mic_tcp_payload* pk,mic_tcp_sock_addr* addr, unsigned long delay)
{
    struct timeval tv;

    /* Send data over a fake IP */
    if(initialized == -1) return -1;
    
    if(delay == 0) {tv.tv_sec = 3600; tv.tv_usec = 0;}    
    else {tv.tv_sec = 0; tv.tv_usec = delay;}  
    if (setsockopt(sys_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        return -1;
    }    
    
    else
    {   
        int recv_size = recvfrom(sys_socket, pk->data, pk->size, 0, (struct sockaddr *)&tmp_addr, &tmp_addr_size);
        pk->size = recv_size;
        return recv_size;
    }
    
}

mic_tcp_payload get_full_stream(mic_tcp_pdu pk)
{
    /* Get a full packet from data and header */
    mic_tcp_payload tmp;
    tmp.size = 15 + pk.payload.size;
    tmp.data = malloc (tmp.size);
    
    memcpy (tmp.data, &pk.header, 15);
    memcpy (tmp.data + 15, pk.payload.data, pk.payload.size);
    
    return tmp;
}

mic_tcp_payload get_data_stream(mic_tcp_payload buff)
{
    mic_tcp_payload tmp;
    tmp.data = malloc(buff.size-15);
    memcpy(tmp.data, buff.data+15, tmp.size);
    return tmp;
}
    

mic_tcp_header get_header(char* packet)
{
    /* Get a struct header from an incoming packet */
    mic_tcp_header tmp;
    memcpy(&tmp, packet, 15);
    return tmp;
}

mic_tcp_payload get_data(mic_tcp_payload packet)
{
    mic_tcp_payload tmp;
    tmp.size = packet.size - 15;
    tmp.data = malloc(tmp.size);
    memcpy(tmp.data, packet.data + 15, tmp.size);
    return tmp;
}

int full_send(mic_tcp_payload buff)
{
    return sendto(sys_socket, buff.data, buff.size, 0, (struct sockaddr *)&remote_addr, remote_size);
}

int partial_send(mic_tcp_payload buff)
{
    count ++;  
    
    if(range == 0)
    {
        jump = 1000.0 /((float) loss_rate);
        range += jump;
    }
    
    
    if(((int)range) == count)
    {
        reverse = -reverse;        
        jump = 1000.0 /((float) loss_rate);
        range += jump + ( (jump / 4) * reverse);
        printf("\n  --  Lost\n");
        return buff.size;
    }   
    else
    {
        return sendto(sys_socket, buff.data, buff.size, 0, (struct sockaddr *)&remote_addr, remote_size);
    } 
	
}

int app_buffer_get(mic_tcp_payload app_buff)
{
    mic_tcp_payload tmp;
    app_buffer* current;

    while(app_buffer_count == 0)
    {
        usleep(1000);
    }

    if(app_buffer_count > 0)    
    {
        pthread_mutex_lock(&lock);
        tmp.size = app_buffer_first->packet.size;
        tmp.data = app_buffer_first->packet.data;

        current = app_buffer_first;
        if(app_buffer_count == 1)
        {
            app_buffer_first = app_buffer_last = NULL;
        }
        else
        {            
            app_buffer_first = app_buffer_first->next;
        }

        memcpy(app_buff.data, tmp.data, min_size(tmp.size, app_buff.size));

        app_buffer_size -= tmp.size;
        app_buffer_count --;
        free(current);
        free(tmp.data);
        
        pthread_mutex_unlock(&lock);  
        
    }      

    return tmp.size;       
}

void app_buffer_set(mic_tcp_payload bf)
{
    
    pthread_mutex_lock(&lock);
    
    app_buffer* tmp = malloc(sizeof(app_buffer));
    tmp->packet.size = bf.size;
    tmp->packet.data = malloc(bf.size);
    tmp->id = global_id++;
    memcpy(tmp->packet.data, bf.data, bf.size);
    tmp->next = NULL; 
    
    if(app_buffer_count == 0)
    {
        app_buffer_first = app_buffer_last = tmp; 
    }
    else
    {
        app_buffer_last->next = tmp;
        app_buffer_last = tmp;   
    }
    
    app_buffer_size += bf.size;
    app_buffer_count ++;
    
    pthread_mutex_unlock(&lock);
}



void* listening(void* arg)
{   
    mic_tcp_pdu pdu_tmp;
    int recv_size; 
    mic_tcp_sock_addr remote;
    mic_tcp_payload tmp_buff;
    
    pthread_mutex_init(&lock, NULL);

    printf("[MICTCP-CORE] Demarrage du thread de reception reseau...\n");
        
    tmp_buff.size = 1500;
    tmp_buff.data = malloc(1500);
    
   
    while(1)
    {     
        tmp_buff.size = 1500;
        recv_size = IP_recv(&tmp_buff, &remote, 0);       
        
        if(recv_size > 0)
        {
            pdu_tmp.header = get_header (tmp_buff.data);
            pdu_tmp.payload = get_data (tmp_buff);
            process_received_PDU(pdu_tmp);
        }
    }    
}


void set_loss_rate(unsigned short rate)
{
    loss_rate = rate;
}

void print_header(mic_tcp_payload bf)
{
    mic_tcp_header hd = get_header(bf.data);
    printf("\nSP: %d, DP: %d, SEQ: %d, ACK: %d, Count: %d, Size: %d.", hd.source_port, hd.dest_port, hd.seq_num, hd.ack_num, app_buffer_count, app_buffer_size);
}

unsigned long get_now_time_msec()
{
    return ((unsigned long) (get_now_time_usec() / 1000));
}

unsigned long get_now_time_usec()
{
    struct timespec now_time;
    clock_gettime( CLOCK_REALTIME, &now_time);
    return ((unsigned long)((now_time.tv_nsec / 1000) + (now_time.tv_sec * 1000000)));
}

int min_size(int s1, int s2)
{
    if(s1 <= s2) return s1;
    return s2;
}
