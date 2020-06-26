#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

#define MAX_CLIENTS 30
const string SERVER_WELCOME = "[Server] Welcome! Type [/help] to learn the server commands!\n";

// { Socket; { channelName; isAdmin } }
typedef struct clientProperties
{
	int socket;
	int isAdmin;

	string nickname;
	string channel;

	set<int> mute;

} Client;

Client clientSocket[30];
sockaddr_in client[MAX_CLIENTS];
char host[NI_MAXHOST];

void Commands(string& command, int clientIndex)
{	
	string response = "";

	if (command == "/help")
	{
		response += "[Server] Commands:\n";
		response += "[Server] /join #channelName\t- Join a channel\n";
		response += "[Server] /nickname username\t- Change Nickname\n";
		response += "[Server] /kick username\n";
		response += "[Server] /mute username\n";
		response += "[Server] /unmute username\n";
		response += "[Server] /whois username\n";
		response += "[Server] /ping\t- pong\n";
		response += "[Server] /quit\t- Exit program\n";
	}

	if (command.find("/join") != string::npos) 
	{
		string channel = command.substr(6);

		if (channel[0] != '#') 
		{
			response += "[Server] The first character must be '#'\n";
			goto END;
		}
		
		if (channel.find(' ') != string::npos || channel.find(',') != string::npos || channel.find(7) != string::npos)
		{
			response += "[Server] The channelName can't contain spaces, commas and CTRL + G\n";
			goto END;
		}
		
		if ((int) channel.size() > 200)
		{
			response += "[Server] The channelName max size is 200 characters\n";
			goto END;
		}

		clientSocket[clientIndex].channel = command.substr(6);
		response += "[Server] Joined " + clientSocket[clientIndex].channel + "\n";

		int channelLen = 0;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (i != clientIndex && clientSocket[i].channel == channel)
			{
				clientSocket[clientIndex].isAdmin = 0;
				channelLen++;
			}
		}

		if (!channelLen) response += "You're the Administrator\n", clientSocket[clientIndex].isAdmin = 1;
		else response += "There is more " + to_string(channelLen) + " client(s) in this channel\n";
	}

	if (command.find("/nickname") != string::npos) 
	{
		string nickname = command.substr(10);

		if ((int) nickname.size() > 50)
		{
			response += "[Server] The nickname max size is 50 characters\n";
			goto END;
		}

		response += "[Server] Nickname changed\n";
		clientSocket[clientIndex].nickname = nickname;
	}

	if (command.find("/kick") != string::npos) 
	{
		if (!clientSocket[clientIndex].isAdmin)
		{
			response += "[Server] You aren't the administrator of this channel\n";
			goto END;
		}

		string nickname = command.substr(6);

		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (clientSocket[i].nickname == nickname && clientSocket[i].channel == clientSocket[clientIndex].channel)
			{
				clientSocket[i].channel = "";
				break;
			}

			if (i + 1 == MAX_CLIENTS)
			{
				response += "[Server] Nickname not found\n";
				goto END;
			}
		}

		response += "[Server] " + nickname + " kicked\n";
		send(clientSocket[i].socket, response.c_str(), (int) response.size(), 0);
	}

	if (command.find("/mute") != string::npos) 
	{
		string nickname = command.substr(6);

		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (clientSocket[i].nickname == nickname)
			{
				clientSocket[clientIndex].mute.insert(clientSocket[i].socket);
				break;
			}

			if (i + 1 == MAX_CLIENTS)
			{
				response += "[Server] Nickname not found\n";
				goto END;
			}
		}
		
		response += "[Server] " + nickname + " muted\n";
	}

	if (command.find("/unmute") != string::npos) 
	{
		string nickname = command.substr(8);

		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (clientSocket[i].nickname == nickname)
			{
				clientSocket[clientIndex].mute.erase(clientSocket[i].socket);
				break;
			}

			if (i + 1 == MAX_CLIENTS)
			{
				response += "[Server] Nickname not found\n";
				goto END;
			}
		}
		
		response += "[Server] " + nickname + " unmuted\n";
	}

	if (command.find("/whois") != string::npos) 
	{
		string nickname = command.substr(7);

		if (!clientSocket[clientIndex].isAdmin)
		{
			response += "[Server] You aren't the administrator of this channel\n";
			goto END;
		}

		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (clientSocket[i].nickname == nickname)
			{
				clientSocket[clientIndex].mute.erase(clientSocket[i].socket);
				break;
			}

			if (i + 1 == MAX_CLIENTS)
			{
				response += "[Server] Nickname not found\n";
				goto END;
			}
		}

		inet_ntop(AF_INET, &client[i].sin_addr, host, NI_MAXHOST);
		string ip(host);

		response += "IP: " + ip + " connected on " + to_string(ntohs(client[i].sin_port)) + "\n";
	}

	if (command == "/ping") 
	{
		response += "[Server] pong\n";
	}

	if (response == "")
	{
		response += "[Server] Command not found\n";
		response += "[Server] Type [/help] to learn the server commands\n";
	}

	END:
		send(clientSocket[clientIndex].socket, response.c_str(), (int) response.size(), 0);
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
	socklen_t clientSize = sizeof(client[0]);
	char svc[NI_MAXSERV];
	int connClients = 0;

	// memset(clientSocket, 0, sizeof(clientSocket) * MAX_CLIENTS);

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
			sd = clientSocket[i].socket;   
				
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
			clientSocket[connClients].socket = accept(masterSocket, (sockaddr *)&client[connClients], &clientSize);

			if (clientSocket[connClients].socket == -1)
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

				clientSocket[connClients].nickname = "User " + to_string(connClients / 10) + to_string(connClients % 10);

				send(clientSocket[connClients].socket, SERVER_WELCOME.c_str(), (int) SERVER_WELCOME.size(), 0);
				connClients++;
			}
		}

		// Clear the buffer
		memset(buf, 0, 4096);

		//else its some IO operation on some other socket
		for (int i = 0; i < connClients; i++)
		{
			sd = clientSocket[i].socket;

			if (FD_ISSET(sd, &readfds))   
            {
				if ((valread = read(sd, buf, 4096)) == 0)   
                {  
					//Somebody disconnected , get his details and print  
					getpeername(sd, (struct sockaddr*)&hint, (socklen_t*)&addrlen);   
					cout << inet_ntoa(hint.sin_addr) << " disconnected" << endl;   
						
					//Close the socket and mark as 0 in list for reuse  
					close(sd);   
					clientSocket[i].socket = 0;
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
						Commands(message, i);
						continue;
					}

					// Broadcast message
					for (int j = 0; j < connClients; j++)
					{
						if (clientSocket[j].mute.count(clientSocket[i].socket))
							continue;

						for (int k = 0; k <= (int) (message.size()) / 4097; k++)
						{
							if (clientSocket[j].channel == clientSocket[i].channel)
							{
								send(clientSocket[j].socket, clientSocket[i].nickname.c_str(), 51, 0);
								send(clientSocket[j].socket, message.c_str() + (4096 * k), 4096, 0);
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
		close(clientSocket[i].socket);
	}

	return 0;
}