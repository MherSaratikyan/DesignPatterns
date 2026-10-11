#include "EmailBuilder.h"

#include <stdexcept>

namespace patterns {

EmailBuilder& EmailBuilder::from(const std::string& address)
{
    email_.from_ = address;
    return *this;
}

EmailBuilder& EmailBuilder::to(const std::string& address)
{
    email_.to_.push_back(address);
    return *this;
}

EmailBuilder& EmailBuilder::cc(const std::string& address)
{
    email_.cc_.push_back(address);
    return *this;
}

EmailBuilder& EmailBuilder::bcc(const std::string& address)
{
    email_.bcc_.push_back(address);
    return *this;
}

EmailBuilder& EmailBuilder::subject(const std::string& subject)
{
    email_.subject_ = subject;
    return *this;
}

EmailBuilder& EmailBuilder::textBody(const std::string& body)
{
    email_.body_ = body;
    email_.isHtml_ = false;
    return *this;
}

EmailBuilder& EmailBuilder::htmlBody(const std::string& body)
{
    email_.body_ = body;
    email_.isHtml_ = true;
    return *this;
}

EmailBuilder& EmailBuilder::attach(const std::string& fileName, std::size_t sizeBytes)
{
    email_.attachments_.push_back({fileName, sizeBytes});
    return *this;
}

EmailBuilder& EmailBuilder::priority(common::Priority priority)
{
    email_.priority_ = priority;
    return *this;
}

Email EmailBuilder::build() const
{
    if (email_.from_.empty())
        throw std::invalid_argument("Email needs a sender");
    if (email_.to_.empty() && email_.cc_.empty() && email_.bcc_.empty())
        throw std::invalid_argument("Email needs at least one recipient");
    if (email_.subject_.empty())
        throw std::invalid_argument("Email needs a subject");

    std::size_t totalBytes = 0;
    for (const auto& attachment : email_.attachments_)
        totalBytes += attachment.sizeBytes;
    if (totalBytes > MaxAttachmentBytes)
        throw std::invalid_argument("Attachments exceed the 10 MB limit");

    return email_;
}

}
