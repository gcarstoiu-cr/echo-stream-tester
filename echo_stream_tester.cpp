#include <cstring>
#include <future>
#include <iostream>
#include <string>

#include <nabto_client_api.h>

// If enabled, it will test the streaming duration. If disabled, it will
// test only the connection establishment duration.
#define REMOTE_STREAM_ENABLE    (1)

static int as_integer(nabto_status_t const status);
static size_t reader(nabto_stream_t stream, size_t dataSize);
static int testRun(std::string deviceId, size_t dataSize);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "usage: echo_stream_tester <device id> <data size>"
                  << std::endl;
        return 1;
    }

    std::string deviceId = argv[1];
    size_t dataSize = std::stoul(argv[2]);

    return testRun(deviceId, dataSize);
}

int as_integer(nabto_status_t const status) {
    return static_cast<std::underlying_type<nabto_status_t>::type>(status);
}

size_t reader(nabto_stream_t stream, size_t dataSize)
{
    nabto_status_t status;
    char* buffer;
    size_t len, readBytes = 0;

    for (;;)
    {
        status = nabtoStreamRead(stream, &buffer, &len);
        if (status == NABTO_INVALID_STREAM || status == NABTO_STREAM_CLOSED)
        {
            std::cout << "-->Stream closed by remote device with status: "
                      << nabtoStatusStr(status) << std::endl;
            break;
        }
        else if (status != NABTO_OK)
        {
            std::cout << "Nabto read error: " << as_integer(status)
                      << std::endl;
            break;
        }
        readBytes += len;
        if (readBytes >= dataSize)
        {
            break;
        }
        nabtoFree(buffer);
    }
    return readBytes;
}

int testRun(std::string deviceId, size_t dataSize) {
    auto start = std::chrono::high_resolution_clock::now();
    nabto_stream_t stream;
    nabto_handle_t session;
    nabto_status_t status;

    // Initialize the nabto client library
    status = nabtoStartup(NULL);
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " initializing nabto client api: "
                  << nabtoStatusStr(status) << std::endl;
        return 1;
    }

    // Open a nabto session using the guest login
    status = nabtoOpenSession(&session, "guest", "");
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " creating nabto session: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }

    auto streamOpened = std::chrono::high_resolution_clock::now();
    // Open a nabto stream
    status = nabtoStreamOpen(&stream, session, deviceId.c_str());
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " opening nabto stream: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }

#if REMOTE_STREAM_ENABLE
    // Send the command '<dataSize>\n' to the server. If the command is accepted '+\n'
    // will be returned, if not '-\n'.
    std::cout << "Sending echo command..." << std::endl;
    bool accepted = false;

    std::string cmd = std::to_string(dataSize);
    status = nabtoStreamWrite(stream, cmd.c_str(), cmd.size());
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " writing to nabto stream: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }

    for (;;) {
        // receive '+' or '-'
        char buffer[1];
        size_t resultLength;
        status = nabtoStreamReadIntoBuf(stream, buffer, 1, &resultLength);
        if (status != NABTO_OK) {
            std::cerr << "Error " << as_integer(status)
                      << " reading from nabto stream: "
                      << nabtoStatusStr(status) << std::endl;
            return 1;
        }

        if (resultLength > 0) {
            if (buffer[0] == '+') {
                accepted = true;
            } else if (buffer[0] == '-') {
                accepted = false;
            } else {
                std::cout << "Received something different from '+' or '-'."
                          << std::endl;
                return 1;
            }
            break;
        }
    }

    for (;;) {
        // receive '\n'
        char buffer[1];
        size_t resultLength;
        status = nabtoStreamReadIntoBuf(stream, buffer, 1, &resultLength);
        if (status != NABTO_OK) {
            std::cerr << "Error " << as_integer(status)
                      << " reading from nabto stream: "
                      << nabtoStatusStr(status) << std::endl;
            return 1;
        }
        if (buffer[0] == '\n') {
            break;
        }
    }

    if (accepted) {
        std::cout << "Command accepted." << std::endl;
    } else {
        std::cout << "Command not accepted." << std::endl;
        return 1;
    }

    nabto_connection_type_t connectionType;
    status = nabtoStreamConnectionType(stream, &connectionType);
    if (status != NABTO_OK) {
            std::cerr << "Error " << as_integer(status)
                      << " reading from nabto stream: "
                      << nabtoStatusStr(status) << std::endl;
            return 1;
    }

    std::cout << "Connection type with remote device: ";
    switch (connectionType)
    {
        case NCT_LOCAL:
            std::cout << "[Local network]" << std::endl;
            break;

        case NCT_P2P:
            std::cout << "[P2P]" << std::endl;
            break;

        case NCT_RELAY:
            std::cout << "[Relay]" << std::endl;
            break;

        default:
            std::cout << "other (" << connectionType << ")" << std::endl;
            break;
    }

    std::cout << "Running echo test..." << std::endl;

    auto startEcho = std::chrono::high_resolution_clock::now();

    // Read data from server (separate thread)
    auto readerFuture = std::async(std::launch::async, reader, stream, dataSize);

    // Wait for reader to finish
    size_t readBytes = readerFuture.get();

    auto durationStream = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - startEcho)
                        .count();
#endif

    // Close stream
    status = nabtoStreamClose(stream);
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " closing nabto stream: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }
    auto durationConnection = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::high_resolution_clock::now() - streamOpened)
                              .count();


    // Close session properly
    nabtoCloseSession(session);
    nabtoShutdown();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - start)
                        .count();
    auto throughputKByte = (readBytes * 1000.0 / durationStream) / 1024;
    auto throughputMbit = throughputKByte * 8 / 1024;

    // Print measurements
    std::cout << "Test results:" << std::endl;
    // std::cout << "  wrote:                " << wroteBytes << " bytes" << std::endl;
    std::cout << "  duration (total):       " << duration << " ms" << std::endl;
    std::cout << "  duration (connection):  " << durationConnection << " ms" << std::endl;
#if REMOTE_STREAM_ENABLE
    std::cout << "  duration (stream only): " << durationStream << " ms" << std::endl;
    std::cout << "  read: " << readBytes << " bytes - throughput: " << throughputKByte << " kBps / "
              << throughputMbit << " Mbps" << std::endl;
#endif
    // Exit code to shell (0 for ok)
    return 0;
}
