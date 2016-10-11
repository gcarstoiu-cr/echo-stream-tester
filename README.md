# Echo Stream Tester

Simple nabto client application to test echo streams using the nabto client api.

## Code checkout

Clone the github repository

```bash
git clone https://github.com/nabto/echo-stream-tester.git
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

## Add the Nabto Client SDK Resources

Download the *Nabto Client SDK Resources* from [developer.nabto.com](https://developer.nabto.com/) and unpack them to get the following folder structure.

```
echo-stream-tester/
├── CMakeLists.txt
├── echo_stream_tester.cpp
├── include
│   └── nabto_client_api.h
├── lib
│   └── libnabto_client_api.so
├── README.md
└── share
    └── nabto
        ├── client_api
        │   └── doc
        │       ├── COPYRIGHT.txt
        │       ├── DISCLAIMER.txt
        │       └── FILES.txt
        └── roots
            ├── cacert.pem
            └── ca.crt

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

Run the application with the first parameter beeing the device id of the server and the second parameter the size of the data to send/receive in bytes.

Example:

```bash
./echo_stream_tester echo.u.nabto.net 1000000
```
