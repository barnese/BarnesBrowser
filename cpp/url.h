#pragma once

#include <string>

class URL
{
    public:
        URL(const std::string& url);
        void request() const;
        void describe() const;
    
    private:
        std::string m_host;
        std::string m_path;
        std::string m_scheme;
        uint16_t m_port;
};