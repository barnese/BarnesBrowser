#include "url.h"
#include <iostream>

void show(std::string body)
{
    bool inTag = false;
    for (char c : body) 
    {
        if (c == '<')
            inTag = true;
        else if (c == '>')
            inTag = false;
        else if (!inTag) 
            std::cout << c;
    }
}

void load(std::string urlString)
{
    URL url(urlString);
    auto body = url.request();
    show(body);

}

int main() 
{
    load("https://browser.engineering/http.html");
    return 0;
}

