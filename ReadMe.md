# 4xSimulation

Build with Cmake:
```sh

Windows:
mkdir build
cd build
cmake ..\ -G "Visual Studio 14 Win64"
Start 4xSimulation.sln and build
Linux / Mac:
mkdir build
cd build
cmake ../
make
cd src
./4xsim
```
### Multiplayer (requires python3)
python binary_network_server.py

The server will bind to all network adapters, so you should be able to connect to 127.0.0.1 port 4000 for local play, or use a local area network address with port 4000.

### Replay
```sh
./4xsim < replay_file
```

Each run of ./4xsim outputs a summary of the game to last_run.

* You cannot use last_run as your input file; copy last_run before playing it back.

### Resume from replay
If you prefer to remain in the terminal after loading a replay:

```sh
cat replay_file - | ./4xsim
```

### Fuzz testing
```sh
python tools/fuzz.py | ./4xsim
```
