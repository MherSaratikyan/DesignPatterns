#pragma once

#include "../Common/MailSubsystems.h"

#include <string>
#include <vector>

namespace plain {

struct Email {
    Email(std::string from,
          std::vector<std::string> to,
          std::vector<std::string> cc,
          std::vector<std::string> bcc,
          std::string subject,
          std::string body,
          bool isHtml,
          common::Priority priority,
          std::vector<common::Attachment> attachments);

    std::string from;
    std::vector<std::string> to;
    std::vector<std::string> cc;
    std::vector<std::string> bcc;
    std::string subject;
    std::string body;
    bool isHtml;
    common::Priority priority;
    std::vector<common::Attachment> attachments;
};

}
