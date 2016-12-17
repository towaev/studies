#include <cstdio>  
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>

#define _WIN32_WINNT 0x501
#define WSAVersion MAKEWORD(2, 0)
#define BUFLEN 100

#pragma comment (lib, "Ws2_32.lib")


using namespace std;

struct PEER {
	SOCKET socket;
	HANDLE handle;
	size_t id;
};
const int peerMaxCount = 10;
PEER* peers[peerMaxCount];

CONST HANDLE thMutex = CreateMutex(NULL, FALSE, NULL);
size_t peerCount = 0;

void errorMessage(const char*msg, const SOCKET *sock) {
	cout << msg << endl;
	if (sock != NULL) {
		closesocket(*sock);
		WSACleanup();
	}
}

HANDLE* killAll() {
	vector<HANDLE> handlePeers;
	WaitForSingleObject(thMutex, INFINITE);
	for (size_t i = 0; i < peerMaxCount; i++) {
		if (peers[i] != NULL) {
			handlePeers.push_back(peers[i]->handle);
			shutdown(peers[i]->socket, SD_BOTH);
			closesocket(peers[i]->socket);
			delete peers[i];
			peerCount--;
			WaitForSingleObject(peers[i]->handle, INFINITE);
			break;			
		}
	}
	ReleaseMutex(thMutex);
	return &handlePeers[0];
}
void killPeerID(size_t id) {
	WaitForSingleObject(thMutex, INFINITE);
	for (size_t i = 0; i < peerMaxCount; i++) {
		if (peers[i] != NULL && peers[i]->id == id) {			
			shutdown(peers[i]->socket, SD_BOTH);
			closesocket(peers[i]->socket);
			WaitForSingleObject(peers[i]->handle, INFINITE);
			delete peers[i];
			peerCount--;			
			break;
		}
	}
	cout << "Peer " << id << " is disconnected from server" << endl;
	ReleaseMutex(thMutex);
}

int readNew(PEER *peer, char *data, size_t buf_len) {
	size_t rc = 0;
	size_t length = 0;
	char *buf = new char[buf_len + 1];

	while (length < buf_len) {
		rc = (size_t)recv(peer->socket, buf, buf_len - length, 0);
		if (rc == SOCKET_ERROR) {			
			return rc;
		} else if (rc > 0) {
			for (size_t i = length; i < length + rc; i++) {
				data[i] = buf[i - length];
			}
			length += rc;
			buf[rc] = '\0';
			cout << +peer->id << " << " << buf << endl;			
			return length;
		}
		else if (rc == 0) {
			cout << "Peer ID " << +peer->id << " has closed the connection" << endl;
			return 0;
		}
		else {
			if (rc != INVALID_SOCKET) {
				errorMessage("Recv failed:\n", &peer->socket);
			}
			return INVALID_SOCKET;
		}
	}
	delete buf;
	return length;
}
void sendNew(PEER *peer, char *data, size_t buf_len) {

	size_t status = 0;
	size_t msgLen = buf_len;
	char *buf = new char[buf_len+1]();

	do {
		status = (size_t)send(peer->socket, data, msgLen, 0);
		if (status > 0) {
			memcpy(buf, data, msgLen);			
			msgLen -= status;			
			cout << +peer->id << " >>" << buf << endl;
		}
		else {
			errorMessage("Send failed:\n", &peer->socket);
		}
	} while (msgLen > 0);
	delete buf;
	return;
}


DWORD WINAPI threadHandler(LPVOID newPeer) {
	WaitForSingleObject(thMutex, INFINITE);
	PEER *client = (PEER *)newPeer;
	if (peerCount < peerMaxCount) {
		client->id = peerCount;
		peers[peerCount] = client;
		peerCount++;
	}
	else
	{
		cout << "Reached maximum number of connections." << endl;
	}
	ReleaseMutex(thMutex);
	size_t localId = client->id;
	cout << "Peer[" << +localId << "] has connected to server" << endl;
	int state = 0;
	char *recData = new char[BUFLEN];
	do {
		state = readNew(client, recData, BUFLEN);
		if (state > 0) {
			sendNew(client, recData, state);
		}
	} while (state > 0);
	delete recData;
	return 0;
}

//listening thread
DWORD WINAPI acceptThread(LPVOID tempSocket) {
	PEER *newPeer;
	while (true) {
		newPeer = new PEER;
		newPeer->socket = accept((SOCKET)tempSocket, NULL, NULL);
		if (newPeer->socket < 0) {
			delete newPeer;
			errorMessage("Accept failed:\n", NULL);
			continue;
		}
		else if (newPeer->socket == INVALID_SOCKET) {
			delete newPeer;
			break;
		}

		if (peerCount < peerMaxCount) {
			newPeer->handle = CreateThread(NULL, 0, &threadHandler, newPeer, 0, NULL);
		}
		else {
			shutdown((newPeer)->socket, SD_BOTH);
			closesocket((newPeer)->socket);
			delete newPeer;

			cout << "Reached maximum number of connections." << endl;
		}
	}

	HANDLE *handles = killAll();

	
	if (handles != NULL) {
		//waiting for threads 
		WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, TRUE, INFINITE);
	}
	return 0;
}

int main() {

	setlocale(LC_ALL, "");
	WSADATA wsaData;
	int state;
	state = WSAStartup(WSAVersion, &wsaData);
	if (state != 0) { cout << "WSAStartup failed." << endl; return -1; }

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	const char* portname = "7500";
	state = getaddrinfo(NULL, portname, &hints, &serverInfo);
	if (state != 0) {
		errorMessage("Failed to get address:\n", NULL);
		return(1);
	}		
	SOCKET lstSocket = INVALID_SOCKET;
	
	lstSocket = socket(serverInfo->ai_family,
		serverInfo->ai_socktype,
		serverInfo->ai_protocol);
	if (lstSocket < 0) {
		errorMessage("Couldn't create socket:\n", NULL);
	}

	state = bind(lstSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (state < 0) {
		errorMessage("Bind failed:\n", &lstSocket);			
	}	
	freeaddrinfo(serverInfo);	

	state = listen(lstSocket, peerMaxCount);
	if (state < 0) {
		errorMessage("Listen failed:\n", &lstSocket);
		return(4);
	}
	HANDLE thAccept = CreateThread(NULL, 0, &acceptThread, (LPVOID)lstSocket, 0, NULL);
	string inpL;
	while (true) {
		getline(cin, inpL);
		if (inpL.compare(":q") == 0) {				
			char recvbuf[BUFLEN];
			int recvbuflen = BUFLEN;
			shutdown(lstSocket, SD_BOTH);		
			closesocket(lstSocket);
			break;
		}
		else if (inpL.substr(0, 3).compare(":k ") == 0) {
			char *temp;
			size_t parsedId =(size_t)strtol(inpL.substr(3, inpL.length()).c_str(), &temp, 10);
			killPeerID(parsedId);
		}
	}

	//waiting for listening thread
	WaitForSingleObject(thAccept, INFINITE);

	if (WSACleanup() != 0) { cout << "WSACleanup failed." << endl; return -1; }

	cout << "Server has shutdown" << endl;

	return 0;
}

