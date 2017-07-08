#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h> //hton
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TIMEOUT_SEC 5
#define TIMEOUT_uSEC 0
#define PAYLOAD_SIZE 64
#define BUFF_SIZE 1500
unsigned short icmp_chksum(unsigned short * addr, int len);

int main (int argc, char **argv)
{

if (argc <2) {
  printf("given command length %d\n", argc);
  printf("give command in this format SUDO ./PROG IP-ADDRESS\n");
  exit (0);
 }
 
/*declare the variable */
int raw_socket,z,pck_size,len_addr;
struct sockaddr_in saddr;
char *packet = NULL;
struct icmphdr *icmp = NULL;
struct iphdr *ip = NULL;
//char *daddr = argv[2];
struct sockaddr_in to_addr,rcv_addr;
fd_set read_fds;
struct timeval timeout;
char recv_buffer[BUFF_SIZE];
struct in_addr daddr;

int seq, nid;
seq = 1; nid = 1;

/*initialize the variable*/
saddr.sin_addr.s_addr = htonl(INADDR_ANY);
saddr.sin_family = AF_INET;
saddr.sin_port = htons (9090);
pck_size = sizeof (struct icmphdr) + PAYLOAD_SIZE;

packet = (char *) malloc (pck_size);
icmp = (struct icmphdr *)packet;

to_addr.sin_family = AF_INET;
to_addr.sin_addr.s_addr = inet_addr(argv[1]); //set the address to ping

/*create raw socket & bind the socket */
raw_socket = socket (AF_INET,SOCK_RAW,IPPROTO_ICMP);
if (raw_socket == -1) printf("error while creating the sokcet\n");

z = bind (raw_socket, (const struct sockaddr *)&saddr,sizeof(saddr));
if ( z < -1) printf ("error while binding the socket\n");

/*set the echo request packet */
icmp->type = ICMP_ECHO;
icmp->code = 0;
icmp->un.echo.sequence = seq;
icmp->un.echo.id = nid;
icmp->checksum = 0;
memset(packet + sizeof (struct icmphdr), rand(), PAYLOAD_SIZE);
icmp->checksum = icmp_chksum((unsigned short *)packet, pck_size);



FD_ZERO (&read_fds);
FD_SET (raw_socket, &read_fds);
timeout.tv_sec = TIMEOUT_SEC;
timeout.tv_usec = TIMEOUT_uSEC;
len_addr = sizeof(rcv_addr);

/*send the request */
z = sendto (raw_socket, packet, pck_size, 0, (struct sockaddr *)&to_addr, sizeof(to_addr));
if (z == -1) printf("error while sending\n");

/* listen the socket using select with timeout */

z = select (raw_socket+1, &read_fds, NULL, NULL, &timeout);
if (z == -1) printf("error while select function\n");
if (!z) printf("timeout of 5 seconds for echo request \n");
else
   {
     if (FD_ISSET (raw_socket, &read_fds)) 
       {
         printf(" packet received to raw socket!!!! - checking is it icmp?!.....\n");
           z= recvfrom(raw_socket, recv_buffer, BUFF_SIZE, 0, (struct sockaddr *) &rcv_addr, &len_addr);
           if (z < 0) printf("error while receiving request from client\n");
            else
                 {
                   icmp = (struct icmphdr *) (recv_buffer + sizeof (struct iphdr));
                   ip = (struct iphdr *) recv_buffer;
                   if (icmp->type == ICMP_ECHOREPLY) {
                    daddr.s_addr = ip->saddr;
                    printf("Yes, echo reply received with %d bytes from destination:%s\n",z,inet_ntoa(daddr));
                    } else 
                         printf("No, received packet is not icmp\n");   
                 }        
       }
    }    
close (raw_socket);
return 0;
}

unsigned short icmp_chksum(unsigned short * addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}


