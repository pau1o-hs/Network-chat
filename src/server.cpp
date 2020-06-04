#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

#define MAX_CLIENTS 30
const string SERVER_WELCOME = "[Server] Welcome! Type [/help] to learn the server commands!\n";

void Commands(string& command, int clientSocket)
{	
	string response = "";
	if (command == "/help")
	{
		response += "[Server] Commands:\n";
		response += "[Server] /ping\tpong\n";
		response += "[Server] /quit\tExit program\n";
	}

	if (command == "/ping") 
	{
		response += "[Server] pong";
	}

	if (response == "")
	{
		response += "[Server] Command not found\n";
		response += "[Server] Type [/help] to learn the server commands\n";
	}

	send(clientSocket, response.c_str(), (int) response.size(), 0);
	return;
}

int main()
{
	// Create a socket
	int masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (masterSocket == -1)
	{
		cerr << "Can't create a socket!";
		return -1;
	}

	// Bind the socket to IP/port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(5400);
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // pton: pointer to number

	if (bind(masterSocket, (sockaddr *)&hint, sizeof(hint)) == -1)
	{
		cerr << "Can't bind to IP/port";
		return -2;
	}

	// Mark the socket for listening in
	if (listen(masterSocket, SOMAXCONN) == -1)
	{
		cerr << "Can't listen!";
		return -3;
	}

	// Clients
	sockaddr_in client[MAX_CLIENTS];
	int connClients = 0;
	socklen_t clientSize = sizeof(client[0]);
	char host[NI_MAXHOST];
	char svc[NI_MAXSERV];
	int clientSocket[MAX_CLIENTS];

	memset(clientSocket, 0, sizeof(clientSocket) * MAX_CLIENTS);

	//set of socket descriptors  
	int activity, sd, max_sd, valread;
	int addrlen = sizeof(hint);
    	fd_set readfds;  

	// While receiving: display message, echo message
	char buf[4096];
	
	while (true)
	{
		//clear the socket set  
		FD_ZERO(&readfds);   
	
		//add master socket to set  
		FD_SET(masterSocket, &readfds);   
		max_sd = masterSocket;   
		
		//add child sockets to set  
		for (int i = 0; i < MAX_CLIENTS; i++)   
		{   
			//socket descriptor  
			sd = clientSocket[i];   
				
			//if valid socket descriptor then add to read list  
			if(sd > 0)   
				FD_SET(sd, &readfds);   
				
			//highest file descriptor number, need it for the select function  
			if(sd > max_sd)   
				max_sd = sd; 
		} 

		//wait for an activity on one of the sockets, timeout is NULL,  
		//so wait indefinitely  
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); 

		if ((activity < 0) && (errno!=EINTR))   
		{   
			printf("select error");   
		}

		//If something happened on the master socket ,  
		//then its an incoming connection  
		if (FD_ISSET(masterSocket, &readfds))   
		{ 
			// Accept a call
			clientSocket[connClients] = accept(masterSocket, (sockaddr *)&client[connClients], &clientSize);

			if (clientSocket[connClients] == -1)
			{
				cerr << "Problem with client[connClients] connecting!";
				// return -4;
			}
			else
			{
				memset(host, 0, NI_MAXHOST);
				memset(svc, 0, NI_MAXHOST);

				int result = getnameinfo((sockaddr *)&client[connClients], sizeof(client[connClients]), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

				if (result)
				{
					cout << host << " connected on " << svc << endl;
				}
				else
				{
					inet_ntop(AF_INET, &client[connClients].sin_addr, host, NI_MAXHOST);
					cout << host << " connected on " << ntohs(client[connClients].sin_port) << endl;
				}

				send(clientSocket[connClients], SERVER_WELCOME.c_str(), (int) SERVER_WELCOME.size(), 0);
				connClients++;
			}
		}

		// Clear the buffer
		memset(buf, 0, 4096);

		//else its some IO operation on some other socket
		for (int i = 0; i < connClients; i++)
		{
			sd = clientSocket[i];

			if (FD_ISSET(sd, &readfds))   
            {
				if ((valread = read(sd, buf, 4096)) == 0)   
                {  
					//Somebody disconnected , get his details and print  
					getpeername(sd, (struct sockaddr*)&hint, (socklen_t*)&addrlen);   
					cout << inet_ntoa(hint.sin_addr) << " disconnected" << endl;   
						
					//Close the socket and mark as 0 in list for reuse  
					close(sd);   
					clientSocket[i] = 0;   

				}
				//Echo back the message that came in  
				else 
				{
					//set the string terminating NULL byte on the end  
					//of the data read  
					buf[valread] = '\0';
					string message(buf);

					cout << "Received: " << buf << endl;

					if (message[0] == '/')
					{
						Commands(message, clientSocket[i]);
						continue;
					}

					// Broadcast message
					char userID[8] = "User XX";
					userID[5] = (i / 10) + '0';
					userID[6] = (i % 10) + '0';
					userID[7] = '\0';

					for (int j = 0; j < connClients; j++)
					{
						// if (i == j) continue;
						int attempt = 0;

						for (int k = 0; k <= (int) (message.size()) / 4097; k++)
						{
							send(clientSocket[j], userID, 8, 0);
							send(clientSocket[j], message.c_str() + (4096 * k), 4096, 0);

							if (!FD_ISSET(sd, &readfds)) 
							{
								k--;
								attempt++;

								if (attempt == 5)
								{
									close(clientSocket[j]);
									clientSocket[j] = 0;
								}
							}
							else {
								attempt = 0;
								
								valread = read(sd, buf, 20);
								buf[valread] = '\0';
								cout << "Client " << clientSocket[j] << ' ' << buf << endl;
							}
						}
					}
				}
			}
		}
	}

	// Close the listening socket
	close(masterSocket);

	// Close socket
	for (int i = 0; i < connClients; i++)
	{
		close(clientSocket[i]);
	}

	return 0;
}