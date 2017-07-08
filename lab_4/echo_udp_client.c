/* metno: 350880 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main () {

struct sockaddr_in addr_srvr, verify_addr_srvr;
char client_udp_buffer[512];
int client_fd,len_addr,z,x;

/*create client socket */
client_fd = socket (AF_INET,SOCK_DGRAM,0);
if (client_fd == -1)
printf (" error while creating client socket\n");

/*initialize the address of the server */
memset(&addr_srvr, 0, sizeof addr_srvr);
addr_srvr.sin_family = AF_INET;
addr_srvr.sin_port = htons(4048);
addr_srvr.sin_addr.s_addr = inet_addr("127.0.0.1");
len_addr = sizeof addr_srvr;

/* send and receive the message with server */
for(;;) {

/* get the message to send and format the message*/
fputs("\n Enter the message to server: ", stdout);
fgets (client_udp_buffer, sizeof client_udp_buffer, stdin);
z = strlen (client_udp_buffer);
if ( z>0 && client_udp_buffer [-z] == '\n')
    client_udp_buffer[z] = 0;

/*send the message to server */
z = sendto (client_fd, client_udp_buffer, strlen(client_udp_buffer),0, (struct sockaddr *) &addr_srvr,len_addr);
if (z<0) printf("error while sending the message\n");

/*Test if message should be terminated */
if (!strncasecmp(client_udp_buffer, "STOP",4))
    break;

/*receive the echo message */
x = sizeof verify_addr_srvr;
z = recvfrom (client_fd,client_udp_buffer, sizeof client_udp_buffer, 0, (struct sockaddr *)&verify_addr_srvr, &x);
if (z<0) printf("error while receiving the message\n");
client_udp_buffer[z] = 0;

/*display the echo message */
printf ("Echo message from the server with address %s port %u: \n\t'%s'\n",
         inet_ntoa(verify_addr_srvr.sin_addr),
         (unsigned)ntohs(verify_addr_srvr.sin_port),
         client_udp_buffer);

   }

/*close the socket */
close (client_fd);
printf("closed connection\n");
return 0;

}






