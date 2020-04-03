#ifndef MICTCP_CORE_H
#define MICTCP_CORE_H

#include <mictcp.h>
#include <math.h>

/**************************************************************
 * Public core functions, can be used for implementing mictcp *
 **************************************************************/

int initialize_components(start_mode sm);

int IP_send(mic_tcp_pdu, mic_tcp_sock_addr);
int IP_recv(mic_tcp_pdu*, mic_tcp_sock_addr*, unsigned long timeout);
int app_buffer_get(mic_tcp_payload);
void app_buffer_put(mic_tcp_payload);

void set_loss_rate(unsigned short);
unsigned long get_now_time_msec();
unsigned long get_now_time_usec();

/**********************************************************************
 * Private core functions, should not be used for implementing mictcp *
 **********************************************************************/
#ifndef API_CS_Port
  #define API_CS_Port 8524
#endif
#ifndef API_SC_Port
  #define API_SC_Port 8525
#endif
#define API_HD_Size 15

typedef struct ip_payload
{
  char* data; /* données transport */
  int size; /* taille des données */
} ip_payload;

int mic_tcp_core_send(mic_tcp_payload);
mic_tcp_payload get_full_stream(mic_tcp_pdu);
mic_tcp_payload get_mic_tcp_data(ip_payload);
mic_tcp_header get_mic_tcp_header(ip_payload);
void* listening(void*);
void print_header(mic_tcp_pdu);

int min_size(int, int);
float mod(int, float);

#endif
