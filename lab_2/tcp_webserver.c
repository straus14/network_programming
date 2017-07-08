/* Metno: 350880 ---- server_tcp_webserver*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main () 
{
/*declare the variable and initialize*/
int z, server_sockfd,client_sockfd,len_add,len_addr,child_pid;
char server_buffer_rcv [1024];
char server_buffer_send[2048];
struct sockaddr_in server_addr , client_addr;
FILE* fp;
char *fname=(char*) malloc(50*sizeof (char));
char *fname_temp=(char *)malloc(50*sizeof (char));
char *fcontent=NULL;
fcontent = (char *) malloc(1024 * sizeof (char));
/*create the socket of server*/
 server_sockfd = socket(AF_INET,SOCK_STREAM,0);
 if (server_sockfd == -1) printf("error while creating the socket\n");

/*initialize the socket address*/
 len_addr = sizeof server_addr;
 memset(&server_addr,0,len_addr);
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(9090);
 server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

/*bind the socket of server*/
 z = bind (server_sockfd, (struct sockaddr *) &server_addr, len_addr);
 if (z == -1) printf("error while binding to server\n");

/*listen */
 z = listen (server_sockfd,5);
 if (z == -1) printf("error while listening\n");

/* start the server loop */


for (;;) {
/*accept*/
  len_addr = sizeof client_addr;
  client_sockfd = accept (server_sockfd, (struct sockaddr *) &client_addr, &len_addr);
  if (client_sockfd == -1) printf("error while accepting client socket\n");

child_pid = fork();
if (child_pid < 0 ) printf("error while creating child process\n");
 else if (child_pid == 0) {
     
    close (server_sockfd); //close the server socket
    z = read (client_sockfd,server_buffer_rcv,sizeof server_buffer_rcv);
    if (z == -1) printf("error while reading from client\n");
    printf("\n the received message from browser is %s", server_buffer_rcv);

/* process the request */
fname_temp = strtok(server_buffer_rcv," /");
fname_temp = strtok(NULL, " ");
if (*fname_temp == '/')
strcpy (fname, fname_temp+1);
else strcpy (fname,fname_temp);
printf("requested file is %s\n",fname);

/*copy the file content*/
fp = fopen (fname,"r");
while (!feof(fp)){
fgets(fcontent,1024,fp);
}
fclose(fp);

printf("content is %s\n",fcontent);
/*generate the response body */
sprintf(server_buffer_send,"HTTP/1.1 200 OK\nContent-Length:%d\nContent-Type: text/html\n\n%s",strlen(fcontent),fcontent);
printf("content is %s\n",server_buffer_send);

/*response to the client */
z = write (client_sockfd,server_buffer_send,2048);
if(z == -1) printf("error while sending to client\n");
exit(0);
}
close (client_sockfd);

  }
return 0;
}
