#include "PlainDemo.h"

#include "../Common/MailSubsystems.h"
#include "Email.h"

#include <iostream>
#include <stdexcept>

namespace plain {

void runDemo()
{
    std::cout << "\n==================== WITHOUT DESIGN PATTERNS ====================\n";

    common::SmtpSettings settings{"smtp.shop.com", 587, "mailer@shop.com", "app-password", true};
    common::AddressValidator validator;
    common::MimeEncoder encoder;
    common::MailLogger logger;

    Email invoice("billing@shop.com",
                  {"alice@example.com"},
                  {"accounts@example.com"},
                  {},
                  "Invoice #1042",
                  "<h1>Thank you for your order</h1>",
                  true,
                  common::Priority::High,
                  {{"invoice-1042.pdf", 48'200}});

    Email passwordReset("no-reply@shop.com",
                        {"bob@example.com"},
                        {},
                        {},
                        "Reset your password",
                        "Use this link to choose a new password.",
                        false,
                        common::Priority::Normal,
                        {});

    Email newsletter("news@shop.com",
                     {},
                     {},
                     {"carol@example.com", "dave-at-example.com"},
                     "October newsletter",
                     "<p>New arrivals are here</p>",
                     true,
                     common::Priority::Low,
                     {});

    for (const Email& email : {invoice, passwordReset, newsletter}) {
        std::cout << "\nSending '" << email.subject << "'\n";

        std::vector<std::string> recipients = email.to;
        recipients.insert(recipients.end(), email.cc.begin(), email.cc.end());
        recipients.insert(recipients.end(), email.bcc.begin(), email.bcc.end());

        std::vector<std::string> addresses{email.from};
        addresses.insert(addresses.end(), recipients.begin(), recipients.end());

        bool valid = true;
        for (const auto& address : addresses) {
            if (!validator.isValid(address)) {
                logger.error("Invalid address '" + address + "', email not sent");
                valid = false;
                break;
            }
        }
        if (!valid) {
            std::cout << "  => NOT SENT\n";
            continue;
        }

        encoder.begin();
        encoder.addHeader("From", email.from);
        if (!email.to.empty())
            encoder.addHeader("To", common::join(email.to, ", "));
        if (!email.cc.empty())
            encoder.addHeader("Cc", common::join(email.cc, ", "));
        encoder.addHeader("Subject", email.subject);
        encoder.addHeader("X-Priority", common::toString(email.priority));
        encoder.addBody(email.body, email.isHtml);
        for (const auto& attachment : email.attachments)
            encoder.addAttachment(attachment);
        std::string data = encoder.finish();

        common::SmtpConnection smtp;
        bool sent = smtp.connect(settings.host, settings.port);
        if (sent) {
            if (settings.useTls)
                smtp.startTls();
            sent = smtp.authenticate(settings.username, settings.password) &&
                   smtp.sendMail(email.from, recipients, data);
            smtp.quit();
        }

        if (sent)
            logger.info("Sent '" + email.subject + "' to " + std::to_string(recipients.size()) + " recipient(s)");
        else
            logger.error("SMTP delivery failed for '" + email.subject + "'");
        std::cout << "  => " << (sent ? "SENT" : "NOT SENT") << "\n";
    }

    try {
        Email noRecipient("billing@shop.com", {}, {}, {}, "Forgot the recipient", "", false,
                          common::Priority::Normal, {});
    } catch (const std::invalid_argument& error) {
        std::cout << "\nRejected email: " << error.what() << "\n";
    }
}

}
