#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

void getCommand(string& command)
{
	cout << endl;
	
	if (command == "/help")
	{
		cout << "[Commands]" << endl;
		cout << "/quit\t" << "Exit program" << endl;
		return;
	}

	if (command == "/quit") 
	{
		exit(0);
	}

	cout << "Command not found" << endl;
	cout << "Type [/help] to learn the server commands" << endl;
	return;
}

int main()
{
	// Create a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return 1;
	}

	// Create a hint structure for the server connect with
	int port = 54000;
	string ipAddress;

	cout << "Type server IPv4: ";
	getline(cin, ipAddress);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server host socket
	int connectRes = connect(sock, (sockaddr*) &hint, sizeof(hint));
	if (connectRes == -1)
		return cerr << "Couldn't connect to the server!" << endl, -1;
	else cout << "Joined the server!" << endl;

	cout << "Type [/help] to learn the server commands" << endl;

	// While loop
	char buf[4096];
	string userInput;

	do {
		// Enter line of text
		cout << "> ";
		getline(cin, userInput);

		if (userInput[0] == '/') 
		{
			getCommand(userInput);
			continue;
		}

		// Send to server
		int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
		if (sendRes == -1)
		{
			cout << "Could not send to server! Whoops!\r\n";
			continue;
		}

		// Wait for server response
		memset(buf, 0, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);

		// Display server response
		cout << "[Server received] " << string(buf, bytesReceived) << "\r\n";
	} while (true);
	
	// Close the socket
	close(sock);

	return 0;
}