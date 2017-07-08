/* metno: 350880 */
#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h> //hton
#include <arpa/inet.h>  //inet_aton
#include <string.h> // memset
#include <unistd.h> //fcntl
#include <fcntl.h> //fcntl
#include <signal.h> //signal

void io_handler(int);
int srvr_fd;

int main ( ) 
{
 struct sockaddr_in addr_srvr, addr_client;
 char srvr_udp_buffer[512];
 int srvr_bind, len_addr;


/* creating server socket */
 srvr_fd = socket (AF_INET, SOCK_DGRAM, 0);
 if (srvr_fd == -1)
 printf ("error while creating server fd\n");

/* initialize the socket address */
memset (&addr_srvr, 0, sizeof addr_srvr);
addr_srvr.sin_family = AF_INET;
addr_srvr.sin_port = htons(4048);
addr_srvr.sin_addr.s_addr = inet_addr("127.0.0.1");
len_addr = sizeof addr_srvr;

/*bind the address to the socket of server */
srvr_bind = bind (srvr_fd, (struct sockaddr *) &addr_srvr, len_addr);
if( srvr_bind == -1)
  printf("error while socket binding");

/* set up singal i/o handler */
signal(SIGIO,io_handler);

/*set the process id's */
if (fcntl(srvr_fd,F_SETOWN,getpid())<0) 
    printf("error while setting process id\n");

/* set the signal as asynchronous */
if (fcntl(srvr_fd,F_SETFL,FASYNC) < 0 )
   printf("error while setting the client socket to asynchronous\n");

for (;;)
;

}

/* signal handling function */
void io_handler (int signal)
{
int len_addr,nBytes;
struct sockaddr_in addr_client;
char srvr_udp_buffer[512];
  len_addr = sizeof addr_client;
  /* receive the message from client and display*/
     nBytes = recvfrom (srvr_fd,srvr_udp_buffer, sizeof srvr_udp_buffer,0,(struct sockaddr *) &addr_client, &len_addr );
     if (nBytes < 0) 
        printf("error while receiving request from client\n");
     srvr_udp_buffer[nBytes] = 0;

/*print the received message */
printf ("message from client %s port %u: \n\t'%s'\n",
             inet_ntoa(addr_client.sin_addr),
             (unsigned)ntohs(addr_client.sin_port),
             srvr_udp_buffer);

/*echo the message */
     nBytes = sendto (srvr_fd,srvr_udp_buffer, sizeof srvr_udp_buffer,0,(struct sockaddr *) &addr_client, len_addr );
     if (nBytes < 0 )
        printf ("error while echoing\n");


return;
}









  
       
 




