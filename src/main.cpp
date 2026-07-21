#include "browser_view.h"

#include "browser_scene.h"
#include "constants.h"

#include <QApplication>
#include <QStyleHints>

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    // Force the application to use light mode.
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    BrowserScene scene;
    scene.setSceneRect(0, 0, Constants::WIDTH + 15, Constants::HEIGHT);

    BrowserView view(&scene);
    view.show();
    view.centerOn(0, 0);
    scene.load("https://browser.engineering/styles.html");
    //scene.load("http://localhost:8000/index.html");

    return app.exec();
}
