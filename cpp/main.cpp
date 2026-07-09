#include "browser_view.h"
#include "browser_scene.h"
#include "constants.h"
#include <QApplication>

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    BrowserScene scene;
    scene.setSceneRect(0, 0, Constants::WIDTH + 15, Constants::HEIGHT);

    BrowserView view(&scene);
    view.show();
    view.centerOn(0, 0);
    //scene.load("https://browser.engineering/examples/xiyouji.html");
    scene.load("https://browser.engineering/graphics.html");
    //scene.load("https://browser.engineering/examples/example1-simple.html");
    //scene.load("http://localhost:8000/fonts.html");

    return app.exec();
}
