
#include <cstdio>  
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define WSAVersion MAKEWORD(2, 0)
#define BUFLEN 100

typedef __int8 peer_t;
typedef __int16 data_t;


struct PEER {
    SOCKET socket;
    HANDLE handle;
    peer_t id;
};
const int peerMaxCount = 10;
PEER* peers[peerMaxCount];

CONST HANDLE thMutex = CreateMutex(NULL, FALSE, NULL);
peer_t peerCount = 0;

void errorMessage(const char*msg, const SOCKET *sock){
	cout << msg << endl; 
	if (sock != NULL)
		closesocket(*sock);
	WSACleanup();
}
int readNew(PEER *peer, char *data, __int16 buf_len) {
    data_t rc = 0;
    data_t length = 0;
    char *buf= new char[buf_len+1] ;

    while(length < buf_len) {
        rc = recv(peer->socket, buf, buf_len - length, 0);
        if(rc > 0) {
            for(data_t i = length; i < length + rc; i++) {
                data[i] = buf[i-length];
            }
            length += rc;
            buf[rc] = '\0';
            cout << +peer->id << " << " <<  buf << endl; 
        } else if(rc == 0) {
            cout << "Peer ID " << +peer->id << " has closed the connection" << endl; 
            return 0;
        } else {
            if(rc != INVALID_SOCKET) {
                errorMessage("Recv failed:\n", &peer->socket);
            }
            return INVALID_SOCKET;
        }
    }
    return length;
}
void sendNew(PEER *peer, char *data, data_t buf_len) {
    data_t status = 0;
    data_t msgLen = buf_len;
    char *buf= new char[buf_len+1] ;

    do {
        status = send(peer->socket, data, msgLen, 0);
        if(status > 0) {
            for(int i = 0; i < buf_len; ++i) {
                if(i < status) {
                    buf[i] = data[i];
                } else {
                    data[i-status] = data[i];
                }
            }
            msgLen -= status;
            buf[status] = '\0';
            cout << +peer->id << " >>" <<  buf << endl; 
        } else {
            errorMessage("Send failed:\n", &peer->socket);
        }
    } while(msgLen > 0);
}


DWORD WINAPI threadHandler(LPVOID newPeer) {
	WaitForSingleObject(thMutex, INFINITE);
    PEER *client = (PEER *) newPeer;
	if (peerCount < peerMaxCount){
		client->id = peerCount;
		peers[peerCount] = client;
		peerCount++;
		}
	else
	{	   
        cout << "Reached maximum number of connections." <<endl;
	}	
	ReleaseMutex(thMutex);
	peer_t localId = client->id;

    int state = 0;
    char *recData = new char[BUFLEN];
    do {
        state = readNew(client, recData, BUFLEN);
        if(state > 0) {
            sendNew(client, recData, BUFLEN);
        }
    } while(state > 0);

    
    cout << "Peer " << localId << " is offline" << endl;
    
	WaitForSingleObject(thMutex, INFINITE);
	for(peer_t i = 0; i < peerMaxCount; i++) {
        if(peers[i] == client) {
           
			shutdown(peers[i]->socket, SD_BOTH);
			closesocket(peers[i]->socket);
			delete peers[i];			
            peerCount--;
            break;
        }
    }
    ReleaseMutex(thMutex);
    delete recData;
    return 0;
}

//listening thread
DWORD WINAPI acceptThread(LPVOID tempSocket) {
    PEER *newPeer;
    while(true) {
        newPeer = new PEER;
        newPeer->socket = accept((SOCKET) tempSocket, NULL, NULL);
        if(newPeer->socket < 0) {
            delete newPeer;
            errorMessage("Accept failed:\n", NULL);
            continue;
        } else if(newPeer->socket == INVALID_SOCKET) {
            delete newPeer;
            break;
        }

		if (peerCount < peerMaxCount){
            newPeer->handle = CreateThread(NULL, 0, &threadHandler, newPeer, 0, NULL);	
        } 
		else {
            shutdown((newPeer)->socket, SD_BOTH);
			closesocket((newPeer)->socket);
			delete newPeer;
			
            cout << "Reached maximum number of connections." <<endl;
        }
    }

    HANDLE *handles = NULL;
	vector<HANDLE> hPeers;
    WaitForSingleObject(thMutex, INFINITE);
    for(peer_t i = 0; i < peerMaxCount; i++) {
        if(peers[i] != NULL) {
            hPeers.push_back(peers[i]->handle);            
			for(peer_t k = 0; k < peerMaxCount; k++) {
				if(peers[k] == peers[i]) {
					shutdown(peers[k]->socket, SD_BOTH);
					closesocket(peers[k]->socket);
					delete peers[k];					
					peerCount--;
					break;
				}
			}
        }
    }
    ReleaseMutex(thMutex);
    if(handles != NULL) {
		//waiting for threads 
        WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, TRUE, INFINITE);
    }
    return 0;
}

int main(){

	setlocale(LC_ALL, "");
	WSADATA wsaData;   
	int state;
	state = WSAStartup(WSAVersion, &wsaData);
	if (state	!= 0) { cout <<"WSAStartup failed."<<endl; return -1; }
	
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	PCSTR portname = "7500";
	state = getaddrinfo(NULL, portname, &hints, &serverInfo);
	if (state != 0) {
		errorMessage("Failed to get address:\n",NULL);
		return(1);
	}
	char msg = 'y';
	struct addrinfo	*ts;
	SOCKET tempSocket = INVALID_SOCKET;
	for ( ts =serverInfo;ts !=NULL; ts=ts ->ai_next){
		tempSocket = socket(
			serverInfo ->ai_family,
		serverInfo ->ai_socktype,
		serverInfo ->ai_protocol);
		if(tempSocket < 0) {
            continue;
        }
        state = setsockopt(tempSocket, SOL_SOCKET, SO_REUSEADDR, &msg, sizeof msg);
        if(state < 0) {
			errorMessage("Failed to set socket options:\n",NULL);			
			return(2);            
        }
        state = bind(tempSocket, ts->ai_addr, ts->ai_addrlen);
        if(state < 0) {
            errorMessage("Bind failed:\n", &tempSocket);
            continue;
        }
        break;
	};
	freeaddrinfo(serverInfo);

    if(ts == NULL) {
        errorMessage("Bind failed:\n", &tempSocket);
		return(3);
    }

    state = listen(tempSocket, peerMaxCount);
    if(state < 0) {
        errorMessage("Listen failed:\n", &tempSocket);
		return(4);
    }

	HANDLE thAccept = CreateThread(NULL, 0, &acceptThread, (LPVOID) tempSocket, 0, NULL);
    string inpL;
    while(true) {
		getline(cin, inpL);

        if(inpL.compare("quit") == 0) {
            shutdown(tempSocket, SD_BOTH);
            closesocket(tempSocket);
            break;     
		}
    }
	//waiting for listening thread
    WaitForSingleObject(thAccept, INFINITE);

	if (WSACleanup() != 0) { cout <<"WSACleanup failed." <<endl; return -1;	}

	cout << "Server has shutdown" << endl; 

	return 0;
}
