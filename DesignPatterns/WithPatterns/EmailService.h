#pragma once

#include "../Common/MailSubsystems.h"
#include "Email.h"

#include <string>

namespace patterns {

class EmailService {
public:
    explicit EmailService(common::SmtpSettings settings);

    bool send(const Email& email);

private:
    bool addressesAreValid(const Email& email);
    std::string encode(const Email& email);
    bool transmit(const Email& email, const std::string& data);

    common::SmtpSettings settings_;
    common::AddressValidator validator_;
    common::MimeEncoder encoder_;
    common::MailLogger logger_;
};

}
