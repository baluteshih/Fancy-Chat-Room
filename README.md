# Fancy-Chat-Room
Project Phase 2 for Computer Network (NTU CSIE 3510, Fall 2021)

## Profile
* Team 49
	* B08902103: 蔡旻諺
	* B08705029: 簡謙益

## Packages Required
* [SQLite](https://www.sqlite.org/index.html) version 3.37.2

## How to run our codes?
To build the server, run
```
$ make server
```
To build the client, run
```
$ make client
```

### Run the server
Run
```
$ ./server <port_number>
```
, where `<port_number>` is the numbering of the port where you would like to put your server.

### Console Mode
Run
```
$ ./client <serverip:port>
```
, where `<serverip:port>` is the common format of IP and port where you run your server on. **Note that the client will fail after sending the first message if the IP and port aren't valid.**
To conceal the error message, run
```
$ ./client <serverip:port> 2>/dev/null
```

The Console Mode supports very few functions. To use more, try our Web Mode. It would be more intuitive and convenient.

### Web Mode
Directly use the browser to connect to the IP and port where you run your server.

We recommend using Mozilla Firefox to connect to our server. More precisely, our standard testing User-Agent is:
```
Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0
```

Our Web Mode supports the following feature:
- basic account system with a secure password database
- change-able password
- basic friend system
- basic chat system
- multiple user chatroom

Enjoy!

