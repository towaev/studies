#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

using namespace std;

#define BUFLEN 300

void error(const char *errMsg, int *socket) {
    cout << errMsg << endl;
    if(socket != NULL) {
        close(*socket);
    }
	exit(1);
}

void sendNew(int *socket, char *data, size_t bufLen) {
    size_t status = 0;
    size_t sendMsg = bufLen+1;
    char *buf = new char[bufLen+1];
	strcpy(buf, data);
    do {
        status = send(*socket, buf, sendMsg, 0);
        if(status > 0) {
            for(size_t i = 0; i < bufLen; ++i) {
                if(i < status) {
                    buf[i] = data[i];
                } else {
                    data[i-status] = data[i];
                }
            }
            sendMsg -= status;

            if(sendMsg != 0) {
                buf[status] = '\0';
            }
         //   cout << ">> " << buf << endl;
        } else {
		error("Couldn't send message:\n", socket);
        }
    } while(sendMsg > 0);
}

int readNew(int *socket, char *data, size_t bufLen) {
    size_t rc = 0;
    size_t length = 0;
    char *buf = new char[bufLen + 1];

    while(length < bufLen) {
        rc = (size_t)recv(*socket, buf, bufLen - length, 0);
        if(rc > 0) {
            for(size_t i = length; i < length + rc; i++) {
                data[i] = buf[i-length];
            }
            length += rc;
            if(rc != bufLen) {
                buf[rc] = '\0';
            }
            cout << "<< " << buf << endl;
		delete []buf;
   	 return length;
        } else if(rc == 0) {
            cout << "Connection ended" << endl;
		delete []buf;
            return 0;
        } else {
            error("Couldn't recive data. Recv error:\n", socket);
        }
    }
	delete []buf;
}



int main(int argc, char *argv[]) {
    int connectSocket = SOCK_CLOEXEC;
	if (argc < 2) {
		cout <<"Usage: "<< argv[0] <<"host port msg...\n";
		exit(EXIT_FAILURE);
	}

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    const char *node =argv[1];
    const char *port =argv[2]; 
    cout << "Starting up client" << endl;
    int iResult = getaddrinfo(node, port, &hints, &serverInfo);
    if(iResult < 0) {
        error("Failed to get peer name:\n", NULL);
    }
    cout << "getaddrinfo succes" << endl;
    char* msg = "y";
    struct addrinfo *p;
    for(p = serverInfo; p != NULL; p = p->ai_next) {
        connectSocket = socket(
                serverInfo->ai_family,
                serverInfo->ai_socktype,
                serverInfo->ai_protocol
        );
        if(connectSocket < 0) {
            continue;
        }
        iResult = setsockopt(connectSocket, SOL_SOCKET, SO_REUSEADDR, &msg, sizeof msg);
        if(iResult < 0) {
            error("Setsockopt failed:\n", NULL);
        }
	cout << "Setsockopt succes" << endl;
        iResult = connect(connectSocket, p->ai_addr, p->ai_addrlen);

        if(iResult < 0) {
            error("Couldn't connect to specified socket:\n", &connectSocket);
            connectSocket = SOCK_CLOEXEC;
            continue;
        }
	
        break;
    }
    freeaddrinfo(serverInfo);

    if(connectSocket == SOCK_CLOEXEC) {
        error("Unable to connect to the server\n", NULL);
    }  

    string inputLine;
    char *data = new char[BUFLEN];
    cout << "Input data" << endl;
    do {
        getline(cin, inputLine);
       /* if(inputLine.length() >= BUFLEN) {
            inputLine = inputLine.substr(0, BUFLEN);
            strcpy(data, inputLine.c_str());
        } else {
            strcpy(data, inputLine.c_str());
            for(size_t i = (size_t) inputLine.length(); i < BUFLEN; ++i) {
                data[i] = '\0';
            }
        }*/
	if(inputLine.length() < BUFLEN) {          
            strcpy(data, inputLine.c_str());
            for(size_t i = (size_t) inputLine.length(); i < BUFLEN; ++i) {
                data[i] = '\0';
            }
        }
	if (inputLine.compare(":dc ") == 0) {
            sendNew(&connectSocket, data, BUFLEN);
            int dcResult = readNew(&connectSocket, data, BUFLEN);
	    if (dcResult > 0)
		cout << "Succesfull disconnect" << endl;
		break;
        }
	else{
		sendNew(&connectSocket, data, strlen(data));
		iResult = readNew(&connectSocket, data, BUFLEN);
	}
    } while(iResult > 0);

    close(connectSocket);
    cout << "Succesfull finish" << endl;
    delete data;

    return 0;
}

