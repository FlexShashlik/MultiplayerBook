#include "RoboCatPCH.h"
#include <iostream>
#include <time.h>

using namespace std;

const int MTU = 1300;

void ProcessDataFromServer(TCPSocketPtr socket, char* data, int size)
{
    cout << endl;
    cout << "Message: \n";
    cout.write(data, size);
    cout << endl;
}

int main(int argc, const char** argv)
{
    if (argc != 2) 
    {
        StringUtils::Log("Usage: %s <client_name>", __argv[0]);
        return -1;
    }

    string clientName = StringUtils::Sprintf("Hello from client: %s\n", argv[1]);

    SocketUtil::StaticInit();
    TCPSocketPtr clientSocket = SocketUtil::CreateTCPSocket(INET);

    SocketAddressPtr receivingAddres = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:8080");
    //SocketAddress receivingAddres(INADDR_ANY, 48000);

    if (clientSocket->Connect(*receivingAddres) != NO_ERROR)
    {
        return 1;
    }

    vector<TCPSocketPtr> readBlockSockets, writeBlockSockets;
    readBlockSockets.push_back(clientSocket);
    writeBlockSockets.push_back(clientSocket);

    vector<TCPSocketPtr> readableSockets, writableSockets;

    while (true)
    {
        int selectedCount = SocketUtil::Select(&readBlockSockets, &readableSockets, &writeBlockSockets, &writableSockets, nullptr, nullptr);
        if (selectedCount > 0)
        {
            //we got a packet—loop through the set ones...
            for (const TCPSocketPtr& socket : readableSockets)
            {
                //it's a regular socket—process the data...
                char segment[MTU] {};
                int dataSize = socket->Receive(segment, MTU);
                if (dataSize > 0)
                {
                    ProcessDataFromServer(socket, segment, dataSize);
                }
            }

            for (const TCPSocketPtr& socket : writableSockets)
            {
                socket->Send(clientName.c_str(), clientName.size() + 1);
                Sleep(500);
            }
        }
        else if(selectedCount < 0)
        {
            SocketUtil::ReportError("SocketUtil::Select");
        }
    }
}
