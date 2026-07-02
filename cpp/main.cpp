#include "url.h"

int main() 
{
    URL url("https://browser.engineering/http.html");
    url.describe();
    url.request();

    return 0;
}

