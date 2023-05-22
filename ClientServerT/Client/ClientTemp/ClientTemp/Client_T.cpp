#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
  
   
    //char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;


    std::cout << "User started \n";
    //std::cout << "Student of group BKI-19 \n";
    //std::cout << "Version 1 \n";
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    bool s;
    bool stop;
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    else {
        printf("Conected!\n");
        recv(ConnectSocket, (char*)&s, sizeof(s), 0);
        stop = !s;
    }
    bool menu = true;
    int key = 0;
   // = false;
   
    int t;
    //form pac
    stop = false;
    while (menu)
    {
        ///
        
          
        //
        std::cout << " Take File ? (1) : " << std::endl;
        if (stop == false) {
            std::cout << " Stop control temp ? (2) : " << std::endl;
        }
        else {
            std::cout << " Start control temp ? (2) : " << std::endl;
        }
        std::cout << " Select control temp ? (3) : " << std::endl;
        std::cout << " Disconect ? (4) : " << std::endl;
        std::cin >> key;
        int k = 0;
  //      char welcomeMsg[255];
        const int BUFFER_SIZE = 1024;
        char bufferFile[BUFFER_SIZE];
        char fileRequested[FILENAME_MAX];
        //int byRecv;
        std::ofstream file;
        while (key == 1)
        {
            if (key == 1) {
                std::cout << "File taking...\n";
                key = 0xC1;
            }
              if (k == 0)send(ConnectSocket, (char*)&key, sizeof(key), 0);

            bool clientClose = false;
            int codeAvailable = 404;
            const int fileNotfound = 404;
            long fileRequestedsize = 0;

            do {
                int fileDownloaded = 0;
                memset(fileRequested, 0, FILENAME_MAX);
                int byRecv = recv(ConnectSocket, (char*)&fileRequested, FILENAME_MAX, 0);
                
                if (byRecv == 0 || byRecv == -1) {
                    clientClose = true;
                }

                byRecv = recv(ConnectSocket, (char*)&codeAvailable, sizeof(int), 0);
                if (byRecv == 0 || byRecv == -1) {
                    clientClose = true;
                    break;  
                }
                if (codeAvailable == 200) {
                    byRecv = recv(ConnectSocket, (char*)&fileRequestedsize, sizeof(long), 0);
                    if (byRecv == 0 || byRecv == -1) {
                        clientClose = true;
                        break;
                    }
                    file.open(fileRequested, std::ios::binary | std::ios::trunc);
                    do {
                        memset(bufferFile, 0, BUFFER_SIZE);
                        byRecv = recv(ConnectSocket, (char*)&bufferFile, BUFFER_SIZE, 0);
                        if (byRecv == 0 || byRecv == -1) {
                            clientClose = true;
                            break;
                        }
                        file.write(bufferFile, byRecv);
                        fileDownloaded += byRecv;
                    } while (fileDownloaded < fileRequestedsize);
                    file.close();
                    std::cout << "File recived: " << fileRequested << std::endl;
                    clientClose = true;


                }
                else if (codeAvailable == 404) {
                    std::cout << "Can't open file or file not found!" << std::endl;
                    key = 0;
                    clientClose = true;
                    break;
                }

            } while (!clientClose);
            key = 0;
            k++;
        }

        ////////////////////////////////////////////////////////////////////////////
        if (key == 3) {
            send(ConnectSocket, (char*)&key, sizeof(key), 0);
            std::cout<< "Select temp\n";
            std::cin >> t;
            send(ConnectSocket, (char*)&t, sizeof(t), 0);
            printf("Connection \n");
           // recv(ConnectSocket, (char*)&key, sizeof(key), 0);
            key = 0;
        }
        if (key == 4) {
            send(ConnectSocket, (char*)&key, sizeof(key), 0);
            printf("Connection closed\n");
            menu = false;
        }
        if (key == 2) {
//            send(ConnectSocket, (char*)&key, sizeof(key), 0);   
            if (stop == false) {
                key = 0xCC;
                send(ConnectSocket, (char*)&key, sizeof(key), 0);
                std::cout << " Stop  " << std::endl;
                stop = true;
            }
            else {
                key = 0xEE;
                send(ConnectSocket, (char*)&key, 1, 0);
                std::cout << " Start " << std::endl;
                stop = false;
            }
            //printf("Stoped\n");
            key = 0;
        }
    }
    iResult = shutdown(ConnectSocket, SD_RECEIVE);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    system("pause");
    return 0;
}