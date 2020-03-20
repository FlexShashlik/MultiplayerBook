#include "RoboCatPCH.h"
#include <iostream>
#include <set>

using namespace std;

const int MTU = 1300;
const char* serverStr = "Hello from server";

set<TCPSocketPtr> written;
vector<TCPSocketPtr> readBlockSockets, writeBlockSockets;
vector<TCPSocketPtr> readableSockets, writableSockets;

void ProcessNewClient(TCPSocketPtr socket, SocketAddress address)
{
    cout << address.ToString() << " connected!\n";
}

void ProcessDataFromClient(TCPSocketPtr newSocket, char* data, int size)
{
    cout << "Message: ";
    cout.write(data, size);
    cout << endl;

    for (const TCPSocketPtr& socket : writableSockets)
    {
        if (socket != newSocket)
        {
            socket->Send(data, size);
        }
    }
}

int main()
{
    SocketUtil::StaticInit();
    TCPSocketPtr serverSocket = SocketUtil::CreateTCPSocket(INET);
    serverSocket->SetNonBlockingMode(true);

    SocketAddressPtr receivingAddres = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:8080");
    //SocketAddress receivingAddres(INADDR_ANY, 48000);

    if (serverSocket->Bind(*receivingAddres) != NO_ERROR)
    {
        return 1;
    }

    if (serverSocket->Listen() != NO_ERROR)
    {
        return 1;
    }

    readBlockSockets.push_back(serverSocket);
    
    while (true)
    {
        int selectedCount = SocketUtil::Select(&readBlockSockets, &readableSockets, &writeBlockSockets, &writableSockets, nullptr, nullptr);
        if (selectedCount > 0)
        {
            //we got a packet—loop through the set ones...
            for (const TCPSocketPtr& socket : readableSockets)
            {
                if (socket == serverSocket)
                {
                    //it's the listen socket, accept a new connection
                    SocketAddress newClientAddress;
                    auto newSocket = serverSocket->Accept(newClientAddress);
                    readBlockSockets.push_back(newSocket);
                    writeBlockSockets.push_back(newSocket);
                    ProcessNewClient(newSocket, newClientAddress);
                }
                else
                {
                    //it's a regular socket—process the data...
                    char segment[MTU] {};
                    int dataSize = socket->Receive(segment, MTU);
                    if (dataSize > 0)
                    {
                        ProcessDataFromClient(socket, segment, dataSize);
                    }
                }
            }

            for (const TCPSocketPtr& socket : writableSockets)
            {
                if (written.find(socket) == written.end())
                {
                    socket->Send(serverStr, strlen(serverStr));
                    written.insert(socket);
                }
            }
        }
        else if (selectedCount < 0)
        {
            SocketUtil::ReportError("SocketUtil::Select");
        }
    }
}
