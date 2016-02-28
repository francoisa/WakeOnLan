#include <memory.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

static const unsigned BC_MSG_LENGTH = 102;
static unsigned char tosend[BC_MSG_LENGTH];
static const unsigned BC_PORTNUMBER = 0;

bool createPacket(const string& macAddress) {
	if (macAddress.length() != 12) {
		cerr << "Bad MAC address." << endl;
		return false;
	}
	if (macAddress.find_first_not_of("0123456789abcdefABCDEF") == string::npos) {
		unsigned char mac[6];
		/** store mac address **/
		/** first 6 bytes of 255 **/
		
		for (int i = 0; i < 6; ++i) {
			stringstream ss;
			string byte = "0x" + macAddress.substr(i*2, 2);
			ss << std::hex << byte;
			ss >> mac[i];
			char * p;
			long n = strtol(byte.c_str(), &p, 16);
			if ( * p != 0 ) { //my bad edit was here
				cout << "not a number" << endl;
			}
			else {
				mac[i] = n;
			}
			tosend[i] = 0xFF;
		}

		/** append it 16 times to packet **/
		for(int i = 1; i <= 16; i++) {
			memcpy(&tosend[i * 6], &mac, 6 * sizeof(unsigned char));
		} 
	}
	else {
		size_t pos = macAddress.find_first_not_of("0123456789abcdefABCDEF");
		cerr << "Bad MAC address: '" << macAddress << "' at position " << pos << endl;
		return false;
	}
	return true;
}

void sendMagicPacket() {
	SOCKET udpSocket;
	struct sockaddr_in udpClient, udpServer;
	BOOL broadcast = TRUE;

	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		cerr << "Create socket error." << endl;
		return;
	}

	/** you need to set this so you can broadcast **/
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (const char*) &broadcast, sizeof broadcast) == -1) {
		cerr << "setsockopt (SO_BROADCAST)" << endl;
		return;
	}

	udpClient.sin_family = AF_INET;
	udpClient.sin_addr.s_addr = INADDR_ANY;
	udpClient.sin_port = 0;

	if (bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient)) == SOCKET_ERROR) {
		cerr << "Bind error." << endl;
		return;
	}

	/** …make the packet as shown above **/

	/** set server end point (the broadcast address)**/
	udpServer.sin_family = AF_INET;
	udpServer.sin_addr.s_addr = inet_addr("192.168.2.255");
	udpServer.sin_port = htons(9);

	/** send the packet **/
	if (sendto(udpSocket, (const char*) &tosend, sizeof(unsigned char) * 102, 0, (struct sockaddr*)&udpServer, sizeof(udpServer)) == -1) {
		cerr << "Send to error." << endl;
	}
}

int main(char argc, char* argv[]) {
	if (argc == 2) {
		string macAddress = argv[1];
		if (createPacket(macAddress)) {
			// Initialize Winsock.
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
			if (iResult != NO_ERROR) {
				cerr << "Error at WSAStartup()" << endl;
			}
			sendMagicPacket();
		}
	}
	else {
		cerr << "Usage  : " << argv[0] << " <MAC ADDRESS>" << endl;
		cerr << "Example: " << argv[0] << " 6CF04977F0A4" << endl;
	}
}