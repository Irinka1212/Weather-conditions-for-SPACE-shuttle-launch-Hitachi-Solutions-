#include "SMTPClient.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

SMTPClient::SMTPClient(const std::string& smtpServer, const std::string& password) 
{
    _smtpServer = smtpServer;
    _password = password;

    if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) 
    {
        std::cout << "Failed to initialize winsock.\n";
    }
}

void SMTPClient::sendFile(SOCKET socketVar, const std::string& fileName) 
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cout << "Failed to open the file.\n";
        return;
    }

    std::string line;
    std::vector<std::string> data;
    while (std::getline(file, line)) 
    {
        data.push_back(line);
    }

    file.close();

    std::stringstream bufferStream;
    for (const auto& entry : data) 
    {
        bufferStream << entry << '\n';
    }
    std::string fileContents = bufferStream.str();

    send(socketVar, fileContents.c_str(), fileContents.length(), 0);
}
 
bool SMTPClient::sendEmail(const std::string& senderEmail, const std::string& receiverEmail, const std::string& fileName) 
{
    // Creating socket
    SOCKET socketVar;
    socketVar = socket(AF_INET, SOCK_STREAM, 0);
    if (socketVar == INVALID_SOCKET) 
    {
        std::cout << "Failed to create socket.\n";
        return false;
    }

    // Getting host
    struct hostent* host;
    host = gethostbyname(_smtpServer.c_str());
    if (host == NULL) 
    {
        std::cout << "Failed to get hostname.\n";
        closesocket(socketVar);
        return false;
    }

    // Getting server address
    SOCKADDR_IN serverAddress;
    serverAddress.sin_family = AF_INET; // IPv4 address family.
    serverAddress.sin_port = htons(465); //convert the port number from host byte order to network byte order
    serverAddress.sin_addr.s_addr = *((unsigned long*)host->h_addr); //sets the IP address of the server in the server address structure
    if (connect(socketVar, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) 
    {
        std::cout << "Failed to connect to server.\n";
        closesocket(socketVar);
        return false;
    }

    // Starting the SMTP conversation
    std::string ehloCommand = "EHLO localhost\r\n";
    send(socketVar, ehloCommand.c_str(), ehloCommand.length(), 0);

    // Receiving response from the server
    char response[4096];
    memset(response, 0, sizeof(response)); // filling a block of memory with a particular value
    recv(socketVar, response, sizeof(response), 0); // receiving data from the connected socket

    if (strstr(response, "AUTH") == nullptr)
    {
        std::cout << "Authentication not supported by the server.\n";
    }
    else
    {
        std::string authCommand = "AUTH LOGIN\r\n";
        send(socketVar, authCommand.c_str(), authCommand.length(), 0);

        memset(response, 0, sizeof(response));
        recv(socketVar, response, sizeof(response), 0);

        if (response[0] != '2') // status code for successful command is 2
        {
            std::cout << "Failed to send authentication.\n";
            closesocket(socketVar);
            return false;
        }
    }

    send(socketVar, senderEmail.c_str(), senderEmail.length(), 0);
    send(socketVar, "\r\n", 2, 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to send username.\n";
        closesocket(socketVar);
        return false;
    }

    send(socketVar, _password.c_str(), _password.length(), 0);
    send(socketVar, "\r\n", 2, 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to authenticate with the server.\n";
        closesocket(socketVar);
        return false;
    }

    std::string mailFromCommand = "MAIL FROM:<" + senderEmail + ">\r\n";
    send(socketVar, mailFromCommand.c_str(), mailFromCommand.length(), 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to set sender email.\n";
        closesocket(socketVar);
        return false;
    }

    std::string rcptToCommand = "RCPT TO:<" + receiverEmail + ">\r\n";
    send(socketVar, rcptToCommand.c_str(), rcptToCommand.length(), 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to set receiver email.\n";
        closesocket(socketVar);
        return false;
    }

    std::string dataCommand = "DATA\r\n";
    send(socketVar, dataCommand.c_str(), dataCommand.length(), 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to start email content.\n";
        closesocket(socketVar);
        return false;
    }

    sendFile(socketVar, fileName);

    std::string endMarker = "\r\n.\r\n";
    send(socketVar, endMarker.c_str(), endMarker.length(), 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    if (response[0] != '2')
    {
        std::cout << "Failed to send email.\n";
        closesocket(socketVar);
        return false;
    }

    std::string quitCommand = "QUIT\r\n";
    send(socketVar, quitCommand.c_str(), quitCommand.length(), 0);

    memset(response, 0, sizeof(response));
    recv(socketVar, response, sizeof(response), 0);

    closesocket(socketVar);

    return true;
}











