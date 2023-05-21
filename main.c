#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;

int main() {

    // fd - file descriptor
    // A file descriptor is an integer value that uniquely identifies an open file or a communication endpoint (socket) within a process.
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    /**
     * socket(domain, type, protocol)
     *
     * domain, or address family — communication domain in which the socket should be created. Some of address families are AF_INET (IPv4), AF_INET6 (IPv6), AF_UNIX (local channel, similar to pipes), AF_ISO (ISO protocols), and AF_NS (Xerox Network Systems protocols).
     * type — type of service. This is selected according to the properties required by the application: SOCK_STREAM (virtual circuit service), SOCK_DGRAM (datagram service), SOCK_RAW (direct IP service). Check with your address family to see whether a particular service is available.
     * I — protocol to be used with the socket. Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as 0.
     *
     */
    while ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    /* htonl converts a long integer (e.g. address) to a network representation */
    /* htons converts a short integer (e.g. port) to a network representation */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    printf("Socket created\n");

    /**
     * (struct sockaddr *)&address - cast to struct sockaddr *
     * Since the address structure may differ based on the type of transport used, the third parameter specifies the length of that structure
     */
    if ((bind(server_fd, (struct sockaddr *) &address, sizeof(address))) < 0) {
        printf("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    /**
     * Before a client can connect to a server, the server should have a socket that is prepared to accept the connections.
     * The listen system call tells a socket that it should be capable of accepting incoming connections
     *
     * The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
     */
    if (listen(server_fd, 3) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while(1) {

        /**
         * The accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
         * The original socket that was set up for listening is used only for accepting connections, not for exchanging data.
         * By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
         */
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        char buffer[30000] = {0};
        int valread = read(new_socket, buffer, 30000);
        if (valread < 0) {
            printf("No bytes are there to read");
        }
        printf("%s\n", buffer);

        if (strstr(buffer, "OPTIONS /") != NULL) {
            // Send CORS headers for preflight request
            char *response_headers = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\nAccess-Control-Max-Age: 86400\nAccess-Control-Allow-Headers: Content-Type\nContent-Length: 0\n\n";
            write(new_socket, response_headers, strlen(response_headers));
        } else if (strstr(buffer, "POST /")) {
            // Send an HTTP response
            char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\n\n Hello from server";
            write(new_socket, response, strlen(response));
        } else if (strstr(buffer, "GET /")) {
            char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\n\n Hello from server";
            write(new_socket, response, strlen(response));
        } else {
            char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\n";
            write(new_socket, response, strlen(response));
        }

        printf("Response sent\n");

        close(new_socket);
    }

    return 0;
}