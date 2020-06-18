# bluetooth-client-server

Bluetooth client & server chat application written in C for UNIX platform

### Prerequisites
```
- gcc compiler with C11 standard
- ncurses library installed (client side only)
- bluez library
- UNIX OS 
- bluetooth hardware support
```

### Description
```
- Bluetooth server app handling up to 10 connections with clients
- Uses AF_BLUETOOTH family for low level bluetooth access
- Server waits for connection from host on channel 1, then sends back new channel for permanent communication
- In client app, read & write operations are separated with help of ncurses, different windows
- Server gets messages and manages them (e.g retransmits)
```

### Compiling:

#### client app
```
gcc client.c -lpthread -lbluetooth -lncurses

```

#### server app
```
For server app you can use CMakeLists included in repository, type:

cmake CMakeLists.txt
make

This produces executable which can be run to start server, or simply type in shell:

gcc server.c -lpthread -lbluetooth
```

### Run

#### client app

```
Firstly, you will be asked to type your name (or nickname, which will be used in conversation), and after that, server address (e.g 00:00:00:00:00:00).
After establishing connection to server, on client's screen two windows are printed:
one for writing message, second one for reading. Messages from server are instantly displayed in reading window.
While writing, escape character is '\n' - this sends message. To quit application, simply type sequence: [\q] and hit enter.
This will lead to connection close and returning from app. 
```

#### server app

```
Running server app is very simple and does not require any additional arguments. Server prints to screen information about a new, permanent connection with client and clents messages.
To quit server app, you can type: 'q' and hit enter. After that, all client connections are closed, and then, program exits. 
```
