# Network-chat
A simple LAN network connection using sockets

## How to run
To open the server, run:
```
$ make run_server
```

To open clients, run in N (1<= N <= 30) terminals:
```
$ make run_client
```

Note: If you desire to copy and paste a message greater than 4096 characters, run:
```
$ stty -icanon
$ make run_client
$ stty icanon
```

## Preview
![Image description](https://github.com/pau1o-hs/Network-chat/blob/master/src/img_preview.png)

## Built with
* Visual Studio Code
* Ubuntu 18.04.1 LTS (WSL)

## Authors
* **Bruno dos Santos**, NºUSP 10786170
* **Paulo Henrique da Silva**, NºUSP 10734515
