#include "ssl_helper.h"
#include <openssl/err.h>
#include <iostream>

SSLHelper::SSLHelper()
{
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();

    m_context = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_default_verify_paths(m_context);
    SSL_CTX_set_verify(m_context, SSL_VERIFY_PEER, nullptr);

    m_ssl = SSL_new(m_context);

    if (m_ssl == nullptr)
    {
        logSSLError(); 
        throw std::runtime_error("Failed to create SSL");
    }
 }

SSL* SSLHelper::connect(int sock, const std::string& host)
{
    SSL_set_fd(m_ssl, sock);
    SSL_set_tlsext_host_name(m_ssl, host.c_str());
    SSL_set1_host(m_ssl, host.c_str());

    if (SSL_connect(m_ssl) <= 0)
    {
        logSSLError();
        throw std::runtime_error("Failed to connect using SSL");
    }

    return m_ssl;
}

int SSLHelper::read(void *buf, int num)
{
    return SSL_read(m_ssl, buf, num);
}

int SSLHelper::write(const void *buf, int num)
{
    return SSL_write(m_ssl, buf, num); 
}

void SSLHelper::shutdown()
{
    SSL_shutdown(m_ssl);
    SSL_free(m_ssl);
    SSL_CTX_free(m_context);
}

void SSLHelper::logSSLError()
{
    int error;
    while ((error = ERR_get_error()))
    {
        std::string errStr = ERR_error_string(error, 0);
        std::cerr << "Error: " << errStr << "\n";
    }
}