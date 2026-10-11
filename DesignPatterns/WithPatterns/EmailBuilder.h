#pragma once

#include "Email.h"

#include <cstddef>
#include <string>

namespace patterns {

class EmailBuilder {
public:
    static constexpr std::size_t MaxAttachmentBytes = 10 * 1024 * 1024;

    EmailBuilder& from(const std::string& address);
    EmailBuilder& to(const std::string& address);
    EmailBuilder& cc(const std::string& address);
    EmailBuilder& bcc(const std::string& address);
    EmailBuilder& subject(const std::string& subject);
    EmailBuilder& textBody(const std::string& body);
    EmailBuilder& htmlBody(const std::string& body);
    EmailBuilder& attach(const std::string& fileName, std::size_t sizeBytes);
    EmailBuilder& priority(common::Priority priority);

    Email build() const;

private:
    Email email_;
};

}
