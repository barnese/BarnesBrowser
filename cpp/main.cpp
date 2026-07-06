#include "url.h"
#include "constants.h"
#include <iostream>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

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

int main(int argc, char *argv[]) 
{
    //load("http://example.org/");
    //load("https://browser.engineering/http.html");
    //load("http://localhost:8000/");

    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(0, 0, Constants::WIDTH, 10000);
    scene.addText("Hello!")->setPos(100, 200);

    QGraphicsView view(&scene);
    view.resize(Constants::WIDTH, Constants::HEIGHT);
    view.show();
    view.centerOn(0, 0);

    return app.exec();
}
