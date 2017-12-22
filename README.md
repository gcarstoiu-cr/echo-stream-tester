# Read Stream Tester

Simple nabto client application to test reading a stream using the nabto client api.
It will display total, connection and stream durations and also data throughput.

Note: you will need to run the device software on a CC3220 board, using code and instructions from: https://github.com/gcarstoiu-cr/unabto-cc3200.git branch cc3220-port.

## Code checkout

Clone the github repository

```bash
git clone https://github.com/gcarstoiu-cr/echo-stream-tester.git
```

## Add the Nabto Client SDK Libraries

Download the *Nabto Client SDK Libraries* from [developer.nabto.com](https://developer.nabto.com/) and unpack them to get the following folder structure.

```
echo-stream-tester/
├── CMakeLists.txt
├── echo_stream_tester.cpp
├── include
│   └── nabto_client_api.h
├── lib
│   └── libnabto_client_api.so
└── README.md
```


## Build the code

In the `echo-stream-tester` directory run

```bash
mkdir build
cd build
cmake ..
make
```

## Run the application

Run the application with the first parameter beeing the device id of the server and the second parameter the size of the data to receive in bytes.

Example:

```bash
./echo_stream_tester echo.u.nabto.net 1000000

```

Result:
```
Sending echo command...
Command accepted.
Connection type with remote device: other (3)
Running echo test...
Test results:
  duration (total):       2420 ms
  duration (connection):  2403 ms
  duration (stream only): 2009 ms
  read: 1048727 bytes - throughput: 509.78 kBps / 3.98265 Mbps
```
