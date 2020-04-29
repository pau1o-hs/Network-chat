# Network-chat
A simple LAN network connection using sockets

## How to run
To open the server, run:
```
$ make run_server
```

To open the client, run:
```
$ make run_client
```

Note: If you desire to copy and paste a message greater than 4096 characters, run:
```
$ stty -icanon
$ make run_client
$ stty icanon
```

## Built with
* Visual Studio Code
* Ubuntu 18.04.1 LTS

### Authors
* Bruno dos Santos
* Paulo Henrique da Silva, NºUSP 10734515
