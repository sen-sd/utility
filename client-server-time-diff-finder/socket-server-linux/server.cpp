// To compile $ sudo g++ -o server server.cpp -pthread

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <thread>
#include <sys/time.h> // Include this header for gettimeofday

#define DEFAULT_PORT 12345

std::string getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm* timeinfo;
    timeinfo = localtime(&tv.tv_sec);

    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03ld",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec / 1000);

    return std::string(buffer);
}


void handleClient(int new_socket) {
    char buffer[1024] = {0};
	while(true)
	{
		// Receive client time
		memset(buffer, 0, sizeof(buffer));
		if (recv(new_socket, buffer, 1024, 0) < 0) {
			perror("recv failed");
			exit(EXIT_FAILURE);
		}

		std::string client_time(buffer);
		std::cout << "Client's time: " << client_time << std::endl;

		// Send both client and server times back to client
		std::string server_time = getTimestamp();

		// Send server time to client
		std::string combined_times = client_time + ",Server:" + server_time;
		send(new_socket, combined_times.c_str(), combined_times.size(), 0);
		std::cout << "Server send:" << combined_times << std::endl;
	}
    close(new_socket);
}

int main(int argc, char* argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    int port = DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Create a socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 12345
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Forcefully attaching socket to the port 12345
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
	{
            perror("accept");
            exit(EXIT_FAILURE);
	}
        std::cout << "Connection established\n";

        std::thread clientThread(handleClient, new_socket);
        clientThread.detach(); // Detach the thread to allow it to run independently
    }
    return 0;
}
