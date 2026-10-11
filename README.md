# Design Patterns: Builder + Facade

This project solves one everyday problem twice: **sending an email from an application** (an invoice, a password reset, a newsletter).

- `WithPatterns/` solves it with the **Builder** and **Facade** patterns.
- `WithoutPatterns/` solves the same problem without them.

Both versions use the same low-level mail code in `Common/`, send the same three emails, and print exactly the same output. The only difference is how the code is organized, so the comparison is fair.

---

## The problem

Sending an email has two separate difficulties.

**1. Creating the email.** An email has many parts, and most of them are optional:

| Field | Required? |
|---|---|
| sender | yes |
| to / cc / bcc lists | at least one recipient |
| subject | yes |
| body, as plain text or HTML | optional |
| priority | optional (defaults to Normal) |
| attachments | optional (10 MB total limit) |

**2. Delivering the email.** Delivery goes through several subsystems that must be called in the right order:

```
AddressValidator  ->  check sender and every recipient
MimeEncoder       ->  begin, add headers (From, To, Cc, Subject, X-Priority), add body, add attachments, finish
SmtpConnection    ->  connect, STARTTLS, AUTH, MAIL FROM / RCPT TO / DATA, QUIT
MailLogger        ->  record success or failure
```

These subsystems (in `Common/MailSubsystems.*`) stand in for what a real mail library offers. They print what they do instead of opening real sockets.

---

## Project layout

```
DesignPatterns/
├── DesignPatterns.cpp            main(): runs both demos
├── Common/
│   └── MailSubsystems.h/.cpp     validator, MIME encoder, SMTP connection, logger (shared)
├── WithPatterns/
│   ├── Email.h/.cpp              the email object (can only be created by the builder)
│   ├── EmailBuilder.h/.cpp       BUILDER: step-by-step construction and validation
│   ├── EmailService.h/.cpp       FACADE: one send() call over all the subsystems
│   └── PatternDemo.h/.cpp        client code using the patterns
└── WithoutPatterns/
    ├── Email.h/.cpp              email struct with a 9-argument constructor
    └── PlainDemo.h/.cpp          client code driving every subsystem directly
```

## Build and run

Open `DesignPatterns.slnx` in Visual Studio and press **Ctrl+F5**. You can also build from a Developer Command Prompt:

```bash
msbuild DesignPatterns.slnx -p:Configuration=Debug -p:Platform=x64
```

```bash
x64\Debug\DesignPatterns.exe
```

---

## Part 1: Builder (creating the email)

### Without the pattern

The only way to create an `Email` is a constructor that takes every field:

```cpp
Email passwordReset("no-reply@shop.com",
                    {"bob@example.com"},
                    {},
                    {},
                    "Reset your password",
                    "Use this link to choose a new password.",
                    false,
                    common::Priority::Normal,
                    {});
```

Problems:

- **You can't tell what an argument means from the call.** What are the two `{}`, the `false`, and the last `{}`? You have to open `Email.h` and count positions.
- **Swapping arguments compiles without error.** `to`, `cc` and `bcc` are all `std::vector<std::string>`. If you swap `cc` and `bcc`, the code still compiles, and your customer's private address becomes visible to every recipient.
- **You must repeat the defaults.** A simple password-reset email still has to pass empty cc, empty bcc, `false`, `Priority::Normal` and an empty attachment list.
- **One rule is hidden in two arguments.** "HTML or plain text" is a `bool` placed separately from the body it describes.
- **Adding a field breaks every caller.** If you add `replyTo`, you must change every constructor call in the codebase, or add another overload, then another, and so on (the *telescoping constructor* problem).
- **The object can be changed later.** The fields are public, so any code can empty `to` after validation and break the email.

### With the Builder

```cpp
Email passwordReset = EmailBuilder()
                          .from("no-reply@shop.com")
                          .to("bob@example.com")
                          .subject("Reset your password")
                          .textBody("Use this link to choose a new password.")
                          .build();

Email invoice = EmailBuilder()
                    .from("billing@shop.com")
                    .to("alice@example.com")
                    .cc("accounts@example.com")
                    .subject("Invoice #1042")
                    .htmlBody("<h1>Thank you for your order</h1>")
                    .attach("invoice-1042.pdf", 48'200)
                    .priority(common::Priority::High)
                    .build();
```

How it works:

- `Email` has a **private constructor** and only getters. `EmailBuilder` is its `friend`, so an `Email` can only come from `build()`.
- Each builder method sets one field and returns `*this`, so the calls can be chained.
- `build()` checks the rules once (a sender, at least one recipient, a subject, attachments under 10 MB). It either returns a valid email or throws `std::invalid_argument`.

What gets easier:

| | Without Builder | With Builder |
|---|---|---|
| Readability | positional arguments, `{}` and `false` with no names | every value has a name: `.cc(...)`, `.htmlBody(...)` |
| Mixing up `to`/`cc`/`bcc` | compiles silently | impossible, because each method has its own name |
| Optional fields | must pass every default | you only write what you need |
| HTML vs text | separate `bool` | `.htmlBody()` / `.textBody()` sets both at once |
| Adding a new field (`replyTo`) | every call site changes | add one method; existing code doesn't change |
| Multiple recipients | build the vector yourself | call `.to()` / `.bcc()` again |
| After creation | public fields, can be broken | immutable, always valid |

---

## Part 2: Facade (delivering the email)

### Without the pattern

The client code has to know every subsystem and the exact SMTP order. In `WithoutPatterns/PlainDemo.cpp`, the body of the send loop is **52 lines**:

```cpp
common::SmtpSettings settings{...};
common::AddressValidator validator;
common::MimeEncoder encoder;
common::MailLogger logger;

for (const Email& email : {invoice, passwordReset, newsletter}) {
    // gather recipients, validate every address, log errors ...
    encoder.begin();
    encoder.addHeader("From", email.from);
    // To, Cc, Subject, X-Priority, body, attachments ...
    std::string data = encoder.finish();

    common::SmtpConnection smtp;
    bool sent = smtp.connect(settings.host, settings.port);
    if (sent) {
        if (settings.useTls)
            smtp.startTls();
        sent = smtp.authenticate(...) && smtp.sendMail(...);
        smtp.quit();
    }
    // log the result ...
}
```

Problems:

- **The client depends on four subsystem types** plus the SMTP settings and the order of the SMTP protocol.
- **The code gets copied.** A real app sends mail from many places: sign-up, password reset, billing, order shipping, newsletters. Each one needs the same 50 lines. Every copy can get the order wrong, for example forgetting `startTls()` (credentials go over the network unencrypted), forgetting `quit()` (connections leak), or leaving out `bcc` from the recipient list.
- **Changes spread across the codebase.** If you add a `Date` header, retry on failure, or switch from SMTP to an HTTP mail API like SendGrid, you must change every place that sends mail.
- **It's hard to test.** Business code such as "send invoice after payment" is mixed with socket-level details.

### With the Facade

```cpp
EmailService mail({"smtp.shop.com", 587, "mailer@shop.com", "app-password", true});

bool sent = mail.send(invoice);
```

`EmailService` (`WithPatterns/EmailService.cpp`) owns the subsystems and runs the steps in the right order: `addressesAreValid()`, then `encode()`, then `transmit()`, then logging. The client only sees `send(email)`.

What gets easier:

| | Without Facade | With Facade |
|---|---|---|
| Client code per email | ~50 lines | 1 line: `mail.send(email)` |
| Types the client must know | `AddressValidator`, `MimeEncoder`, `SmtpConnection`, `MailLogger`, `SmtpSettings` | `EmailService` |
| SMTP order (connect, TLS, auth, send, quit) | every caller must get it right | written once |
| Sending from a new place in the app | copy the whole block | call `send()` |
| Switching SMTP to an HTTP API | change every caller | change only `EmailService.cpp` |
| Unit testing business code | must simulate sockets | replace one service |

The subsystems are still available. A Facade does not hide them completely: code with unusual needs can still use `SmtpConnection` directly. The Facade just makes the common path simple.

---

## How the two patterns work together

Each pattern solves a different half of the problem:

```
            Builder                                Facade
 ┌──────────────────────────────┐   ┌─────────────────────────────────────────┐
 │ from/to/cc/subject/body/...  │──▶│ validate → MIME → SMTP session → log    │
 │ build() = always valid Email │   │ send(email) = one call                  │
 └──────────────────────────────┘   └─────────────────────────────────────────┘
        "WHAT to send"                          "HOW to send it"
```

The complete client code with both patterns:

```cpp
Email email = EmailBuilder().from(...).to(...).subject(...).textBody(...).build();
mail.send(email);
```

The Builder guarantees that `send()` always receives a structurally valid email, so the Facade doesn't need to check for a missing sender or an empty recipient list. It only checks what it is responsible for: whether the address format is valid and whether delivery succeeded.

---

## Overall comparison

| Aspect | Without patterns | With Builder + Facade |
|---|---|---|
| Creating an email | 9 positional arguments | named, chainable steps |
| Email after creation | mutable public struct | immutable, always valid |
| Sending an email | ~50 lines of subsystem calls | `mail.send(email)` |
| Knowledge needed by client code | every subsystem and the SMTP protocol | `EmailBuilder` and `EmailService` |
| Adding a field or a delivery step | edit every call site | edit one class |
| Risk of copy/paste bugs | high (TLS, quit, bcc) | logic exists in one place |
| Number of files/classes | fewer | more (`EmailBuilder`, `EmailService`) |

## When the patterns are *not* worth it

The patterns have costs, and the comparison should be honest about them:

- **More code up front.** The pattern version adds two classes (`EmailBuilder`, `EmailService`). If the program sends one kind of email from one place, the plain version is shorter and fine.
- **Builder is unnecessary for small objects.** For an object with two or three required fields and no optional ones, a normal constructor is clearer.
- **A Facade can grow into a "god object".** If `EmailService` also starts handling templates, unsubscribe lists and analytics, it should be split up.
- **The plain version naturally moves toward a Facade.** When you notice the 52 send lines are copied, the obvious fix is to move them into a function, which is the start of a Facade anyway. The pattern gives that refactoring a name and a standard shape.

## Sample output

Both demos print the following (shown once):

```
Sending 'Invoice #1042'
  [MIME] Encoded 6 headers, body and 1 attachment(s) into 377 bytes
  [SMTP] Connected to smtp.shop.com:587
  [SMTP] STARTTLS negotiated
  [SMTP] AUTH mailer@shop.com -> 235 OK
  [SMTP] MAIL FROM:<billing@shop.com>
  [SMTP] RCPT TO:<alice@example.com>
  [SMTP] RCPT TO:<accounts@example.com>
  [SMTP] DATA (377 bytes) -> 250 OK
  [SMTP] QUIT
  [Log] INFO: Sent 'Invoice #1042' to 2 recipient(s)
  => SENT

Sending 'Reset your password'
  ...
  => SENT

Sending 'October newsletter'
  [Log] ERROR: Invalid address 'dave-at-example.com', email not sent
  => NOT SENT

Rejected email: Email needs at least one recipient
```
