#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <cstring>
using namespace std;
#define BUF_SIZE 500

int main(int argc, char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, j;
	size_t len;
	ssize_t nread;
	char buf[BUF_SIZE];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	Try each address until we successfully connect(2).
	If socket(2) (or connect(2)) fails, we (close the socket
	and) try the next address. */

	rp = result;
	if (rp == NULL) {               /* No address succeeded */
		close(sfd);
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}
	sfd = socket(rp->ai_family, rp->ai_socktype,
               rp->ai_protocol);
	//fprintf(stdout, "Socket: %d\n", sfd);
	
	if (sfd == -1){
		fprintf(stderr, "Could not create socket\n");
		close(sfd);
           	exit(EXIT_FAILURE);
		
	}
	int state = 0;
	state = connect(sfd, rp->ai_addr, rp->ai_addrlen);
	//fprintf(stdout, "Bind return: %d\n", state);	
	if (state ==-1)
	{
		close(sfd);
		fprintf(stderr, "Could not connect to server\n");
		exit(EXIT_FAILURE);        	                   
	}

	freeaddrinfo(result);           /* No longer needed */

	/* Send remaining command-line arguments as separate
	datagrams, and read responses from server */

	//for (j = 3; j < argc; j++) {
	string inputLine;
	char *data = new char[BUF_SIZE]; 
	do{
		getline(cin, inputLine);
		len = inputLine.length() + 1;
		/* +1 for terminating null byte */

		if (len + 1 > BUF_SIZE) {
			fprintf(stderr,
			"Ignoring long message in argument %d\n", j);
			continue;
		}
		strcpy(data, inputLine.c_str());
           	for(size_t i = (size_t) inputLine.length(); i < BUF_SIZE; ++i) {
                	data[i] = '\0';
		}
		if (write(sfd, data, len) != len) {
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}

		nread = read(sfd, buf, BUF_SIZE);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		printf("Received %zd bytes: %s\n", nread, buf);

	} while(nread > 0);

	exit(EXIT_SUCCESS);
}

