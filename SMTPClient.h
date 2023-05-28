#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <sstream>
#include "Mail.h"

class SMTPClient 
{
public:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> SslSocket;

    SMTPClient();

    void write();
    std::string read();

    void handshake();
    void connect(const std::string& hostname, unsigned short port);
    void authLogin(const std::string& hostname, unsigned short port, const std::string& username, const std::string& password);

    void sendEmail(const Mail& mail);
private:
    std::string serverId_;
    std::stringstream message_;
    boost::asio::io_service service_;
    boost::asio::ssl::context sslContext_;
    SslSocket sslSocket_;
    boost::asio::ip::tcp::socket& socket_;
    bool tlsEnabled_;
};
