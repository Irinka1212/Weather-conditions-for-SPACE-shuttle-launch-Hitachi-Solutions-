#pragma once
#include <string>
#include <winsock.h>
#pragma comment(lib,"wsock32")

class SMTPClient 
{
public:
    SMTPClient(const std::string& smtpServer, const std::string& password);
    ~SMTPClient() { WSACleanup(); }

    bool sendEmail(const std::string& senderEmail, const std::string& receiverEmail, const std::string& fileName);

private:
    std::string _smtpServer;
    std::string _password;
    WSADATA _wsaData;

    void sendFile(SOCKET socketVar, const std::string& fileName);
};
