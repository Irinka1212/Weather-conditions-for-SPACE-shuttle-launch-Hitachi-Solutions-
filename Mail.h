#pragma once
#include <iostream>
#include <string>
#include <vector>

class Mail 
{
public:
    Mail(const std::string& sender, const std::string& recipient, const std::string& subject, const std::string& body)
        : sender_(sender), recipient_(recipient), subject_(subject), body_(body) {}

    const std::string& getSender() const { return sender_; }
    const std::string& getRecipient() const { return recipient_; }
    const std::string& getSubject() const { return subject_;  }
    const std::string& getBody() const { return body_; }
    const std::string& getContentType() const { return contentType_;}

private:
    std::string sender_;
    const std::string& recipient_;
    std::string subject_;
    std::string body_;
    std::string contentType_;
};
