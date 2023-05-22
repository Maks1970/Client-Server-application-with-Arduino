#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <fstream>
#include<mutex>


#include <iostream>
#include "ARdu.h"
#include <thread>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

extern HANDLE hCOMPort;


int key = 0;
using namespace std;
mutex mut;
bool check = true;


int __cdecl main(void)
{
    
    WSADATA wsaData;
    int iResult;
    int con = 1;
    
    SOCKET ListenSocket = INVALID_SOCKET;
    

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    //int iSendResult;
    //char recvbuf[DEFAULT_BUFLEN];
    //int recvbuflen = DEFAULT_BUFLEN;
    int tt=20;

    
    conCom();

    cout << "\n";

    
            bool menu = true;
        
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    
    int sizehints = sizeof(hints);
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    std::cout << "Server started \n";
    std::cout << "Temperature Check\n";
    std::cout << "Select temperature to start\n";
 

    
    cin >> tt;
    while (true)
    {
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000,'\n');    
            cout << "Enter INTEGER\n";
            cin >> tt;
        }
        else
        {
            break;
        }
    }
        
   
    //запуск потоків запису та читання СОМ порта
    COMPortStartThreads();

    sendT(tt);

  //  cout << "Temperature entered! T=" << tt << "\n";
    
  
    
    while (con == 1) {
        
    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
  
    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

   
  
        SOCKET ClientSocket = INVALID_SOCKET;
        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        
        // Accept a client socket
            ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else { send(ClientSocket, (char*)&check, sizeof(check), 0); }
        
        // No longer need server socket
        closesocket(ListenSocket);

       // std::cout << "Conected Client!\n";
        
        
        
        //thread th2(ReadCOM, ref(check));
        menu = true;
       
        while (menu==true)
        {
         //  
           // bool chk=check;
           
            recv(ClientSocket, (char*)&key, sizeof(key), 0);
            
            int k = 0;
            const int BUFFER_SIZE = 1024;
            //char bufferFile[BUFFER_SIZE];
            //char fileRequested[FILENAME_MAX];
            int byRecv;
            std::ofstream file;
          //  mut.lock();
            while (key == 0xC1)
            {
               //  if (k == 0)send(ClientSocket, (char*)&key, sizeof(key), 0);
               
                bool clientClose = false;
                char fileRequested[FILENAME_MAX];
                const int fileAvailable = 200;
                const int fileNotfound = 404;
                const int BUFFER_SIZE = 1024;
                char bufferFile[BUFFER_SIZE];

                std::ifstream file;
                do {
                    int fileDownloaded = 0;
                    memset(fileRequested, 0, FILENAME_MAX);
                    /*std::cout << "Enter file name: " << std::endl;
                    std::cin >> fileRequested, FILENAME_MAX;  */
                    if (strcpy_s(fileRequested, FILENAME_MAX, "Datatem.txt") != 0) {
                        cout << "Errrr";
                        // Обработка ошибки
                    }

                    byRecv = send(ClientSocket,(char*)&fileRequested, FILENAME_MAX, 0);
                    if (byRecv == 0 || byRecv == -1) {
                        clientClose = true;
                        break;
                    }

                    file.open(fileRequested, std::ios::binary);
                    if (file.is_open()) {
                        int bySendinfo = send(ClientSocket, (char*)&fileAvailable, sizeof(int), 0);
                        if (bySendinfo == 0 || bySendinfo == -1) {
                            clientClose = true;
                        }
                        file.seekg(0, std::ios::end);
                        long fileSize = file.tellg();
                        bySendinfo = send(ClientSocket, (char*)&fileSize, sizeof(long), 0);
                        if (bySendinfo == 0 || bySendinfo == -1) {
                            clientClose = true;
                        }
                        file.seekg(0, std::ios::beg);
                        do {
                            file.read(bufferFile, BUFFER_SIZE);
                            if (file.gcount() > 0)
                                bySendinfo = send(ClientSocket, (char*)&bufferFile, file.gcount(), 0);
                            if (bySendinfo == 0 || bySendinfo == -1) {
                                clientClose = true;
                                break;
                            }
                        } while (file.gcount() > 0);
                       
                        file.close();
                        std::cout << "File sended!!!\n";
                        clientClose = true;
                        key = 0;
                        //if (k == 0)send(ClientSocket, (char*)&key, sizeof(key), 0);
                    }
                    else {
                        int bySendCode = send(ClientSocket, (char*)&fileNotfound, sizeof(int), 0);
                        if (bySendCode == 0 || bySendCode == -1) {
                            clientClose = true;
                            break;
                        }
                    }
                } while (!clientClose);

            }
          //  mut.unlock();
            if (key==3) {
                recv(ClientSocket, (char*)&tt, sizeof(tt), 0);
                cout << "Set new control T ";
                sendT(tt);
                key = 0;
            }
            if (key == 4) {
                key = 0;
                menu = false;
              
            }
            if (key == 0xCC) {
                sendStop(check);
                cout << "system stoped\n";
                key = 0;
            }
            if (key == 0xEE) {
                sendStart(check);
                cout << "system started\n";
                key = 0;
            }

        }
        //tth3.detach();
        
        
    }
    
    
  
   
    
    // system("pause");
    void COMTerminate(void);
    CloseHandle(hCOMPort);                  //закрыть порт

    
    return 0;
}