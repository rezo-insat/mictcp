#ifndef MICTCP_CORE_H
#define MICTCP_CORE_H

#include <mictcp.h>
#include <math.h>

/**************************************************************
 * Public core functions, can be used for implementing mictcp *
 **************************************************************/

int initialize_components(start_mode sm);

int IP_send(mic_tcp_pdu, mic_tcp_sock_addr);
int IP_recv(mic_tcp_payload*, mic_tcp_sock_addr*, unsigned long delay);
int app_buffer_get(mic_tcp_payload);
void app_buffer_set(mic_tcp_payload);

void set_loss_rate(unsigned short);
unsigned long get_now_time_msec();
unsigned long get_now_time_usec();

/**********************************************************************
 * Private core functions, should not be used for implementing mictcp *
 **********************************************************************/
int full_send(mic_tcp_payload);
int partial_send(mic_tcp_payload);
mic_tcp_payload get_full_stream(mic_tcp_pdu);
mic_tcp_payload get_data_stream(mic_tcp_payload);
mic_tcp_header get_header(char*);
void* listening(void*);
void print_header(mic_tcp_payload);

int min_size(int, int);
float mod(int, float);

#endif
