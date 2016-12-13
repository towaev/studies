
#include <stdio.h>  
#include <winsock.h>



int main(){
	WSADATA wsaData;   // if this doesn't work
					   //WSAData wsaData; // then try this instead
					   // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:


	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) { printf("WSAStartup failed.\n"); return -1; }
		

	struct sockaddr_in local;
	int s;
	int s1;
	int rc;
	char buf[1];

	local.sin_family = AF_INET;
	local.sin_port = htons(7500);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	//local.sin_addr.s_addr = htonl(INADDR_ANY);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		perror("socket call failed");
		WSACleanup();
		return(1);
	}
	rc = bind(s, (struct sockaddr *)&local, sizeof(local));
	if (rc < 0)
	{
		perror("bind call failure");
		WSACleanup();
		return(2);
	}
	rc = listen(s, 5);
	if (rc)
	{
		perror("listen call failed");
		WSACleanup();
		return(3);
	}
	s1 = accept(s, NULL, NULL);
	if (s1 < 0)
	{
		perror("accept call failed");
		WSACleanup();
		return(4);
	}
	else printf("Connection established \n");
	rc = recv(s1, buf, 1, 0);
	if (rc <= 0)
	{
		perror("recv call failed");
		WSACleanup();
		return(5);
	}
	printf("Request recieved: \n %c\n", buf[0]);
	rc = send(s1, "R", 1, 0);
	if (rc <= 0)
		perror("send call failed");
	else printf("Reply sent \n");
	
	if (WSACleanup() != 0) { printf("WSACleanup failed.\n"); return -1;	}
	getch();

	return 0;
}