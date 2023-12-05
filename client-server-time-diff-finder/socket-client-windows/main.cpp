#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <ctime>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345

std::string getTimestamp() {
    SYSTEMTIME st;
    // GetSystemTime(&st);
    GetLocalTime(&st);

    char buffer[1024] = { 0 };
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    return std::string(buffer);
}

// Time diff in nanao sec
ULONGLONG CalculateTimeDifference(const SYSTEMTIME& time1, const SYSTEMTIME& time2) {
    FILETIME fileTime1, fileTime2;
    SystemTimeToFileTime(&time1, &fileTime1);
    SystemTimeToFileTime(&time2, &fileTime2);

    ULARGE_INTEGER uliTime1, uliTime2;
    uliTime1.LowPart = fileTime1.dwLowDateTime;
    uliTime1.HighPart = fileTime1.dwHighDateTime;
    uliTime2.LowPart = fileTime2.dwLowDateTime;
    uliTime2.HighPart = fileTime2.dwHighDateTime;

    ULONGLONG timeValue1 = uliTime1.QuadPart;
    ULONGLONG timeValue2 = uliTime2.QuadPart;

    // return (timeValue2 > timeValue1) ? (timeValue2 - timeValue1) : (timeValue1 - timeValue2);
    return (timeValue2 > timeValue1) ? (timeValue2 - timeValue1) : (timeValue1 - timeValue2);
}


// Time diff in nanao sec
ULONGLONG GetTimeInNanoSec(const SYSTEMTIME& time1) {
    FILETIME fileTime1, fileTime2;
    SystemTimeToFileTime(&time1, &fileTime1);

    ULARGE_INTEGER uliTime1;
    uliTime1.LowPart = fileTime1.dwLowDateTime;
    uliTime1.HighPart = fileTime1.dwHighDateTime;

    return uliTime1.QuadPart;
}

double ConvertNanosecondsToMilliseconds(ULONGLONG nanoseconds) {
    return static_cast<double>(nanoseconds) / 10000; // Divide by 10,000 to convert 100-nanoseconds to milliseconds
}

double ConvertNanosecondsToMilliseconds(double nanoseconds) {
    return nanoseconds / 10000; // Divide by 10,000 to convert 100-nanoseconds to milliseconds
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server_ip> <server_port>\n";
        return 1;
    }

    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char server_reply[1024] = { 0 };
    const char* message = "get_time";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cout << "Could not create socket\n";
        return 1;
    }

    // Set up server information from command line arguments
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int result = inet_pton(AF_INET, server_ip, &server.sin_addr);
    if (result != 1) {
        std::cout << "Invalid address\n";
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    // Connect to remote server
    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cout << "Connect error\n";
        return 1;
    }

    while (true) 
    {
        std::string client_time = getTimestamp();

        std::string client_send_Time = "Client:" + client_time;

        // Send client time to server
        send(s, client_send_Time.c_str(), client_send_Time.size(), 0);

        // Receive client and server times from server
        char client_server_times[1024] = { 0 };
        if (recv(s, client_server_times, 1024, 0) < 0) {
            std::cout << "recv failed\n";
            return 1;
        }

        SYSTEMTIME stResponseReceivedTime;
        // GetSystemTime(&stResponseReceivedTime);
        GetLocalTime(&stResponseReceivedTime);

        ULONGLONG nanoResponseReceivedTime = GetTimeInNanoSec(stResponseReceivedTime);

        std::cout << "Received times: " << client_server_times << std::endl;

        // Calculate time difference
        std::string received_times(client_server_times);
        std::size_t client_pos = received_times.find("Client:");
        std::size_t server_pos = received_times.find(",Server:");

        std::string client_time_str = received_times.substr(client_pos + 7, server_pos - (client_pos + 7));
        std::string server_time_str = received_times.substr(server_pos + 8);

        SYSTEMTIME clientSendTime;
        sscanf_s(client_time_str.c_str(), "%hd-%hd-%hdT%hd:%hd:%hd.%hd",
            &clientSendTime.wYear, &clientSendTime.wMonth, &clientSendTime.wDay,
            &clientSendTime.wHour, &clientSendTime.wMinute, &clientSendTime.wSecond,
            &clientSendTime.wMilliseconds);


        SYSTEMTIME serverSendTime;
        sscanf_s(server_time_str.c_str(), "%hd-%hd-%hdT%hd:%hd:%hd.%hd",
            &serverSendTime.wYear, &serverSendTime.wMonth, &serverSendTime.wDay,
            &serverSendTime.wHour, &serverSendTime.wMinute, &serverSendTime.wSecond,
            &serverSendTime.wMilliseconds);


        ULONGLONG nanoSendTime = GetTimeInNanoSec(clientSendTime);

        ULONGLONG timeDiff = CalculateTimeDifference(clientSendTime, serverSendTime);

        double serverClientDiff = ConvertNanosecondsToMilliseconds(timeDiff);

        //ULONGLONG networkTraficTime = (nanoResponseReceivedTime - nanoSendTime);

        //double orgDiff = serverClientDiff;

        //if (serverClientDiff != 0)
        //{
        //    // Diff = time diff between server and client - Trafic time /2
        //    orgDiff = serverClientDiff - (double)networkTraficTime / 2.0;

        //}
        //auto client_time_point = std::chrono::system_clock::from_time_t(std::stoi(client_time_received.substr(6, 4)), std::stoi(client_time_received.substr(11, 2)), std::stoi(client_time_received.substr(14, 2)), std::stoi(client_time_received.substr(17, 2)), std::stoi(client_time_received.substr(20, 2)), std::stoi(client_time_received.substr(23)));
        //auto server_time_point = std::chrono::system_clock::from_time_t(std::stoi(server_time_received.substr(6, 4)), std::stoi(server_time_received.substr(11, 2)), std::stoi(server_time_received.substr(14, 2)), std::stoi(server_time_received.substr(17, 2)), std::stoi(server_time_received.substr(20, 2)), std::stoi(server_time_received.substr(23)));

        //auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(server_time_point - client_time_point);

        std::cout << "Time difference: " << serverClientDiff << " Milliseconds seconds\n";

        // Wait approximately 1 second before repeating
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
