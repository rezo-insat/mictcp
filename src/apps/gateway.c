#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <mictcp.h>


extern errno;

#define MAX_UDP_SEGMENT_SIZE 1480
#define CUMULATED_BUFF_NB 1

/**
 * Function that performs UDP to TCP behavioral adaptation making it look like TCP was used.
 * Losses can be emulated by setting the loss paramter to 1.
 * returns void
 */
void udp_to_tcp(struct sockaddr_in listen_on, struct sockaddr_in transmit_to, int loss) {

     // Define the socket on which we listen and which we use for sending packets out
     int listen_sockfd;

     // A buffer used to store received segments before they are forwarded
     char buffer[MAX_UDP_SEGMENT_SIZE];
     

     // Addresses for the work to be performed
     struct sockaddr_in serv_addr = listen_on;
     struct sockaddr_in cliaddr;
     socklen_t len = sizeof(cliaddr);

     // We construct the socket to be used by this function
     listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

     if (bind(listen_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
          printf("ERROR on binding: ");
          perror(0);
          printf("\n");
     }

     listen(listen_sockfd,5);


     // Initialize a packet count variable used when loss emulation is active
     int count = 0;
     ssize_t n = -1;

     // Main activity loop, we never exit this, user terminates with SIGKILL
     while(1) {
          bzero(buffer,MAX_UDP_SEGMENT_SIZE);

          n = recvfrom(listen_sockfd, buffer, MAX_UDP_SEGMENT_SIZE, 0, (struct sockaddr *) &cliaddr, &len);
          if (n < 0) {
               perror(0); 
          }

          if(loss == 1) {
               // We emulate losses every 600 packets by delaying the processing by 2 seconds.
	       if(count++ == 600) {
                    printf("Simulating TCP loss\n");
                    sleep(2);
                    count = 0;
               }
          }

          // We forward the packet to its final destination
          n = sendto(listen_sockfd, buffer, n, 0, (struct sockaddr *) &transmit_to, len);
          if (n < 0) {
               perror(0);
          } 
     }

     // We never execute this but anyway, for sanity
     close(listen_sockfd);
} 

struct timespec tsSubtract (struct  timespec  time1, struct  timespec  time2) {    
    struct  timespec  result ;

    /* Subtract the second time from the first. */
    if ((time1.tv_sec < time2.tv_sec) || ((time1.tv_sec == time2.tv_sec) &&
			    (time1.tv_nsec <= time2.tv_nsec))) {
	    /* TIME1 <= TIME2? */
			        result.tv_sec = result.tv_nsec = 0 ;
    } else {	/* TIME1 > TIME2 */
					            result.tv_sec = time1.tv_sec - time2.tv_sec ;
						            if (time1.tv_nsec < time2.tv_nsec) {
								                result.tv_nsec = time1.tv_nsec + 1000000000L - time2.tv_nsec ;
										            result.tv_sec-- ;				/* Borrow a second. */
											            } else {
													                result.tv_nsec = time1.tv_nsec - time2.tv_nsec ;
															        }
							        }

		    return (result) ;

}

/**
 * Function that emulates TCP behavior while reading a file making it look like TCP was used.
 * Losses can be emulated by setting the loss paramter to 1.
 * returns void
 */
void file_to_tcp(struct sockaddr_in listen_on, struct sockaddr_in transmit_to, int loss) {

     // Define the socket on which we listen and which we use for sending packets out
     int listen_sockfd;

     // A buffer used to store received segments before they are forwarded
     char buffer[MAX_UDP_SEGMENT_SIZE];

     // Addresses for the work to be performed
     struct sockaddr_in serv_addr = listen_on;
     struct sockaddr_in cliaddr;
     socklen_t len = sizeof(cliaddr);

     // We construct the socket to be used by this function
     listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

     //if (bind(listen_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     //     printf("ERROR on binding: ");
     //     perror(0);
     //     printf("\n");
     //}

     //listen(listen_sockfd,5);


     // Initialize a packet count variable used when loss emulation is active
     int count = 0;
     ssize_t n = -1;

     FILE * fd = fopen("../video/video.bin", "rb");

     struct timespec currentTime;
     struct timespec lastTime;
     struct timespec rem;
     int firstValue = 0;

     // Main activity loop, we never exit this, user terminates with SIGKILL
     while(!feof(fd)) {
          bzero(buffer,MAX_UDP_SEGMENT_SIZE);

          n = fread(&currentTime, 1, sizeof(struct timespec), fd);
	  if(firstValue > 0) {
	       // We need to sleep a while
	       struct timespec difference = tsSubtract(currentTime, lastTime);
	       nanosleep(&difference, &rem);
	  } else {
	       firstValue++;
	  }
	  lastTime = currentTime;
          n = fread(buffer, 1, sizeof(n), fd);
          n = fread(buffer, 1, *((int *)buffer), fd);
          if (n < 0) {
               perror(0); 
          }

          if(loss == 1) {
               // We emulate losses every 600 packets by delaying the processing by 2 seconds.
	       if(count++ == 600) {
                    printf("Simulating TCP loss\n");
                    sleep(2);
                    count = 0;
               }
          }

          // We forward the packet to its final destination
          n = sendto(listen_sockfd, buffer, n, 0, (struct sockaddr *) &transmit_to, len);
          if (n < 0) {
               perror(0);
          } 
     }

     // We never execute this but anyway, for sanity
     close(listen_sockfd);
} 

/**
 * Function that reads a file and delivers to MICTCP.
 * returns void
 */
void file_to_mictcp(struct sockaddr_in listen_on, struct sockaddr_in transmit_to) {

     // Define the socket on which we listen
     int listen_sockfd;

     // A buffer used to store received segments before they are forwarded
     char buffer[MAX_UDP_SEGMENT_SIZE];

     // Addresses for the work to be performed
     struct sockaddr_in serv_addr = listen_on;
     struct sockaddr_in cliaddr;
     socklen_t len = sizeof(cliaddr);
     
     // MICTCP stuff
     int mic_tcp_sockfd;
     start_mode start_mode_mic_tcp = CLIENT;
     mic_tcp_sock_addr mic_tcp_dest_addr;
     mic_tcp_dest_addr.ip_addr = (char* ) &(transmit_to.sin_addr.s_addr);
     mic_tcp_dest_addr.port = transmit_to.sin_port;

     // We construct the UDP and MICTCP sockets to be used by this function
     listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     
     if((mic_tcp_sockfd = mic_tcp_socket(start_mode_mic_tcp)) == -1) {
          printf("ERROR creating the MICTCP socket\n");
     }

     // We now connect the MICTCP socket
     if(mic_tcp_connect(mic_tcp_sockfd, mic_tcp_dest_addr) == -1) {
          printf("ERROR connecting the MICTCP socket\n");
     }
     
//     if (bind(listen_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
//          printf("ERROR on binding the UDP socket: ");
//          perror(0);
//          printf("\n");
//     }

//     listen(listen_sockfd,5);


     // Initialize a packet count variable used when loss emulation is active
     int count = 0;
     ssize_t n = -1;

     FILE * fd = fopen("../video/video.bin", "rb");

     struct timespec currentTimeFile;
     struct timespec firstTimeFile;
     struct timespec lastTimeFile;
     struct timespec timeFirstPacket;
     struct timespec currentTime;
     struct timespec rem;
     int firstValue = 0;

     // Main activity loop, we never exit this, user terminates with SIGKILL
     while(1) {
          bzero(buffer, MAX_UDP_SEGMENT_SIZE);

	  n = fread(&currentTimeFile, 1, sizeof(struct timespec), fd);
	  if(firstValue > 0) {
	       // We need to sleep a while
               if( clock_gettime( CLOCK_REALTIME, &currentTime) == -1 ) {
	            perror( "clock gettime" );
	       }
	       struct timespec timeSinceFirstPacket = tsSubtract(currentTime, timeFirstPacket);
	       struct timespec timeSinceFirstTimeFile = tsSubtract(currentTimeFile, firstTimeFile);
	       struct timespec difference = tsSubtract(timeSinceFirstTimeFile, timeSinceFirstPacket);
	       nanosleep(&difference, &rem);
	  } else {
	       firstTimeFile = currentTimeFile;
               if( clock_gettime( CLOCK_REALTIME, &timeFirstPacket) == -1 ) {
	            perror( "clock gettime" );
	       }
	       firstValue++;
	  }
	  lastTimeFile = currentTimeFile;
          n = fread(buffer, 1, sizeof(n), fd);
          n = fread(buffer, 1, *((int *)buffer), fd);
          if (n < 0) {
               perror(0); 
          }

          // We forward the packet to its final destination
          n = mic_tcp_send(mic_tcp_sockfd, buffer, n); 
          if (n <= 0) {
               printf("ERROR on MICTCP send\n");
          } 
     }

     // We never execute this but anyway, for sanity
     close(listen_sockfd);
      
     // Same for MICTCP
     mic_tcp_close(mic_tcp_sockfd);
} 



/**
 * Function that listens on UDP and delivers to MICTCP.
 * returns void
 */
void udp_to_mictcp(struct sockaddr_in listen_on, struct sockaddr_in transmit_to) {

     // Define the socket on which we listen
     int listen_sockfd;

     // A buffer used to store received segments before they are forwarded
     char buffer[MAX_UDP_SEGMENT_SIZE];

     // Addresses for the work to be performed
     struct sockaddr_in serv_addr = listen_on;
     struct sockaddr_in cliaddr;
     socklen_t len = sizeof(cliaddr);
     
     // MICTCP stuff
     int mic_tcp_sockfd;
     start_mode start_mode_mic_tcp = CLIENT;
     mic_tcp_sock_addr mic_tcp_dest_addr;
     mic_tcp_dest_addr.ip_addr = (char* ) &(transmit_to.sin_addr.s_addr);
     mic_tcp_dest_addr.port = transmit_to.sin_port;

     // We construct the UDP and MICTCP sockets to be used by this function
     listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     
     if((mic_tcp_sockfd = mic_tcp_socket(start_mode_mic_tcp)) == -1) {
          printf("ERROR creating the MICTCP socket\n");
     }

     // We now connect the MICTCP socket
     if(mic_tcp_connect(mic_tcp_sockfd, mic_tcp_dest_addr) == -1) {
          printf("ERROR connecting the MICTCP socket\n");
     }
     
     if (bind(listen_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
          printf("ERROR on binding the UDP socket: ");
          perror(0);
          printf("\n");
     }

     listen(listen_sockfd,5);


     // Initialize a packet count variable used when loss emulation is active
     int count = 0;
     ssize_t n = -1;

     // Main activity loop, we never exit this, user terminates with SIGKILL
     while(1) {
          bzero(buffer, MAX_UDP_SEGMENT_SIZE);

          n = recvfrom(listen_sockfd, buffer, MAX_UDP_SEGMENT_SIZE, 0, (struct sockaddr *) &cliaddr, &len);
          if (n < 0) {
               perror(0); 
          }

          // We forward the packet to its final destination
          n = mic_tcp_send(mic_tcp_sockfd, buffer, n); 
          if (n <= 0) {
               printf("ERROR on MICTCP send\n");
          } 
     }

     // We never execute this but anyway, for sanity
     close(listen_sockfd);
      
     // Same for MICTCP
     mic_tcp_close(mic_tcp_sockfd);
} 

/**
 * Function that listens on MICTCP and delivers to UDP.
 * returns void
 */
void mictcp_to_udp(struct sockaddr_in listen_on, struct sockaddr_in transmit_to) {

     // Define the socket on which we listen
     int sending_sockfd;

     // A buffer used to store received segments before they are forwarded
     char buffer[MAX_UDP_SEGMENT_SIZE];
     int rcv_values [CUMULATED_BUFF_NB];    
     char* Buff [CUMULATED_BUFF_NB];
     int iii;
     for (iii=0; iii<CUMULATED_BUFF_NB; iii++)
     {
         Buff[iii] = malloc(MAX_UDP_SEGMENT_SIZE);
     }

     // Addresses for the work to be performed
     struct sockaddr_in serv_addr = listen_on;
     struct sockaddr_in cliaddr;
     socklen_t len = sizeof(cliaddr);
     
     // MICTCP stuff
     int mic_tcp_sockfd;
     start_mode start_mode_mic_tcp = SERVER;
     mic_tcp_sock_addr mic_tcp_listen_addr, mic_tcp_remote_addr;
     mic_tcp_listen_addr.ip_addr = (char* ) &(listen_on.sin_addr.s_addr);
     mic_tcp_listen_addr.port = listen_on.sin_port;

     // We construct the UDP and MICTCP sockets to be used by this function
     sending_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     
     if((mic_tcp_sockfd = mic_tcp_socket(start_mode_mic_tcp)) == -1) {
          printf("ERROR creating the MICTCP socket\n");
     }

     if (mic_tcp_bind(mic_tcp_sockfd, mic_tcp_listen_addr) == -1) {
          printf("ERROR on binding the MICTCP socket\n");
     }

     if(mic_tcp_accept(mic_tcp_sockfd, &mic_tcp_remote_addr) == -1) {
          printf("ERROR on accept on the MICTCP socket\n");
     }

     // Initialize a packet count variable used when loss emulation is active
     int count = 0;
     ssize_t n = -1;

     // Main activity loop, we never exit this, user terminates with SIGKILL
     while(1) {
         int k;
         for(k=0; k<CUMULATED_BUFF_NB; k++) 
         {
            bzero(Buff[k], MAX_UDP_SEGMENT_SIZE);
         }
         
         for(k=0; k<CUMULATED_BUFF_NB; k++) 
         {
              n = mic_tcp_recv(mic_tcp_sockfd, Buff[k], MAX_UDP_SEGMENT_SIZE);
              if (n <= 0) {
                   printf("ERROR on mic_recv on the MICTCP socket\n");
              }
         }

         // We forward the packet to its final destination
         for(k=0; k<CUMULATED_BUFF_NB; k++) 
         {
              n = sendto(sending_sockfd, Buff[k], n, 0, (struct sockaddr *) &transmit_to, len);
              if (n <= 0) {
                   perror(0);
              } 
         }
         
         //sleep(1);
     }

     // We never execute this but anyway, for sanity
     close(sending_sockfd);
      
     // Same for MICTCP
     mic_tcp_close(mic_tcp_sockfd);
} 


static void usage(void) {
        printf("usage: gateway [-p|-s][-t tcp|mictcp] (<server>) <port>\n");
}


int main(int argc, char ** argv) {

     // Should losses be emulated?
     int loss = 1;
     int transport = 0;
     int puits = -1;

     // What sockaddr should this program listen on?
     // Always on port 1234 (data from VLC arrives there using UDP)
     struct sockaddr_in serv_addr;
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(1234);

     // Where should this program send the received data?
     struct sockaddr_in dest_addr;
     bzero((char *) &dest_addr, sizeof(dest_addr));
     dest_addr.sin_family = AF_INET;
     struct hostent * hp = gethostbyname("127.0.0.1");
     bcopy ( hp->h_addr, &(dest_addr.sin_addr.s_addr), hp->h_length);
     dest_addr.sin_port = htons(1234);

     extern int optind;
     int ch;

     while ((ch = getopt(argc, argv, "t:sp")) != -1) {
          switch (ch) {
               case 't':
                    if(strcmp(optarg, "mictcp") == 0) {
                         transport = 1;
                    } else if(strcmp(optarg, "tcp") == 0) {
                         transport = 0;
                    } else {
                         printf("Unrecognized transport : %s\n", optarg);
                    } 
                    break;
                case 's':
                    if(puits == -1) {
                         puits = 0; 
                    } else {
                         puits = -2;
                    }
                    break;
                case 'p':
                    if(puits == -1) {
                         puits = 1; 
                    } else {
                         puits = -2;
                    }
                    break;
               default:
                    usage();
          }
     }
     if(puits < 0) {
          usage();
          return -1;
     }
     argc -= optind;
     argv += optind;
    
     if((puits == 1 && argc != 1) || ((puits == 0) && argc != 2)) {
          usage();
          return -1;
     }

     if(puits == 0) {
          hp = gethostbyname(argv[0]);
          bcopy ( hp->h_addr, &(dest_addr.sin_addr.s_addr), hp->h_length);
          dest_addr.sin_port = htons(atoi(argv[1]));
     } else {
          serv_addr.sin_port = htons(atoi(argv[0]));
     }

     if(transport == 0) {
          if(puits == 0) {
               // We receive on UDP and emulate TCP behavior before sending the data out
               file_to_tcp(serv_addr, dest_addr, loss);
          } else {
               printf("No gateway needed for puits using UDP\n");
          }
     } else if(transport == 1) {
          if(puits == 0) {
               // We receive on UDP and send the data using MICTCP
               file_to_mictcp(serv_addr, dest_addr);
          } else {
               // We receive on MICTCP and send the data using UDP
               mictcp_to_udp(serv_addr, dest_addr);
          }

     }
}
