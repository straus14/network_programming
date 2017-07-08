/*#include <stdio.h>      
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
#include <linux/if_packet.h> // structrure sockaddr_ll
#include <sys/ioctl.h> //ioctl local call
#include <net/if.h> // header for ifnamsiz */

#define DEFAULT_IF "wlan0"
#define DST_MAC0 0xFF
#define DST_MAC1 0xFF
#define DST_MAC2 0xFF
#define DST_MAC3 0xFF
#define DST_MAC4 0xFF
#define DST_MAC5 0xFF

#define BUFFER_SIZE_ARP 1500

#define TIMEOUT_ARP 20
// datagram socket, manual array for ip, ip interface by name

void my_arp(char *ip, struct sockaddr_ll *mac_ptr)

{

printf("the ip address is:%s \n",ip);
// define variable
struct sockaddr_in source_ip, dest_ip;
struct sockaddr_ll source_mac, dstaddr;
struct ifreq ifr;
char ifName[IFNAMSIZ];
int packet_socket,z,pck_size,len_addr;
char *rcv_buffer = (char *) malloc (BUFFER_SIZE_ARP);;
struct sockaddr rcv_addr;
struct ether_arp *rep = (struct ether_arp *) rcv_buffer;
//source_ip.sin_addr.s_addr = inet_addr("192.168.1.8");
dest_ip.sin_addr.s_addr = inet_addr(ip);
fd_set read_fds;
struct timeval timeout;
struct sockaddr_in verify_ip;

// define arp structure
struct ether_arp req;
req.arp_hrd = htons(ARPHRD_ETHER);   /* Format of hardware address.  ARPHRD_IEEE802-------------chekc!!!*/
req.arp_pro = htons(ETH_P_IP);      /* Format of protocol address.  */
req.arp_hln = ETHER_ADDR_LEN;         /* Length of hardware address.  */
req.arp_pln = sizeof (in_addr_t);    /* Length of protocol address.  */
req.arp_op = htons(ARPOP_REQUEST);   /* ARP opcode (command).  */
//memcpy (req.arp_sha,
//memcpy (&req.arp_spa,&source_ip.sin_addr.s_addr,sizeof(req.arp_spa));
memset (&req.arp_tha,0,sizeof (req.arp_tha));
memcpy (&req.arp_tpa,&dest_ip.sin_addr.s_addr, sizeof(req.arp_tpa));


//create the packet
 packet_socket = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
 if (packet_socket == -1) 
   { 
     perror("socket()");
     exit(-1);
   }

// fill in for sockaddr_ll
strcpy (ifName, DEFAULT_IF);
memset(&ifr, 0, sizeof(struct ifreq));
strncpy(ifr.ifr_name, ifName, IFNAMSIZ-1);
if (ioctl(packet_socket, SIOCGIFINDEX, &ifr) < 0)
    perror("ioctl");


dstaddr.sll_ifindex = ifr.ifr_ifindex;
dstaddr.sll_halen = ETH_ALEN;
dstaddr.sll_family = AF_PACKET;
dstaddr.sll_protocol = htons(ETH_P_ARP);
dstaddr.sll_addr[0] = DST_MAC0;
dstaddr.sll_addr[1] = DST_MAC1;
dstaddr.sll_addr[2] = DST_MAC2;
dstaddr.sll_addr[3] = DST_MAC3;
dstaddr.sll_addr[4] = DST_MAC4;
dstaddr.sll_addr[5] = DST_MAC5;

len_addr = sizeof (rcv_addr);
//get the mac address
if(ioctl(packet_socket, SIOCGIFHWADDR, &ifr) < 0)
   perror ("ioctl mac");

//if(ifr.ifr_hwaddr.sa_family!= ARPHRD_ETHER)
 // printf("something wrong\n");

memcpy(&req.arp_sha,&ifr.ifr_hwaddr.sa_data,sizeof(req.arp_sha));

//get the ip address 
if(ioctl(packet_socket,SIOCGIFADDR,&ifr) < 0)
   perror("ioctl ip");

struct sockaddr_in* source_ip_addr = (struct sockaddr_in*)&ifr.ifr_hwaddr.sa_data;
memcpy(&req.arp_spa,&source_ip_addr->sin_addr.s_addr,sizeof(req.arp_spa));


do
{
// send the arp request 
z = sendto(packet_socket, &req ,sizeof(req),0,(struct sockaddr*)&dstaddr,sizeof(struct sockaddr_ll)); 
     if ( z < 0) perror ("sendto");

printf("arp request sent\n");
//receive arp request
FD_ZERO (&read_fds);
FD_SET (packet_socket, &read_fds);
timeout.tv_sec = TIMEOUT_ARP;
timeout.tv_usec = 0;
z = select (packet_socket+1,&read_fds, NULL, NULL, &timeout);
 if (z == -1) {
         perror("select()-arp function");
         continue;
       }
       else if (!z) printf("check other hosts connected in the network no not?\n"); /* timeout */
       else if ( FD_ISSET(packet_socket, &read_fds))
              {
 pck_size = recvfrom (packet_socket, rcv_buffer, BUFFER_SIZE_ARP, 0, &rcv_addr, (socklen_t *)&len_addr);
                   if (pck_size == -1) {
                          perror("recvfrom()");
                         }
              }

memcpy(&verify_ip.sin_addr.s_addr,&rep->arp_spa,sizeof(verify_ip.sin_addr.s_addr));
}while(!(verify_ip.sin_addr.s_addr==(dest_ip.sin_addr.s_addr)));
// print the mac
if (pck_size >0)
 { 
  printf(" the mac address is: %02X:%02X:%02X:%02X:%02X:%02X\n", rep->arp_sha[0], rep->arp_sha[1], rep->arp_sha[2], rep->arp_sha[3], rep->arp_sha[4], rep->arp_sha[5]);

memcpy(mac_ptr->sll_addr,rep->arp_sha,sizeof(req.arp_tha));
}          
            
return;

}

