#include "ssl_helper.h"

SSL* SSLHelper::initWithSocket(int sock, const std::string& host)
{
    m_context = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_default_verify_paths(m_context);
    SSL_CTX_set_verify(m_context, SSL_VERIFY_PEER, nullptr);

    m_ssl = SSL_new(m_context);

    if (m_ssl == nullptr) 
        throw std::runtime_error("Failed to create SSL");
    
    SSL_set_fd(m_ssl, sock);
    SSL_set_tlsext_host_name(m_ssl, host.c_str());
    SSL_set1_host(m_ssl, host.c_str());

    if (SSL_connect(m_ssl) != 1)
        throw std::runtime_error("Failed to connect using SSL");

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