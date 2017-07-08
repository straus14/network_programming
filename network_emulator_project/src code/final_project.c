/* Final project for network programming laboratory, INETS, RWTH Aachen University
 * Sudarshan Sadananda Matr. nr. 350880
 * Theerawat Kiatdarakun Matr. nr. 340962
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  //malloc
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h> //provides ip header
#include <netinet/if_ether.h> //for eth_p_all
#include <net/ethernet.h> //for ethernet header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/if_packet.h> //structrure sockaddr_ll
#include <sys/ioctl.h> //ioctl local call
#include <net/if.h> // header for ifnamsiz
#include <sys/select.h> // to select function
#include <sys/time.h>
#include <time.h>
#include "my_arp.h" //contains function to get MAC address automatically
#include <fcntl.h>
#include <termios.h>

/* macros */
#define BUFFER_SIZE 65535 // buffer size set to theoretical maximum
#define SEND_BUFF_SIZE 7981
#define TIMEOUT_SEC 0
#define TIMEOUT_uSEC 10

#define DEFAULT_IF "wlan0"

typedef int bool;
#define TRUE 1
#define FALSE 0

/*---------------------------usage of this code for emulation ------------------------------------------------


format to run this code : "<./filename> <emulation command> <statistic> <ip1> <ip2>"

<emulation command>
delay - d
loss - l
error - e
out of order - o
normal (simple forwarder) - n

<statistic>
loss and error should be given input of percentage number.
delay should be given in microseconds
out-of-order should be number of frame to get out of order

Example to emulate packet error with 14% with two random local host , command is,

  sudo ./executable l 14 192.168.1.10 192.168.1.58


-------------------------------------------------------------------------------------------------------------*/
struct termios stdin_orig;  // Structure to save parameters

double time_diff(struct timeval x , struct timeval y);
void term_reset();
void term_nonblocking();

int main (int argc, char **argv)
{
	term_nonblocking(); //allow a pressed key to be read without the need of carriage return

	//record the time at the beginning
	time_t begin;
	struct tm * time_begin;
	time (&begin);
	time_begin = localtime ( &begin );
	struct timeval program_begin, program_finished;
	gettimeofday(&program_begin , NULL);

	//produce a summary file
	FILE *fptr;
	fptr = fopen("result.txt", "rb+");
	if(fptr == NULL) //if file does not exist, create it
	{
		fptr = fopen("result.txt", "wb");
	}

	fprintf(fptr, "N th packet						source			destination\n\n");

	if (argc < 5)
	{
		printf("usage <./filename> <emulation command> <statistic> <ip1> <ip2>\n");
		printf("emulation command: l - loss, d- delay, o - out of order, e - error\n");
		printf("statistic: microseconds (d), percentage number (l & e), periodic number of frames before out-of-order (o)\n");
		printf("(with emmulation command) example: ./file e 14 <ip1> <ip2>\n");
		printf("(without emulation command) example: ./file n 100 <ip1> <ip2>\n");
		printf("Press 's' and enter to exit emulator");
		exit(0);
	}

	/* declare variables used */
	char *rcv_buffer = (char *) malloc (BUFFER_SIZE);
	char *rcv_buffer_temp = (char *) malloc (BUFFER_SIZE);
	int packet_socket, pck_size, z;
	socklen_t len_addr;
	struct sockaddr_in rcv_addr;
	struct timeval timeout;
	fd_set read_fds;
	struct ifreq if_idx;
	char ifName[IFNAMSIZ];
	int len_ll;
	pck_size = 0;
	unsigned long int rand_thrsld;
	int buttonPressed;

	int sendPkt = FALSE;
	int sendOutofOrderPkt = FALSE;
	int pktSentNum=0;
	int pktLostNum=0;
	int pktErrNum=0;
	int delayGap;
	int argv2;

	struct iphdr *iph = (struct iphdr *)rcv_buffer;
	bool addr_flg = FALSE;
	struct sockaddr_in dest;
	struct sockaddr_ll mac1_addr, mac2_addr;

	/* create packet socket */
	packet_socket = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if (packet_socket == -1)
	{
		perror("socket()");
		exit(-1);
	}

	/*set the address for destination mac in sockadd_ll address*/
	strcpy (ifName, DEFAULT_IF);
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(packet_socket, SIOCGIFINDEX, &if_idx) < 0)
	perror("ioctl");
	//get the first mac address
	mac1_addr.sll_ifindex = if_idx.ifr_ifindex;
	mac1_addr.sll_halen = ETH_ALEN;
	mac1_addr.sll_family = AF_PACKET;
	mac1_addr.sll_protocol = htons(ETH_P_IP);
	my_arp(argv[3],&mac1_addr);
	//get the second mac address
	mac2_addr.sll_ifindex = if_idx.ifr_ifindex;
	mac2_addr.sll_halen = ETH_ALEN;
	mac2_addr.sll_family = AF_PACKET;
	mac2_addr.sll_protocol = htons(ETH_P_IP);
	my_arp(argv[4],&mac2_addr);

	len_ll = sizeof(struct sockaddr_ll);

	if(strstr("dleon", argv[1]) == NULL || atoi(argv[2]) == 0) { //check if user enters correct inputs
		printf("invalid input");
	return 0;
	}
	else if(*argv[1] == 'l'|| *argv[1] == 'e'){
		rand_thrsld = (RAND_MAX/100 * atoi(argv[2]));
		printf("threshold is %lu\n", rand_thrsld);
	}
	else if(*argv[1] == 'd'|| *argv[1] == 'o')
		argv2=atoi(argv[2]);

	while(buttonPressed != 's'  || buttonPressed==EOF)
	{
		buttonPressed = getchar();

		FD_ZERO (&read_fds);
		FD_SET (packet_socket, &read_fds);
		timeout.tv_sec = TIMEOUT_SEC;
		timeout.tv_usec = TIMEOUT_uSEC;
		len_addr = sizeof (rcv_addr);
		/* use select() to receive packet */
		z = select (packet_socket+1,&read_fds, NULL, NULL, &timeout);
		if (z == -1) {
			 perror("select()");
			 continue;
		}
		else if (!z); //printf("..."); /* timeout is represented by three dots */

		else if ( FD_ISSET(packet_socket, &read_fds))
		{
			/* receive frame - copy to receive buffer */
			pck_size = recvfrom (packet_socket, rcv_buffer, BUFFER_SIZE, 0,(struct sockaddr *) &rcv_addr, &len_addr);
			if (pck_size == -1) {
			   perror("recvfrom()");
			   continue;
			}
			if (pck_size > 0)
			{
				dest.sin_addr.s_addr = iph->daddr;                  //optimize using rcv_addr
				if (!strcmp(argv[3],inet_ntoa(dest.sin_addr)))
				 addr_flg = FALSE;
				else if (!strcmp(argv[4],inet_ntoa(dest.sin_addr)))
				 addr_flg = TRUE;
				else
				{
				  pck_size = 0;
				  //addr_flg = 0;
				  goto wrongpacket;
				}
			 }
		}

		//---------------------> call the emulation function with send function here (pck_size as flag).

		if (pck_size > 0)
		{
			pktSentNum++;

			switch (*argv[1])
			{
				case 'd' :

				 //	fprintf(fptr, "%dth delayed packet sent				%s		%s\n",
				//		pktSentNum, argv[3], argv[4]);
					usleep(argv2); //in usec
					sendPkt=TRUE;
					break;

				case 'l' :

					if ( rand() <= rand_thrsld){
						pktLostNum++;
						fprintf(fptr, "%dth pkt dropped from %d incoming pkt		%s		%s\n",
							pktLostNum, pktSentNum, argv[3], argv[4]);
					}
					else{
						sendPkt=TRUE;
					}
					break;

				case 'e' :

					if ( rand () <= rand_thrsld)
					{
						printf("error packet sent\n");
						pktErrNum++;
						memcpy ( rcv_buffer+20+20, "error", 5); // considering iphdr = 20 bytes & tcphdr = 20bytes
						fprintf(fptr, "%dth erroneous pkt from incoming %d pkt		%s		%s\n",
							pktErrNum, pktSentNum, argv[3], argv[4]);
					}
					sendPkt=TRUE;
					break;

				case 'o' :

					delayGap = argv2;
					/* detail:
					 * suppose delayGap = 3 then instead of sending
					 * 1 2 3 4 5 6 7 8 9 10 11 ...
					 * we send 1 2 4 3 5 7 6 7 8 10 9 11 ... (swap every multiple of 3 pkt and its next pkt)
					 * the drawback of this method is that if the number of last packet is a multiple of
					 * "delayGap" then only that one packet will not be sent ... simple solution is
					 * if(this is true) then send that packet after exiting the while(1) loop...right now this
					 * problem should be negligible as many packets are sent
					 */
					sendPkt=TRUE;

					if(pktSentNum % delayGap == 0){
						memcpy(rcv_buffer_temp, rcv_buffer, pck_size);
						sendPkt=FALSE;
					}
					else if(pktSentNum % delayGap == 1){
						sendOutofOrderPkt = TRUE;
						fprintf(fptr, "%dth and %dth packet are swapped			%s		%s\n",
								pktSentNum-1, pktSentNum, argv[3], argv[4]);
					}
					break;

				default:	printf("sent packet without emulation\n");
							sendPkt=TRUE;
							break;
			}

			if(sendPkt){
				if (addr_flg)
				  z = sendto(packet_socket, rcv_buffer, pck_size, 0,(struct sockaddr*)&mac2_addr,len_ll);
				else
				  z = sendto(packet_socket, rcv_buffer, pck_size, 0,(struct sockaddr*)&mac1_addr,len_ll);

				if ( z < 0) perror ("sendto");

				sendPkt=FALSE; //clear cond.
			}

			if(sendOutofOrderPkt){
				if (addr_flg)
				  z = sendto(packet_socket, rcv_buffer_temp,pck_size,0,(struct sockaddr*)&mac2_addr,len_ll);
				else
				  z = sendto(packet_socket, rcv_buffer_temp,pck_size,0,(struct sockaddr*)&mac1_addr,len_ll);

				if ( z < 0) perror ("sendto");

				sendOutofOrderPkt = FALSE;
			}
			pck_size = 0;
		}

		wrongpacket:/*do nothing*/ ;
	}
		gettimeofday(&program_finished , NULL); //finish program

		fprintf (fptr, "Begin at: %s\nTotal time elapsed : %.0lf us %d\n\n", asctime (time_begin),
			time_diff(program_begin ,program_finished));

	fprintf(fptr, "\nNetwork Emulation of overall %d packets transmitted\n\n", pktSentNum);

	switch (*argv[1])
	{
		case 'd' :
		  fprintf(fptr, "Each packet is delayed by %d microsecond\n", *argv[2]);
		  break;
		case 'l' :
		  fprintf(fptr, "%.2f percent of all packets were lost\n", (double) pktLostNum/pktSentNum*100);
		  break;
		case 'e' :
		  fprintf(fptr, "%.2f percent of all packets had errors\n", (double) pktErrNum/pktSentNum*100);
		  break;
		case 'o' :
		  fprintf(fptr, "The packet is out of order every %d packets\n", *argv[2]);
		  break;
		default  :
		  fprintf(fptr, "The command is not valid\n");
		  break;
	}

	fclose(fptr);

	close (packet_socket);
	return 0;
}

double time_diff(struct timeval t1 , struct timeval t2)
{
double t1_microsec , t2_microsec , diff;

t1_microsec = (double)t1.tv_sec*1000000 + (double)t1.tv_usec;
t2_microsec = (double)t2.tv_sec*1000000 + (double)t2.tv_usec;

diff = t2_microsec - t1_microsec;

return diff; //return in microseconds
}

void term_reset() {
        tcsetattr(STDIN_FILENO,TCSANOW,&stdin_orig);
        tcsetattr(STDIN_FILENO,TCSAFLUSH,&stdin_orig);
}

void term_nonblocking() {
        struct termios newt;
        tcgetattr(STDIN_FILENO, &stdin_orig);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // non-blocking
        newt = stdin_orig;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        atexit(term_reset);
}

