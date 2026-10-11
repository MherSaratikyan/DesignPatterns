#pragma once

#include "../Common/MailSubsystems.h"

#include <string>
#include <vector>

namespace patterns {

class EmailBuilder;

class Email {
public:
    const std::string& from() const { return from_; }
    const std::vector<std::string>& to() const { return to_; }
    const std::vector<std::string>& cc() const { return cc_; }
    const std::vector<std::string>& bcc() const { return bcc_; }
    const std::string& subject() const { return subject_; }
    const std::string& body() const { return body_; }
    bool isHtml() const { return isHtml_; }
    common::Priority priority() const { return priority_; }
    const std::vector<common::Attachment>& attachments() const { return attachments_; }

    std::vector<std::string> allAddresses() const;
    std::vector<std::string> allRecipients() const;

private:
    friend class EmailBuilder;
    Email() = default;

    std::string from_;
    std::vector<std::string> to_;
    std::vector<std::string> cc_;
    std::vector<std::string> bcc_;
    std::string subject_;
    std::string body_;
    bool isHtml_ = false;
    common::Priority priority_ = common::Priority::Normal;
    std::vector<common::Attachment> attachments_;
};

}
