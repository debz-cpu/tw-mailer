# TW-Mailer

A socket-based client–server mail system written in C/C++ for Linux. Clients connect to the server over TCP and use a simple plain-text protocol to send, list, read and delete messages, similar to an internal company mail server.

The project is built in three stages as part of the *Distributed Systems (VERTS)* course at FH Technikum Wien:

1. **Basic**: iterative server, plain-text protocol, file-based mail spool
2. **Pro**: concurrent server, LDAP login, IP blacklisting, session handling
3. **Security**: TLS, encrypted mail storage, HMAC integrity checks, digital signatures, access control

## Features / Roadmap

### Basic Version
- [ ] TCP client and server using the POSIX Socket API
- [ ] Commands: `SEND`, `LIST`, `READ`, `DEL`, `QUIT`
- [ ] Per-user inbox persisted in a mail-spool directory
- [ ] Input validation (usernames: max. 8 chars, `a-z`, `0-9`; subject: max. 80 chars)
- [ ] Multi-line messages of any length, terminated by `.\n`
- [ ] Error handling with meaningful error messages
- [ ] Makefile with `all` and `clean` targets

### Pro Version
- [ ] Concurrent server (`fork()` or threads)
- [ ] Synchronization of critical sections (concurrent access to the mail spool)
- [ ] No zombie processes (`SIGCHLD` handling / `waitpid`)
- [ ] `LOGIN` command authenticated against LDAP (OpenLDAP C API)
- [ ] Max. 3 login attempts per user and IP, then IP blacklisted for 1 minute
- [ ] Persistent blacklist
- [ ] Sender and username taken from the session instead of being sent by the client

### Security Extension
- [ ] TLS-encrypted connection (OpenSSL) with a self-signed server certificate
- [ ] Client verifies the server certificate before communicating
- [ ] Encryption of stored messages (data at rest)
- [ ] Integrity check using HMAC
- [ ] Digital signatures to verify the sender of each message
- [ ] Access control: users can only access their own mails
- [ ] Mapping of all security measures to the CIA objectives (see docs)

## Tech Stack

| Area | Technology |
|---|---|
| Language | C / C++ (C++14) |
| Platform | Linux (developed in a Docker container) |
| Networking | POSIX Socket API (TCP stream sockets) |
| Concurrency | `fork()` / pthreads, <semaphores / mutexes / file locks> |
| Authentication | OpenLDAP C API (`libldap2-dev`) |
| Cryptography | OpenSSL (`libssl-dev`) |
| Build | GNU Make, g++ |

## Requirements

On Ubuntu / Debian:

```bash
sudo apt update
sudo apt install build-essential libldap2-dev libssl-dev
```

## Build

```bash
make        # builds twmailer-client and twmailer-server
make clean  # removes binaries and object files
```

## Usage

Start the server:

```bash
./twmailer-server <port> <mail-spool-directory>
# example
./twmailer-server 6543 ./mailspool
```

Start the client:

```bash
./twmailer-client <ip> <port>
# example
./twmailer-client 127.0.0.1 6543
```

## Protocol

All messages are plain text. Each field ends with a newline (`\n`). Multi-line message bodies end with a line containing only a dot (`.\n`).

### Basic Version

| Command | Client sends | Server responds |
|---|---|---|
| `SEND` | `SEND`, sender, receiver, subject, message lines, `.` | `OK` or `ERR` |
| `LIST` | `LIST`, username | message count, then one subject per line |
| `READ` | `READ`, username, message number | `OK` + full message, or `ERR` |
| `DEL` | `DEL`, username, message number | `OK` or `ERR` |
| `QUIT` | `QUIT` | no response, connection closes |

### Pro Version (after login)

Sender and username are taken from the session, so the client no longer sends them.

| Command | Client sends | Server responds |
|---|---|---|
| `LOGIN` | `LOGIN`, LDAP username, password | `OK` or `ERR` |
| `SEND` | `SEND`, receiver, subject, message lines, `.` | `OK` or `ERR` |
| `LIST` | `LIST` | message count, then one subject per line |
| `READ` | `READ`, message number | `OK` + full message, or `ERR` |
| `DEL` | `DEL`, message number | `OK` or `ERR` |
| `QUIT` | `QUIT` | no response |

All commands except `LOGIN` and `QUIT` require an authenticated session.

### Example session

```text
SEND
if24b001
if24b002
Meeting tomorrow
Hi, are we still meeting at 10?
See you.
.
→ OK
```

## Project Structure

```text
TW-Mailer/
├── src/
│   ├── client/        # twmailer-client
│   ├── server/        # twmailer-server
│   └── common/        # shared code (e.g. readline, protocol helpers)
├── docs/              # architecture & security documentation (PDF)
├── Makefile
├── LICENSE
└── README.md
```

## Architecture

<Short description once implemented, e.g.:>

- **Server:** listens on the given port; for each accepted connection it <forks a child process / starts a thread> that handles the session until `QUIT` or disconnect.
- **Mail spool:** one subfolder per user, one file per message: `<mailspool>/<username>/<message-id>`.
- **Synchronization:** <how concurrent writes to the same inbox and the blacklist are protected>.
- **Large messages:** <how messages of unlimited length are read and written, e.g. line-by-line streaming instead of one fixed buffer>.

## Security Design

<Fill in during the security stage. Justify each choice.>

| Measure | Algorithm / Technology | Confidentiality | Integrity | Availability |
|---|---|:-:|:-:|:-:|
| Transport encryption | TLS <version> (OpenSSL) | ✔ | ✔ | |
| Encryption at rest | <e.g. AES-256-GCM> | ✔ | ✔ | |
| Integrity check | HMAC-<SHA-256> | | ✔ | |
| Digital signatures | <e.g. Ed25519 / RSA-PSS> | | ✔ | |
| Access control | <e.g. DAC / RBAC> | ✔ | ✔ | |
| Login rate limiting / blacklist | 3 attempts → 1 min block | ✔ | | ✔ |

**Known limitations:** <e.g. self-signed certificate instead of a real PKI, public keys distributed manually, ...>

## Authors

- Kingsley Debrah – [@debz-cpu](https://github.com/debz-cpu)
- <Teammate name> – <GitHub link>

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
