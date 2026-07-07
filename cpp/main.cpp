#include "url.h"
#include "constants.h"
#include <iostream>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

/// Gets the textual content of an HTML document.
std::string lex(std::string body)
{
    std::string text;
    bool inTag = false;

    for (char c : body) 
    {
        if (c == '<')
            inTag = true;
        else if (c == '>')
            inTag = false;
        else if (!inTag) 
            text += c;
    }

    return text;
}

/// Loads the HTML page from the given URL.
void load(QGraphicsScene* scene, std::string urlString)
{
    try
    {
        URL url(urlString);
        auto body = url.request();
        auto content = lex(body);

        int cursorX = Constants::HSTEP;
        int cursorY = Constants::VSTEP;

        QString text = QString::fromUtf8(content.data(), content.size());

        for (QChar c : text)
        {
            scene->addText(c)->setPos(cursorX, cursorY);
            cursorX += Constants::HSTEP;

            if (cursorX >= Constants::WIDTH - Constants::HSTEP)
            {
                cursorY += Constants::VSTEP;
                cursorX = Constants::HSTEP;
            }
        }
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
    }
}

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(0, 0, Constants::WIDTH, 10000);

    QGraphicsView view(&scene);
    view.resize(Constants::WIDTH, Constants::HEIGHT);
    view.show();
    view.centerOn(0, 0);

    std::string url = "https://browser.engineering/examples/xiyouji.html";
    load(&scene, url);

    return app.exec();
}
