#include "PatternDemo.h"

#include "EmailBuilder.h"
#include "EmailService.h"

#include <iostream>
#include <stdexcept>

namespace patterns {

void runDemo()
{
    std::cout << "================ WITH DESIGN PATTERNS (Builder + Facade) ================\n";

    EmailService mail({"smtp.shop.com", 587, "mailer@shop.com", "app-password", true});

    Email invoice = EmailBuilder()
                        .from("billing@shop.com")
                        .to("alice@example.com")
                        .cc("accounts@example.com")
                        .subject("Invoice #1042")
                        .htmlBody("<h1>Thank you for your order</h1>")
                        .attach("invoice-1042.pdf", 48'200)
                        .priority(common::Priority::High)
                        .build();

    Email passwordReset = EmailBuilder()
                              .from("no-reply@shop.com")
                              .to("bob@example.com")
                              .subject("Reset your password")
                              .textBody("Use this link to choose a new password.")
                              .build();

    Email newsletter = EmailBuilder()
                           .from("news@shop.com")
                           .bcc("carol@example.com")
                           .bcc("dave-at-example.com")
                           .subject("October newsletter")
                           .htmlBody("<p>New arrivals are here</p>")
                           .priority(common::Priority::Low)
                           .build();

    for (const Email& email : {invoice, passwordReset, newsletter}) {
        std::cout << "\nSending '" << email.subject() << "'\n";
        bool sent = mail.send(email);
        std::cout << "  => " << (sent ? "SENT" : "NOT SENT") << "\n";
    }

    try {
        EmailBuilder()
            .from("billing@shop.com")
            .subject("Forgot the recipient")
            .build();
    } catch (const std::invalid_argument& error) {
        std::cout << "\nRejected email: " << error.what() << "\n";
    }
}

}
