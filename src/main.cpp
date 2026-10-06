#include "browser.h"

#include "networking/url.h"

#include <QApplication>
#include <QFile>
#include <QStyleHints>
#include <QTextStream>

/// Loads style sheets used by Qt to style the UI.
void loadStyleSheets(QApplication* app)
{
    QFile file("assets/styles/chrome.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app->setStyleSheet(styleSheet);
        file.close();
    }
}

/// Main entry point.
int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    // Force the application to use light mode.
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    loadStyleSheets(&app);

    Browser browser;
    browser.resize(815, 600);
    browser.newTab();
    browser.show();
    
    return app.exec();
}
