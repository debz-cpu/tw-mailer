#include <iostream>   // std::cout, std::cerr
#include <string>     // std::string, std::stoi
#include <cstdlib>    // EXIT_SUCCESS, EXIT_FAILURE
#include <cstring>        // memset
#include <netinet/in.h>   // sockaddr_in, htons
#include <arpa/inet.h>    // inet_pton

int main(int argc, char *argv[])
{
    // ===== ✏️ YOURS – Step C1: validate arguments =====

    // Expected: ./twmailer-client <ip> <port>
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return EXIT_FAILURE;
    }

    // argv[1] is the server's IP address as text, e.g. "127.0.0.1"
    std::string ip = argv[1];

    // argv[2] is the port as text; convert it to a number
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (const std::exception &) {
        std::cerr << "Error: port must be a number\n";
        return EXIT_FAILURE;
    }
    if (port < 1 || port > 65535) {
        std::cerr << "Error: port must be between 1 and 65535\n";
        return EXIT_FAILURE;
    }

       // ===== 🔒 FIXED – Step C2: build the server address =====

    // Describe the server: IPv4, its port (network byte order)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));   // clear leftover memory
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert the IP text ("127.0.0.1") into the 4-byte binary form.
    // inet_pton returns 1 if it worked, 0 if the text is not a valid IPv4 address.
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1) {
        std::cerr << "Error: invalid IP address: " << ip << "\n";
        return EXIT_FAILURE;
    }

    // Temporary: show that the address is valid
    std::cout << "Valid address: " << ip << ":" << port << "\n";

    return EXIT_SUCCESS;
}