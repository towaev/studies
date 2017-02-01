/*TCP multithreaded client (windows)
Created by Eugene Vorobej (gr.435013) as an university project project
server commands:
:k userId - To disconnect specific user 
(e.g. type  ":k 1"
to disconnect user with ID 1)

:q - To properly close server 

client command list:
:dc - disconnect user who send this
:a - parking
:b - taking car away
:c - parking bill and сhange
:d - payment hystory
:h - for help
*/

#include <cstdio>  
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <time.h> 
#define _WIN32_WINNT 0x501
#define WSAVersion MAKEWORD(2, 0)
#define BUFLEN 100

#pragma comment (lib, "Ws2_32.lib")


using namespace std;

class PEER {
public:
	SOCKET socket;
	HANDLE handle;
	size_t id;	
	string carNumber;	
	int parkPrice;
	int recievedPayment;
	bool isParked =false;
};
const int peerMaxCount = 2;
PEER* peers[peerMaxCount];
string paymentHistory = "\nPayment history:\n";
int paymentCount = 0;
int totalPaymentSum = 0;
const char* pass = "admin";

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
			peers[i] = NULL;
			peerCount--;
			//WaitForSingleObject(peers[i]->handle, INFINITE);
			//break;			
		}
		
	}
	ReleaseMutex(thMutex);
	if (handlePeers.size() > 0)
		return &handlePeers[0];
	else return NULL;
}
void killPeerID(size_t id) {
	WaitForSingleObject(thMutex, INFINITE);
	bool peerExists = false;
	for (size_t i = 0; i < peerMaxCount; i++) {
		if (peers[i] != NULL && peers[i]->id == id) {
			peerExists = true;
			shutdown(peers[i]->socket, SD_BOTH);
			closesocket(peers[i]->socket);
			WaitForSingleObject(peers[i]->handle, INFINITE);
			delete peers[i];
			peers[i] = NULL;
			peerCount--;
			//cout << "Peer " << id << " is disconnected from server" << endl;
			break;
		}
	}
	if (!peerExists) 
		cout << "There is no such peer" << endl;
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
			killPeerID(peer->id);
			return 0;
		}
		else {
			if (rc != INVALID_SOCKET) {
				errorMessage("Recv failed:\n", &peer->socket);
			}
			return INVALID_SOCKET;
		}
	}
	delete[] buf;
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
	delete[] buf;
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
	
	size_t localId = client->id;
	cout << "Peer[" << +localId << "] has connected to server" << endl;
	int state = 0;
	ReleaseMutex(thMutex);
	char *recData = new char[BUFLEN];



	//--------------------------------------------------------------
	/* Обработка сообщений от клиента*/
	do {
		state = readNew(client, recData, BUFLEN);
		string str ="";

		if (state > 0) {
			int len = strlen(recData);
			char *bufData = new char[len+1];			
			if (strncmp(recData,":a " , 3) == 0) {				
				if (len - 3 > 12) len = 16;
				for (int i = 0; i < len - 3; i++) {
					bufData[i] = recData[i + 3];
					if (i == len - 4) { bufData[i+1] = '\0'; 
					break;
					}

				}				
				str = str + bufData;			
				client->carNumber = str;		

				srand(time(NULL));

				client->parkPrice = 100*(rand()%20);
				str = "Car " + client->carNumber + " is parked\n";	
				client->isParked = true;
				char* sendbuf = new char[str.length() + 1];
				strcpy(sendbuf, str.c_str());
				sendNew(client, sendbuf, strlen(sendbuf));
				delete[]sendbuf;
			}
			else if (strncmp(recData, ":b ", 3) == 0) {	
				if (client->isParked == true) {
					char intstr[10];
					_itoa(client->parkPrice, intstr, 10);
					str = "Car " + client->carNumber + ".\n   Parking cost: " 
						+ string(intstr) + "\n";
					paymentHistory += str;
					str = "You can take your car " + client->carNumber + " after successful payment."
						"\nParking cost: " + string(intstr) + "\n";
					char* sendbuf = new char[str.length() + 1];
					strcpy(sendbuf, str.c_str());
					sendNew(client, sendbuf, strlen(sendbuf));
					delete[]sendbuf;
				}
				else
				{
					str = "You haven't parked your car yet.\n";
					char* sendbuf = new char[str.length()+1];
					strcpy(sendbuf, str.c_str());
					sendNew(client, sendbuf, strlen(sendbuf));
					delete[]sendbuf;
				}
			}
			else if (strncmp(recData, ":c ", 3) == 0) {
				if (client->isParked == true) {
					if (len - 3 > 11) len = 14;
					for (int i = 0; i < len - 3; i++) {
						bufData[i] = recData[i + 3];
						if (i + 3 == len) bufData[i] = '\0';
					}

					str = str + bufData;
					int payment = stoi(str);
					if (payment < client->parkPrice)
					{
						str = "Not enough money. Please try again.\n";
						char* sendbuf = new char[str.length() + 1];
						strcpy(sendbuf, str.c_str());
						sendNew(client, sendbuf, strlen(sendbuf));
						delete[]sendbuf;
					}
					else {
						client->recievedPayment = payment;

						int change = client->recievedPayment - client->parkPrice;
						str = "Recieved payment: " + to_string(payment) +
							"\nChange: " + to_string(change);
						paymentHistory += str;
						str += "\nThanks! Come again!\n";
						client->isParked = false;
						char* sendbuf = new char[str.length() + 1];
						strcpy(sendbuf, str.c_str());
						sendNew(client, sendbuf, strlen(sendbuf));
						totalPaymentSum += client->parkPrice;
						delete[]sendbuf;
					}
				}
				else {
					str = "You haven't parked your car yet.\n";
					char* sendbuf = new char[str.length()+1];
					strcpy(sendbuf, str.c_str());
					sendNew(client, sendbuf, strlen(sendbuf));
					delete[]sendbuf;
				}
			}
			else if (strncmp(recData, ":d ", 3) == 0) {
				str = "Please enter password:\n";
				char* sendbuf = new char[str.length() + 1];
				strcpy(sendbuf, str.c_str());
				sendNew(client, sendbuf, strlen(sendbuf));
				int rn = 0;
				rn = readNew(client, recData, BUFLEN);
				if (rn > 0) {
					
					if (strncmp(recData, pass, strlen(pass)) == 0){
						str = "Correct password\nTotal sum recieved:"+to_string(totalPaymentSum);
						str += paymentHistory;
						char* sendbuf = new char[str.length() + 1];
						strcpy(sendbuf, str.c_str());
						sendNew(client, sendbuf, strlen(sendbuf));
						delete[]sendbuf;
					}
					else{
						str = "Invalid password.\n";
						char* sendbuf = new char[str.length() + 1];
						strcpy(sendbuf, str.c_str());
						sendNew(client, sendbuf, strlen(sendbuf));
						delete[]sendbuf;
					}						
				}			
				delete[]sendbuf;
			}
			else if (strncmp(recData, ":dc ", 4) == 0) {		
				
				str = "User was succesfully disconnected from server\n";
				char* sendbuf = new char[str.length() + 1];
				strcpy(sendbuf, str.c_str());
				sendNew(client, sendbuf, strlen(sendbuf));				
				state = 0;
				delete[]sendbuf;	

			}
			else if (strncmp(recData, ":h ", 3) == 0) {
				str = "command list:\n :dc - disconnect user who send this\n :a \%carnumber - parking "
					"\n :b - taking car away \n :c \%payment - parking bill and change\n"
					" :d - payment hystory(admin only) \n :h - for command list\n" ;
				char* sendbuf = new char[str.length() + 1];
				strcpy(sendbuf, str.c_str());
				sendNew(client, sendbuf, strlen(sendbuf));
				delete[]sendbuf;
			}
			else
			{
				str = "Send `:h ` for help\n";
				char* sendbuf = new char[str.length() + 1];
				strcpy(sendbuf, str.c_str());
				sendNew(client, sendbuf, strlen(sendbuf));
				delete[]sendbuf;
			}
			delete[]bufData;
		}
	} while (state > 0);
	
	delete [] recData;
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
			//killPeerID(newPeer->id);

		}
		else {
			shutdown((newPeer)->socket, SD_BOTH);
			closesocket((newPeer)->socket);
			delete newPeer;

			cout << "Reached maximum number of connections." << endl;
		}
	}

	HANDLE *handles = killAll();
	//waiting for threads 
	if (handles != NULL) {		
		WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, TRUE, INFINITE);		
	}	
	return 0;
}




int main(int argc, char *argv[]) {

	setlocale(LC_ALL, "");
	WSADATA wsaData;
	int state;
	if (argc != 2) {
		cerr << "Usage:"<<argv[0] <<" port\n";
		exit(EXIT_FAILURE);
	}
	state = WSAStartup(WSAVersion, &wsaData);
	if (state != 0) { cout << "WSAStartup failed." << endl; return -1; }

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	const char* portname = argv[1];
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

