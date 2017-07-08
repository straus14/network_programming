/* Metno: 350880 ---- server_tcp_webserver*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

void response(int client_sockfd, FILE* fp,char *fname,char *fname_temp,char *fcontent,char *server_buffer_rcv,char * server_buffer_send);

#define BUF_SIZE 2048
int main () 
{
/*declare the variable and initialize*/
int z,len_addr,child_pid,client_sockfd_1,client_sockfd_2;
int server_sockfd[2];
int count =0;
char server_buffer_rcv [BUF_SIZE];
char server_buffer_send[BUF_SIZE];
struct sockaddr client_addr;
FILE* fp;
char *fname=(char*) malloc(50*sizeof (char));
char *fname_temp=(char *)malloc(50*sizeof (char));
char *fcontent=NULL;
fcontent = (char *) malloc(1024 * sizeof (char));

struct addrinfo hints;
struct addrinfo *result, *rp;
const struct sockaddr *sa = NULL;
char *portnum = "9090";
char host [BUF_SIZE]; 
char service [BUF_SIZE];
len_addr = sizeof client_addr;
/*initialize the hints & get address*/
memset(&hints, 0, sizeof(struct addrinfo));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_NUMERICSERV;
hints.ai_protocol = 0;
hints.ai_canonname = NULL;
hints.ai_addr = NULL;
hints.ai_next = NULL;

/*get the address required */
z = getaddrinfo(NULL, portnum, &hints, &result);
 if(z!=0) printf("error calling addrinfo\n");


/*print host name*/
z = gethostname (host, sizeof host);
if (z != 0) printf("error while getting hostname\n");
else printf("hostname-computer : %s\n",host);


/*for loop to create socket with getaddrinfo returned address */
for (rp = result; rp!=NULL; rp = rp->ai_next) {
/*create socket */
   server_sockfd[count] = socket (rp->ai_family,rp->ai_socktype,rp->ai_protocol);
   if (server_sockfd[count] == -1) printf("error while creating the socket\n");
   


/*bind socket*/
  z = bind(server_sockfd[count],rp->ai_addr,rp->ai_addrlen);
  if(z == -1) printf("error while binding the socket\n");

/*listen socket*/
  z = listen(server_sockfd[count],5);
  if(z == -1) printf("error while listening socket\n");
count++;

}

/* start the server loop */

for (;;) {

/*accept*/
//processing for the first socket
client_sockfd_1 = accept (server_sockfd[0],(struct sockaddr *) &client_addr, &len_addr);
  if (client_sockfd_1 == -1) printf("error while accepting client socket\n"); 
/*print client host and service */
 sa = &client_addr;
 getnameinfo(sa,len_addr,host, sizeof (host), service, sizeof (service),NI_NAMEREQD);
 printf("hostname-client : %s\n",host);
 printf("service name : %s\n", service);
 sa = NULL;
/*process client request with fork*/
   child_pid = fork();
   if (child_pid < 0 ) printf("error while creating child process\n");
       else if (child_pid == 0) {
         close (server_sockfd[0]);
         response( client_sockfd_1, fp, fname, fname_temp, fcontent, server_buffer_rcv, server_buffer_send);
         exit(0);
        }
    close(client_sockfd_1);

//processing for the second socket
client_sockfd_2 = accept (server_sockfd[1],(struct sockaddr *) &client_addr, &len_addr);
  if (client_sockfd_2 == -1) printf("error while accepting client socket\n"); 
/*print client host and service */
 sa = &client_addr;
 getnameinfo(sa,len_addr,host, sizeof (host), service, sizeof (service),NI_NAMEREQD);
 printf("hostname-client : %s\n",host);
 printf("service name : %s\n", service);
 sa = NULL;
/*process client request with fork*/
   child_pid = fork();
   if (child_pid < 0 ) printf("error while creating child process\n");
       else if (child_pid == 0) {
         close (server_sockfd[1]);
         response( client_sockfd_2, fp, fname, fname_temp, fcontent, server_buffer_rcv, server_buffer_send);
         exit(0);
        }
    close(client_sockfd_2);

  }

return 0;
}



void response(int client_sockfd, FILE* fp,char *fname,char *fname_temp,char *fcontent,char *server_buffer_rcv,char * server_buffer_send)
{


int z;
int x = 2048;
/*read the request */
z = read (client_sockfd,server_buffer_rcv, x);
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
fgets(fcontent,x,fp);
}
fclose(fp);

printf("content is %s\n",fcontent);

/*generate the response body */
sprintf(server_buffer_send,"HTTP/1.1 200 OK\nContent-Length:%d\nContent-Type: text/html\n\n%s",strlen(fcontent),fcontent);
printf("content is %s\n",server_buffer_send);

/*response to the client */
z = write (client_sockfd,server_buffer_send,x);
if(z == -1) printf("error while sending to client\n");

return;

}


