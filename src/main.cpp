#include "browser_view.h"
#include "browser_scene.h"
#include "constants.h"
#include "html_tree/html_parser.h"
#include "url.h"

#include <QApplication>
#include <QStyleHints>

int testHTMLParser() 
{
    std::string url = "http://localhost:8000/index.html";
    //std::string url = "http://localhost:8000/simple_invalid.html";
    std::string body = URL(url).request();
    auto parser = HTMLParser(body);
    auto rootNode = parser.parse();
    parser.printTree(rootNode.get());
    return 0;
}

int runBrowser(int argc, char* argv[])
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

int main(int argc, char *argv[]) 
{
    return runBrowser(argc, argv);
    //return testHTMLParser();
}
