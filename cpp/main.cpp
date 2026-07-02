#include <iostream>
#include "helpers.h"

class URL {
  public:
    URL(const std::string& url)
    {
      auto parts = split(url, "://");

      m_scheme = parts[0];
      m_url = parts[1];
    }

    void describe() const
    {
      std::cout << "url = " << m_url << "\n";
      std::cout << "scheme = " << m_scheme << "\n";
    }

  private:
    std::string m_url;
    std::string m_scheme;
};

int main() {
  URL url("https://www.apple.com/");
  url.describe();

  return 0;
}

