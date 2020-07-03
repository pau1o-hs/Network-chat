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
#include <signal.h>

using namespace std;

char MACRO_RECEIVED[20] = "Received\0";

int main()
{
	// IGNORE CTRL+C
	signal(SIGINT, SIG_IGN);

	// Create a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return 1;
	}

	// Create a hint structure for the server connect with
	int port;
	string ipAddress = "";

	cout << "[Local] Type [/connect ip:5400] to establish a connection to the server" << endl;
	string str = "";
	while (str.find("/connect ") == string::npos) getline(cin, str);

	int found = str.find_last_of(":");

	ipAddress = str.substr(9, found - 9);
	port = stoi(str.substr(found+1));

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server host socket
	int connectRes = connect(sock, (sockaddr*) &hint, sizeof(hint));
	if (connectRes == -1)
	{
		if (connectRes == -1) cerr << "Couldn't connect to the server!" << endl;
		exit(0);
	}

	// FD_SET Stuff
	int activity, bytesReceived;
	fd_set readfds;

	// While loop
	char buf[4096];
	string userInput;

	// GET SERVER WELCOME
	FD_ISSET(sock, &readfds);

	bytesReceived = read(sock, buf, 4096);
	buf[bytesReceived] = '\0';

	cout << buf << endl;

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

			if (userInput == "/quit") goto DISCONNECT;

			for (int i = 0; i <= (int) (userInput.size()) / 4097; i++)
			{
				// Send to server
				int sendRes = send(sock, userInput.c_str() + (4096 * i), 4096, 0);
				if (sendRes == -1)
				{
					cout << "Could not send to server! Whoops!\r\n";
					continue;
				}
			}

			// RECEIVE COMMAND REQUEST
			if (userInput[0] == '/')
			{
				FD_ISSET(sock, &readfds);

				bytesReceived = read(sock, buf, 4096);
				buf[bytesReceived] = '\0';

				cout << buf << endl;
				continue;
			}
		}

		// Server response
		memset(buf, 0, 4096);

		if (FD_ISSET(sock, &readfds))
		{
			if ((bytesReceived = read(sock, buf, 51)) == 0)
			{
				cout << endl << "[Local] Server closed" << endl;
				exit(0);
			}

			// Display server response
			// Username
			buf[bytesReceived] = '\0';

			cout << "\r" << '[' << buf << "] ";

			// Message
			FD_ISSET(sock, &readfds);

			bytesReceived = read(sock, buf, 4096);
			buf[bytesReceived] = '\0';

			cout << buf << endl;
		}
	}
		
DISCONNECT:
	// Close the socket
	close(sock);
	cout << "[Local] Goodbye\n";

	return 0;
}