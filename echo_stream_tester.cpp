#include <cstring>
#include <future>
#include <iostream>
#include <string>

#include <nabto_client_api.h>

static int as_integer(nabto_status_t const status);
static size_t reader(nabto_stream_t stream);
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

size_t reader(nabto_stream_t stream) {
    nabto_status_t status;
    char* buffer;
    size_t len, readBytes = 0;

    for (;;) {
        status = nabtoStreamRead(stream, &buffer, &len);
        if (status == NABTO_INVALID_STREAM || status == NABTO_STREAM_CLOSED) {
            break;
        } else if (status != NABTO_OK) {
            std::cout << "Nabto read error: " << as_integer(status)
                      << std::endl;
            break;
        }
        readBytes += len;
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

    // Open a nabto stream
    status = nabtoStreamOpen(&stream, session, deviceId.c_str());
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " opening nabto stream: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }

    // Send the command 'echo\n' to the server. If the command is accepted '+\n'
    // will be returned, if not '-\n'.
    std::cout << "Sending echo command..." << std::endl;
    bool accepted = false;

    std::string cmd = "echo\n";
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

    std::cout << "Running echo test..." << std::endl;

    auto startEcho = std::chrono::high_resolution_clock::now();

    // Read data from server (separate thread)
    auto readerFuture = std::async(std::launch::async, reader, stream);

    // Write data to server
    size_t wroteBytes = 0;
    char chunk[2048];
    std::memset(chunk, 0, sizeof(chunk));
    while (dataSize > 0) {
        size_t toWrite = (dataSize < sizeof(chunk)) ? dataSize : sizeof(chunk);

        status = nabtoStreamWrite(stream, chunk, toWrite);
        if (status != NABTO_OK && status != NABTO_BUFFER_FULL) {
            std::cerr << "Error " << as_integer(status)
                      << " writing to nabto stream: " << nabtoStatusStr(status)
                      << std::endl;
            return 1;
        }

        if (status == NABTO_OK) {
            wroteBytes += toWrite;
            dataSize -= toWrite;
        }
    }

    // Close stream
    status = nabtoStreamClose(stream);
    if (status != NABTO_OK) {
        std::cerr << "Error " << as_integer(status)
                  << " closing nabto stream: " << nabtoStatusStr(status)
                  << std::endl;
        return 1;
    }

    // Wait for reader to finish
    size_t readBytes = readerFuture.get();

    auto durationEcho = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - startEcho)
                        .count();

    // Close session properly
    nabtoCloseSession(session);
    nabtoShutdown();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - start)
                        .count();

    // Print measurements
    std::cout << "Test results:" << std::endl;
    std::cout << "  wrote:                " << wroteBytes << " bytes" << std::endl;
    std::cout << "  read:                 " << readBytes << " bytes" << std::endl;
    std::cout << "  duration:             " << duration << " ms" << std::endl;
    std::cout << "  duration (echo only): " << durationEcho << " ms" << std::endl;

    // Exit code to shell (0 for ok)
    return 0;
}
