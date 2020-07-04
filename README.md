# Network-chat
An Internet Relay Chat network connection using sockets

## How to run
Use serveo.net to expose local servers to the internet through **SSH port forwarding**:
```
$ ssh -R 5400:localhost:5400 serveo.net
(Result) Forwarding TCP connections from serveousercontent.com:5400
```

To open the **server**, run:
```
$ make run_server
```

To open **clients**, run in N (1 <= N <= 30) terminals:
```
$ make run_client
```

To **connect** the client to the server through the SSH, is needed to convert the domain name to an IP Address:
```
In the browser:
https://www.hcidata.info/host2ip.cgi
Input: serveousercontent.com
Output: 159.89.214.31

In the client terminal process:
/connect 159.89.214.31:5400
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
