#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

using namespace std;

#define BUFLEN 100 
typedef uint16_t data_t;

void error(const char *errMsg, int *socket) {
    cout << errMsg << endl;
    if(socket != NULL) {
        close(*socket);
    }
	exit(1);
}

void sendNew(int *socket, char *data, data_t bufLen) {
    data_t status = 0;
    data_t sendMsg = bufLen;
    char buf[bufLen];
    do {
        status = send(*socket, data, sendMsg, 0);
        if(status > 0) {
            for(data_t i = 0; i < bufLen; ++i) {
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
            cout << ">> " << buf << endl;
        } else {
			error("Couldn't send message:\n", socket);
        }
    } while(sendMsg > 0);
}

int readNew(int *socket, char *data, data_t bufLen) {
    data_t rc = 0;
    data_t length = 0;
    char buf[bufLen];

    while(length < bufLen) {
        rc = recv(*socket, buf, bufLen - length, 0);
        if(rc > 0) {
            for(data_t i = length; i < length + rc; i++) {
                data[i] = buf[i-length];
            }
            length += rc;
            if(rc != bufLen) {
                buf[rc] = '\0';
            }
            cout << "<< " << buf << endl;
        } else if(rc == 0) {
            cout << "Connection ended" << endl;
            return 0;
        } else {
            error("Couldn't recive data. Recv error:\n", socket);
        }
    }
    return length;
}



int main() {
    int connectSocket = SOCK_CLOEXEC;

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    const char *node ="192.168.56.1";
    const char *port ="7500"; 
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
        if(inputLine.compare("quit") == 0) {
            break;
        }
        if(inputLine.length() >= BUFLEN) {
            inputLine = inputLine.substr(0, BUFLEN);
            strcpy(data, inputLine.c_str());
        } else {
            strcpy(data, inputLine.c_str());
            for(data_t i = (data_t) inputLine.length(); i < BUFLEN; ++i) {
                data[i] = '\0';
            }
        }


        sendNew(&connectSocket, data, BUFLEN);

        iResult = readNew(&connectSocket, data, BUFLEN);
    } while(iResult > 0);

    close(connectSocket);
    cout << "Succesfull finish" << endl;
    delete data;

    return 0;
}

