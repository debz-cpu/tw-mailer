/*
 * twmailer-server
 * Usage: ./twmailer-server <port> <mail-spool-directoryname>
 *
 * Current state: validates the arguments and sets up a listening TCP socket.
 */

// ===== Includes =====
#include <iostream>       // std::cout, std::cerr  (C++ output)
#include <string>         // std::string, std::stoi
#include <cstdlib>        // EXIT_SUCCESS, EXIT_FAILURE
#include <cstdio>         // perror  (prints the system's error reason)
#include <cstring>        // memset  (fills memory with a value)
#include <sys/socket.h>   // socket, setsockopt, bind, listen
#include <netinet/in.h>   // sockaddr_in, htons, INADDR_ANY
#include <unistd.h>       // close

int main(int argc, char *argv[])
{
    // ===== ✏️ YOURS – Step 1: validate the command-line arguments =====

    // argc = number of words on the command line (program name included).
    // We need exactly 3: program name, port, spool directory.
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <mail-spool-directoryname>\n";
        return EXIT_FAILURE;
    }

    // argv[1] is text ("6543"). std::stoi converts it to a number.
    // If the text is not a number ("abc"), std::stoi throws an exception,
    // which we catch to print a clear error instead of crashing.
    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::exception &) {
        std::cerr << "Error: port must be a number\n";
        return EXIT_FAILURE;
    }

    // A port is a 16-bit number, so only 1–65535 are valid.
    if (port < 1 || port > 65535) {
        std::cerr << "Error: port must be between 1 and 65535\n";
        return EXIT_FAILURE;
    }

    // Folder where the mails will be stored (used later for SEND/LIST/READ/DEL).
    std::string spoolDir = argv[2];

    // ===== 🔒 FIXED – Step 2: create the socket =====

    // Ask the Linux kernel for a network endpoint.
    // AF_INET     = IPv4
    // SOCK_STREAM = TCP (reliable, ordered byte stream)
    // Returns a file descriptor (a small number, e.g. 3) or -1 on error.
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");   // prints e.g. "socket: Permission denied"
        return EXIT_FAILURE;
    }

    // ===== 🔒 FIXED – Step 3: socket option + bind to the port =====

    // After the server stops, Linux blocks the port for ~1 minute.
    // SO_REUSEADDR lets us restart the server immediately on the same port.
    int reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(listen_fd);   // clean up the socket we already opened
        return EXIT_FAILURE;
    }

    // Describe the address we want to listen on.
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));   // clear random leftover memory
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // accept on every network interface
    address.sin_port = htons(port);         // port in network byte order (big-endian)

    // Attach the socket to that address ("put the house number on the door").
    // bind() is generic, so the IPv4 struct is cast to the general sockaddr type.
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");     // e.g. "bind: Address already in use"
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // ===== 🔒 FIXED – Step 4: start listening =====

    // Switch the socket into server mode.
    // The 5 (backlog) = how many clients may wait in the queue
    // while the server is busy with another client.
    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }
    std::cout << "Server listening on port " << port << "\n";

    // ===== Cleanup =====
    // Temporary: in Step 5 the accept loop goes here, before closing.
    close(listen_fd);

    return EXIT_SUCCESS;
}