#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace common {

enum class Priority { Low, Normal, High };

std::string toString(Priority priority);
std::string join(const std::vector<std::string>& items, const std::string& separator);

struct Attachment {
    std::string fileName;
    std::size_t sizeBytes = 0;
};

struct SmtpSettings {
    std::string host;
    int port = 587;
    std::string username;
    std::string password;
    bool useTls = true;
};

class AddressValidator {
public:
    bool isValid(const std::string& address) const;
};

class MimeEncoder {
public:
    void begin();
    void addHeader(const std::string& name, const std::string& value);
    void addBody(const std::string& body, bool html);
    void addAttachment(const Attachment& attachment);
    std::string finish();

private:
    std::ostringstream message_;
    int headerCount_ = 0;
    int attachmentCount_ = 0;
};

class SmtpConnection {
public:
    bool connect(const std::string& host, int port);
    void startTls();
    bool authenticate(const std::string& username, const std::string& password);
    bool sendMail(const std::string& from, const std::vector<std::string>& recipients, const std::string& data);
    void quit();

private:
    bool connected_ = false;
    bool authenticated_ = false;
};

class MailLogger {
public:
    void info(const std::string& message);
    void error(const std::string& message);
};

}
