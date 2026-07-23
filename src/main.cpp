#include "browser.h"
#include "url.h"

#include <QApplication>
#include <QStyleHints>

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    // Force the application to use light mode.
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    Browser browser;
    browser.resize(815, 600);
    browser.newTab(URL("https://browser.engineering/styles.html"));
    browser.show();
    
    return app.exec();
}
