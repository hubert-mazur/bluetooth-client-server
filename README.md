# bluetooth-client-server

### Compiling:

#### client app
gcc client.c -lpthread -lbluetooth -lncurses

required bluez library, curses library installed

#### server app

gcc server.c -lpthread -lbluetooth

required bluez library installed