#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

int main( void )
{
	struct sockaddr_in peer;
	int s;
	int rc;
	char buf[ 1 ];

	peer.sin_family = AF_INET;
	peer.sin_port = htons( 7500 );
	peer.sin_addr.s_addr = inet_addr( "192.168.56.1" );

	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( s < 0 )
	{
		perror( "socket call failed" );
		return( 1 );
	}
/*@.bp*/
	printf("Establishing connection\n");
	rc = connect( s, ( struct sockaddr * )&peer, sizeof( peer ) );
	if ( rc )
	{
		perror( "connect call failed" );
		return( 1 );
	}
	else
	printf("Connection established\n");

	rc = send( s, "1", 1, 0 );
	if ( rc <= 0 )
	{
		perror( "send call failed" );
		return( 1 );
	}
	else
	printf("Message sent\n");
	rc = recv( s, buf, 1, 0 );
	if ( rc <= 0 )
		perror( "recv call failed" );
	else
		printf( "Reply recieved: \n %c\n", buf[ 0 ] );
	return( 0 );
}

