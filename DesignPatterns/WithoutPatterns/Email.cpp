#include "Email.h"

#include <stdexcept>
#include <utility>

namespace plain {

Email::Email(std::string from,
             std::vector<std::string> to,
             std::vector<std::string> cc,
             std::vector<std::string> bcc,
             std::string subject,
             std::string body,
             bool isHtml,
             common::Priority priority,
             std::vector<common::Attachment> attachments)
    : from(std::move(from)),
      to(std::move(to)),
      cc(std::move(cc)),
      bcc(std::move(bcc)),
      subject(std::move(subject)),
      body(std::move(body)),
      isHtml(isHtml),
      priority(priority),
      attachments(std::move(attachments))
{
    if (this->from.empty())
        throw std::invalid_argument("Email needs a sender");
    if (this->to.empty() && this->cc.empty() && this->bcc.empty())
        throw std::invalid_argument("Email needs at least one recipient");
    if (this->subject.empty())
        throw std::invalid_argument("Email needs a subject");

    std::size_t totalBytes = 0;
    for (const auto& attachment : this->attachments)
        totalBytes += attachment.sizeBytes;
    if (totalBytes > 10 * 1024 * 1024)
        throw std::invalid_argument("Attachments exceed the 10 MB limit");
}

}
