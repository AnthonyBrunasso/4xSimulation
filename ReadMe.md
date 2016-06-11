# 4xSimulation

Prebuild steps:

cd tools
python type_generator.py

Build with:

Make

### Multiplayer
source host_server.sh

Clients can use netcat to connect to the with with any of the following to input commands:
* nc 127.0.0.1 1200
* nc 127.0.0.1 1201
* nc 127.0.0.1 1202
* nc 127.0.0.1 1203

Netcat to the game history port, and pipe the data to the ./4xsim executable:
* nc 127.0.0.1 4000 | ./4xsim
* nc 127.0.0.1 4001 | ./4xsim
* nc 127.0.0.1 4002 | ./4xsim
* nc 127.0.0.1 4003 | ./4xsim

Each listen socket is only valid for one connection. See the shell script for how to restart a new listen socket.

### Replay
./4xsim < replay_file

Each run of ./4xsim outputs a summary of the game to last_run.

* You cannot use last_run as your input file, so copy it before playing it back.

### Resume from replay
If you prefer to remain in the terminal after loading a replay:

cat replay_file - | ./4xsim
