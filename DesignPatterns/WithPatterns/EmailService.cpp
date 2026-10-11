#include "EmailService.h"

#include <utility>

namespace patterns {

EmailService::EmailService(common::SmtpSettings settings)
    : settings_(std::move(settings))
{
}

bool EmailService::send(const Email& email)
{
    if (!addressesAreValid(email))
        return false;

    std::string data = encode(email);

    if (!transmit(email, data)) {
        logger_.error("SMTP delivery failed for '" + email.subject() + "'");
        return false;
    }

    logger_.info("Sent '" + email.subject() + "' to " +
                 std::to_string(email.allRecipients().size()) + " recipient(s)");
    return true;
}

bool EmailService::addressesAreValid(const Email& email)
{
    for (const auto& address : email.allAddresses()) {
        if (!validator_.isValid(address)) {
            logger_.error("Invalid address '" + address + "', email not sent");
            return false;
        }
    }
    return true;
}

std::string EmailService::encode(const Email& email)
{
    encoder_.begin();
    encoder_.addHeader("From", email.from());
    if (!email.to().empty())
        encoder_.addHeader("To", common::join(email.to(), ", "));
    if (!email.cc().empty())
        encoder_.addHeader("Cc", common::join(email.cc(), ", "));
    encoder_.addHeader("Subject", email.subject());
    encoder_.addHeader("X-Priority", common::toString(email.priority()));
    encoder_.addBody(email.body(), email.isHtml());
    for (const auto& attachment : email.attachments())
        encoder_.addAttachment(attachment);
    return encoder_.finish();
}

bool EmailService::transmit(const Email& email, const std::string& data)
{
    common::SmtpConnection smtp;
    if (!smtp.connect(settings_.host, settings_.port))
        return false;
    if (settings_.useTls)
        smtp.startTls();

    bool sent = smtp.authenticate(settings_.username, settings_.password) &&
                smtp.sendMail(email.from(), email.allRecipients(), data);

    smtp.quit();
    return sent;
}

}
