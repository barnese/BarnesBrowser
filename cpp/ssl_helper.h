#pragma once

#include <string>
#include <openssl/ssl.h>

class SSLHelper
{
    public:
        SSL* initWithSocket(int sock, const std::string& host);
        int read(void *buf, int num);
        int write(const void *buf, int num);
        void shutdown();
    
    private:
        SSL* m_ssl;
        SSL_CTX* m_context;
};
