#include "Email.h"

namespace patterns {

std::vector<std::string> Email::allRecipients() const
{
    std::vector<std::string> result = to_;
    result.insert(result.end(), cc_.begin(), cc_.end());
    result.insert(result.end(), bcc_.begin(), bcc_.end());
    return result;
}

std::vector<std::string> Email::allAddresses() const
{
    std::vector<std::string> result{from_};
    auto recipients = allRecipients();
    result.insert(result.end(), recipients.begin(), recipients.end());
    return result;
}

}
