/* assignment 5 - exercise 2 - packet socket sniffing */
#include <stdio.h>      
#include <string.h>
#include <stdlib.h>  // malloc
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h> // provides ip header
#include <netinet/if_ether.h> // for eth_p_all
#include <net/ethernet.h> // for ethernet header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>


#define BUFFER_SIZE 65536
#define TIMEOUT_SEC 1
#define TIMEOUT_uSEC 0

int main ()
{
 unsigned char *buffer = (unsigned char *) malloc (BUFFER_SIZE);
 int packet_socket, z;
 int pck_size, len_addr;
 struct sockaddr rcv_addr;
 fd_set read_fds;
 struct ethhdr *eth =NULL;
 struct iphdr *ip =NULL;
 struct in_addr daddr;
 struct timeval timeout;
 struct protoent *proto;
 

 /*create packet socket */
 packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
 if (packet_socket == -1) printf("error while creating socket\n");

while (1)
 {
  FD_ZERO (&read_fds);
  FD_SET (packet_socket, &read_fds);
  timeout.tv_sec = TIMEOUT_SEC;
  timeout.tv_usec = TIMEOUT_uSEC;
  len_addr = sizeof (rcv_addr);
  z = select (packet_socket+1,&read_fds, NULL, NULL, &timeout);
  if (z == -1) {
         printf("error while select function\n");
       }
     else if (!z) printf("timeout this time !\n");
     else if ( FD_ISSET(packet_socket, &read_fds))
                    {
                     
                      /*receive the packet in buffer */
                      pck_size = recvfrom (packet_socket, buffer, BUFFER_SIZE, 0, &rcv_addr, (socklen_t *)&len_addr);
                      if (pck_size == -1) {
                          printf("error while receiving frame\n");
                          //exit(0);
                         }
                      printf("Information of the received packet here\n");
                      printf("Number of bytes : %d\n",pck_size);
                      eth = (struct ethhdr *) buffer;
                      proto = getprotobynumber (eth->h_proto);
                      if (!(proto == NULL)) {                     
                            printf("network layer protocol : %d - %s\n",eth->h_proto, proto->p_name);
                            ip = (struct iphdr *) (buffer + sizeof (struct ethhdr));
                         }
                      proto = getprotobynumber (ip->protocol);
                      if (!(proto == NULL)) 
                             printf("transport layer protocol: %d - %s\n", ip->protocol, proto->p_name);
                      daddr.s_addr = ip->saddr;
                      printf("source address: %s\n",inet_ntoa(daddr));

                  }
              else  printf("fooled--- fd not set\n"); 
 }

close (packet_socket);
printf("Tschusssss\n");
return 0;
}

















