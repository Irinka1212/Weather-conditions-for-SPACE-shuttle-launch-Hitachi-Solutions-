#include "SMTPClient.h"
#include "Mail.h"
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <boost/array.hpp>
#include <boost/system/error_code.hpp>

std::string base64_encode(const std::string& input) 
{
	BIO* bmem = nullptr;
	BIO* b64 = nullptr;
	BUF_MEM* bptr = nullptr;

	b64 = BIO_new(BIO_f_base64());
	if (!b64) 
	{
		return "";
	}

	bmem = BIO_new(BIO_s_mem());
	if (!bmem) 
	{
		BIO_free_all(b64);
		return "";
	}

	b64 = BIO_push(b64, bmem);

	BIO_write(b64, input.data(), static_cast<int>(input.length()));
	BIO_flush(b64);

	BIO_get_mem_ptr(b64, &bptr);

	std::string encoded_data(bptr->data, bptr->length);

	BIO_free_all(b64);

	return encoded_data;
}

SMTPClient::SMTPClient() :
	sslContext_(boost::asio::ssl::context::tls_client),
	sslSocket_(service_, sslContext_),
	socket_(sslSocket_.next_layer())
{}

void SMTPClient::write() 
{
	boost::system::error_code error;
	const std::string str = message_.str();
	const boost::asio::const_buffers_1 buffer = boost::asio::buffer(str);

	if (tlsEnabled_) 
	{
		sslSocket_.write_some(buffer, error);
	}
	else 
	{
		socket_.write_some(buffer, error);
	}

	if (error) 
	{
		std::cerr << "error: " << error.message();
	}
	message_.str(std::string());
}

std::string SMTPClient::read()
{
	boost::array<char, 4096> buffer;
	std::string answer;
	boost::system::error_code error;
	std::size_t bytesReceived = 0;

	std::function<void(const boost::system::error_code&, std::size_t)> readHandler;

	if (tlsEnabled_)
	{
		readHandler = [&](const boost::system::error_code& ec, std::size_t bytesRead)
		{
			if (!ec)
			{
				bytesReceived = bytesRead;
				answer.assign(buffer.data(), bytesReceived);
			}
			else
			{
				error = ec;
			}
		};

		sslSocket_.async_read_some(boost::asio::buffer(buffer), readHandler);
	}
	else
	{
		readHandler = [&](const boost::system::error_code& ec, std::size_t bytesRead)
		{
			if (!ec)
			{
				bytesReceived = bytesRead;
				answer.assign(buffer.data(), bytesReceived);
			}
			else
			{
				error = ec;
			}
		};

		socket_.async_receive(boost::asio::buffer(buffer), readHandler);
	}

	if (error)
	{
		throw boost::system::system_error(error);
	}

	return answer;
}

void SMTPClient::handshake() 
{
	std::string serverName = socket_.remote_endpoint().address().to_string();
	if (tlsEnabled_) 
	{
		boost::system::error_code error;
		tlsEnabled_ = false;
		serverId_ = read();
		message_ << "EHLO " << serverName << "\r\n";
		write();
		read();
		message_ << "STARTTLS" << "\r\n";
		write();
		read();
		sslSocket_.handshake(SslSocket::client, error);
		if (error) 
		{
			std::cerr << "Handshake error: " << error.message();
		}
		tlsEnabled_ = true;
	}
	else 
	{
		serverId_ = read();
	}
	message_ << "EHLO " << serverName << "\r\n";
	write();
	read();
}

void SMTPClient::connect(const std::string& hostname, unsigned short port) 
{
	boost::system::error_code error = boost::asio::error::host_not_found;
	boost::asio::ip::tcp::resolver resolver(service_);
	boost::asio::ip::tcp::resolver::query query(hostname, std::to_string(port));
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	boost::asio::ip::tcp::resolver::iterator end;

	while (error && endpoint_iterator != end) 
	{
		socket_.close();
		socket_.connect(*endpoint_iterator++, error);
	}

	if (error) 
	{
		std::cerr << "error: " << error.message();
	}
	else 
	{
		handshake();
	}
}

void SMTPClient::authLogin(const std::string& hostname, unsigned short port, const std::string& username, const std::string& password)
{
	connect(hostname, port);
	std::string user_hash = base64_encode(username);
	std::string pswd_hash = base64_encode(password);

	message_ << "AUTH LOGIN\r\n";
	write();
	read();
	message_ << user_hash;
	write();
	read();
	message_ << pswd_hash;
	write();
	read();
}

void SMTPClient::sendEmail(const Mail& mail)
{
	const std::string& sender = mail.getSender();
	const std::string& recipient = mail.getRecipient();
	const std::string& contentType = mail.getContentType();

	message_ << "MAIL FROM: <" << sender << ">\r\n";
	write();
	read();

	message_ << "RCPT TO: <" << recipient << ">\r\n";
	write();
	read();

	message_ << "DATA\r\n";
	write();
	read();

	message_ << "From: " << sender << "\r\n"; 

	if (!contentType.empty()) 
	{
		message_ << "MIME-Version: 1.0\r\n";
		message_ << "Content-Type: " << contentType << "\r\n";
	}

	message_ << "Subject: " << mail.getSubject() << "\r\n";

	message_ << "\r\n";

	message_ << mail.getBody() << "\r\n.\r\n";
	write();
	read();
}
