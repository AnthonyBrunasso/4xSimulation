# 4xSimulation

Prebuild steps:

```sh
cd tools
python serialization.py
```

Build with Cmake:
```sh
From root directory, make a build directory to store all build related artifacts.
mkdir build
cd build
If on windows:
cmake ..\ -G "Visual Studio 14 Win64"
Start 4xSimulation.sln and build
If not on windows:
cmake ../
make
cd src
./4xSimulation
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
