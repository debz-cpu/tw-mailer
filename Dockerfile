FROM ubuntu:24.04

# Compiler, make, debugger, leak checker, LDAP + OpenSSL (for later), netcat (for testing)
RUN apt-get update && apt-get install -y \
    build-essential gdb valgrind \
    libldap2-dev libssl-dev \
    netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app