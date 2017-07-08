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

#define TIMEOUT_SEC 1
#define TIMEOUT_uSEC 0
#define FALSE 0
#define TRUE 1

#define BUF_SIZE 2048

fd_set master_fds;
int main () 
{
/*declare the variable and initialize*/
int z,len_addr,child_pid,client_sockfd_1,client_sockfd_2;
int server_sockfd[2];
int count =0;
char server_buffer_rcv [BUF_SIZE];
char server_buffer_send[BUF_SIZE];
struct sockaddr_in6 srver_addr , client_addr;
FILE* fp;
char *fname=(char*) malloc(50*sizeof (char));
char *fname_temp=(char *)malloc(50*sizeof (char));
char *fcontent=NULL;
fcontent = (char *) malloc(1024 * sizeof (char));

struct addrinfo hints;
struct addrinfo *result, *rp;
char *portnum = "9090";

int fd_max,client_fd,stop_req;
stop_req = FALSE;
fd_set read_fds;
struct timeval timeout;
char str;

// initialize fd set
FD_ZERO(&master_fds);
FD_ZERO(&read_fds);
len_addr = sizeof (client_addr);

/*initialize the hints & get address*/
memset(&hints, 0, sizeof(struct addrinfo));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_NUMERICSERV;
hints.ai_protocol = 0;
hints.ai_canonname = NULL;
hints.ai_addr = NULL;
hints.ai_next = NULL;

z = getaddrinfo(NULL, portnum, &hints, &result);
 if(z!=0) printf("error calling addrinfo\n");

for (rp = result; rp!=NULL; rp = rp->ai_next) {
/*create socket*/
   server_sockfd[count] = socket (rp->ai_family,rp->ai_socktype,rp->ai_protocol);
   if (server_sockfd[count] == -1) printf("error while creating the socket\n");
/*bind socket*/
  z = bind(server_sockfd[count],rp->ai_addr,rp->ai_addrlen);
  if(z == -1) printf("error while binding the socket\n");


/*listen socket*/http://127.0.0.1:9090/file1.txt
  z = listen(server_sockfd[count],5);
  if(z == -1) printf("error while listening socket\n");
count++;

}
/* initialize the master_fds */
FD_SET (0, &master_fds);
FD_SET (server_sockfd[0], &master_fds);
FD_SET (server_sockfd[1], &master_fds);

fd_max = (server_sockfd[0] > server_sockfd[1]) ? server_sockfd[0] : server_sockfd[1];

printf("sock_fd0 = %d\n",server_sockfd[0]);
printf("sock_fd1 = %d\n",server_sockfd[1]); 
printf("fd_max is %d\n", fd_max);
/* start the server loop */
for (;;) {
int i;
/* initialize the read_fd */
FD_ZERO (&read_fds);
read_fds = master_fds;

/*initialize timeout timer */
timeout.tv_sec = TIMEOUT_SEC;
timeout.tv_usec = TIMEOUT_uSEC;

/* call the select function */
z = select (fd_max+1, &read_fds, NULL, NULL, &timeout);  // select to check the request
printf ("number of id's %d\n",z);
if (z == -1) printf("error while select function\n");
 if (!z) printf("no requests in the specified time\n");
  else
    {
      if (FD_ISSET (0, &read_fds))  // check for the keyboad input and process the request
          {
            str = getchar ();
            if ( 'q' == str)        //quit server
                goto quit;
            else if ('s' == str)
                 stop_req = TRUE;  /* stop accepting new connection - set variable*/
          }
       for ( i =1; i<=fd_max ; i++)
          {
             printf("now testing %d\n", i);
            if (FD_ISSET ( i, &read_fds))
             {
                if (  (i == server_sockfd[0])||(i == server_sockfd[1]))
                  {   
                    if (!stop_req)
                    {
                       printf("he is set %d\n",i);
                      client_fd = accept (i, (struct sockaddr *)&client_addr, &len_addr); // accept new connection 
                       if (client_fd == -1)
                          { 
                             printf ( "error while accepting the connection \n");
                          }
                       else 
                          {
                            FD_SET (client_fd, &master_fds);                       // response to the connection 
                            if (client_fd > fd_max) fd_max = client_fd;
                           response (client_fd, fp, fname, fname_temp, fcontent, server_buffer_rcv, server_buffer_send);
                           printf("got the msggggggggggggggggggggg right\n");
                          }             
                     } 
                     else { 
                           FD_CLR(server_sockfd[0],&master_fds);
                           FD_CLR(server_sockfd[1],&master_fds);
                           printf ("server can't accept new connection\n"); // if s is pressed server can't accept new connection  
                           }
                  }                   
                else 
                     {  
                     printf("he is not set %d\n",i);                  
                    response (client_fd, fp, fname, fname_temp, fcontent, server_buffer_rcv, server_buffer_send);
                     printf("is this unnecessary\n");
                     }
             }
          }
    }
} // end of for loop

quit: printf(" This server is quitted - tchuuuuuuusssssss\n");
return 0;
}



void response(int client_sockfd, FILE* fp,char *fname,char *fname_temp,char *fcontent,char *server_buffer_rcv,char * server_buffer_send)
{


int z;
int x = 2048;
/*read the request */
z = read (client_sockfd,server_buffer_rcv, x);
    if (z == -1) printf("error while reading from client\n");
    if (z==0) 
         {
           printf (" connection is closed \n");
           FD_CLR (client_sockfd, &master_fds);
           close(client_sockfd);
           return;
         }          
printf("\n the received message from browser is %s", server_buffer_rcv);
/* process the request */
fname_temp = strtok(server_buffer_rcv," /");
fname_temp = strtok(NULL, " ");
if (*fname_temp == '/')
strcpy (fname, fname_temp+1);
else strcpy (fname,fname_temp);
printf("requested file is %s\n",fname);
if (strcmp(fname, "fevicon.ico")==0)
strcpy(fname,"file1.txt");

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


