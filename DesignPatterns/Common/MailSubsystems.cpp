#include "MailSubsystems.h"

#include <iostream>

namespace common {

std::string toString(Priority priority)
{
    switch (priority) {
    case Priority::Low:    return "5 (Low)";
    case Priority::Normal: return "3 (Normal)";
    case Priority::High:   return "1 (High)";
    }
    return "3 (Normal)";
}

std::string join(const std::vector<std::string>& items, const std::string& separator)
{
    std::string result;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0)
            result += separator;
        result += items[i];
    }
    return result;
}

bool AddressValidator::isValid(const std::string& address) const
{
    if (address.find(' ') != std::string::npos)
        return false;
    auto at = address.find('@');
    if (at == std::string::npos || at == 0)
        return false;
    auto dot = address.find('.', at + 2);
    return dot != std::string::npos && dot + 1 < address.size();
}

void MimeEncoder::begin()
{
    message_.str("");
    message_.clear();
    headerCount_ = 0;
    attachmentCount_ = 0;
    addHeader("MIME-Version", "1.0");
}

void MimeEncoder::addHeader(const std::string& name, const std::string& value)
{
    message_ << name << ": " << value << "\r\n";
    ++headerCount_;
}

void MimeEncoder::addBody(const std::string& body, bool html)
{
    message_ << "--BOUNDARY\r\n"
             << "Content-Type: " << (html ? "text/html" : "text/plain") << "; charset=utf-8\r\n\r\n"
             << body << "\r\n";
}

void MimeEncoder::addAttachment(const Attachment& attachment)
{
    message_ << "--BOUNDARY\r\n"
             << "Content-Disposition: attachment; filename=\"" << attachment.fileName << "\"\r\n"
             << "Content-Transfer-Encoding: base64\r\n\r\n"
             << "<" << attachment.sizeBytes << " bytes of base64>\r\n";
    ++attachmentCount_;
}

std::string MimeEncoder::finish()
{
    message_ << "--BOUNDARY--\r\n";
    std::string data = message_.str();
    std::cout << "  [MIME] Encoded " << headerCount_ << " headers, body and "
              << attachmentCount_ << " attachment(s) into " << data.size() << " bytes\n";
    return data;
}

bool SmtpConnection::connect(const std::string& host, int port)
{
    connected_ = !host.empty();
    if (connected_)
        std::cout << "  [SMTP] Connected to " << host << ":" << port << "\n";
    return connected_;
}

void SmtpConnection::startTls()
{
    std::cout << "  [SMTP] STARTTLS negotiated\n";
}

bool SmtpConnection::authenticate(const std::string& username, const std::string& password)
{
    authenticated_ = connected_ && !password.empty();
    std::cout << "  [SMTP] AUTH " << username << (authenticated_ ? " -> 235 OK" : " -> 535 rejected") << "\n";
    return authenticated_;
}

bool SmtpConnection::sendMail(const std::string& from, const std::vector<std::string>& recipients, const std::string& data)
{
    if (!authenticated_)
        return false;
    std::cout << "  [SMTP] MAIL FROM:<" << from << ">\n";
    for (const auto& recipient : recipients)
        std::cout << "  [SMTP] RCPT TO:<" << recipient << ">\n";
    std::cout << "  [SMTP] DATA (" << data.size() << " bytes) -> 250 OK\n";
    return true;
}

void SmtpConnection::quit()
{
    if (connected_)
        std::cout << "  [SMTP] QUIT\n";
    connected_ = false;
    authenticated_ = false;
}

void MailLogger::info(const std::string& message)
{
    std::cout << "  [Log] INFO: " << message << "\n";
}

void MailLogger::error(const std::string& message)
{
    std::cout << "  [Log] ERROR: " << message << "\n";
}

}
