#pragma once

#include <string>
#include <openssl/ssl.h>

class SSLHelper
{
    public:
        SSLHelper();
        SSL* connect(int sock, const std::string& host);
        int read(void *buf, int num);
        int write(const void *buf, int num);
        void shutdown();
    
    private:
        void logSSLError();

        SSL* m_ssl;
        SSL_CTX* m_context;
};
