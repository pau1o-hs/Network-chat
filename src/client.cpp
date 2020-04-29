#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <limits.h>

using namespace std;

void getCommand(string& command)
{
	cout << endl;
	
	if (command == "/help")
	{
		cout << "[Commands]" << endl;
		cout << "/quit\t" << "Exit program" << endl;
		cout << endl;
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
	int port = 5400;
	string ipAddress = "127.0.0.1";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server host socket
	int connectRes = connect(sock, (sockaddr*) &hint, sizeof(hint));
	if (connectRes == -1)
		return cerr << "Couldn't connect to the server!" << endl, -1;
	else cout << "Joined the server!" << endl;

	cout << "Type [/help] to learn the server commands" << endl << endl;

	// FD_SET Stuff
	int activity, bytesReceived;
	fd_set readfds;

	// While loop
	char buf[4096];
	string userInput;

	while (true) 
	{
		cout << "> ";
		fflush(stdout);

		/* Watch stdin (fd 0) to see when it has input. */
		FD_ZERO(&readfds);

		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(sock, &readfds);

		activity = select(sock + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno!=EINTR))   
		{   
			cout << "select error" << endl;
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			// Enter line of text
			getline(cin, userInput);

			if (userInput[0] == '/') 
			{
				getCommand(userInput);
				continue;
			}

			for (int i = 0; i <= (int) (userInput.size()) / 4097; i++)
			{
				// Send to server
				int sendRes = send(sock, userInput.c_str() + (4096 * i), sizeof(char) * 4096, 0);
				if (sendRes == -1)
				{
					cout << "Could not send to server! Whoops!\r\n";
					continue;
				}
			}
		}

		// Server response
		memset(buf, 0, 4096);

		if (FD_ISSET(sock, &readfds))
		{
			if ((bytesReceived = read(sock, buf, 4096)) == 0)
			{
				cout << endl << "[Server closed]" << endl;
				exit(0);
			}

			buf[bytesReceived] = '\0';

			// Display server response
			cout << "\r" << "[Server Broadcast] " << buf << endl;
			// cout << "> ";
		}
	}
	
	// Close the socket
	close(sock);

	return 0;
}