#pragma once
#include <string>
#include <winsock.h>
#pragma comment(lib,"wsock32")
#include <openssl/ssl.h>
#include <openssl/err.h>

class SMTPClient 
{
public:
    SMTPClient(const std::string& smtpServer, const std::string& password);
    ~SMTPClient() { WSACleanup(); }

    SSL_CTX* initSSL();
    bool performHandshake(SSL* ssl);
    bool sendSSLData(SSL* ssl, const char* data, int dataSize);
    bool receiveSSLData(SSL* ssl, char* buffer, int bufferSize);

    bool sendEmail(const std::string& senderEmail, const std::string& receiverEmail, const std::string& fileName);

private:
    std::string _smtpServer;
    std::string _password;
    WSADATA _wsaData;

    void sendFile(SOCKET socketVar, const std::string& fileName);
};
