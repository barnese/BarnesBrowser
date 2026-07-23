#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabBar>
#include <QWidget>

class Chrome : public QWidget
{
public:
    Chrome(QWidget* parent = nullptr);

private:
    void createTabBar();
    void createAddressBar();

    void handleTabChanged(int index);

    QHBoxLayout* m_tabBarLayout;
    QHBoxLayout* m_addressBarLayout;
    QPushButton* m_addTabButton;
    QPushButton* m_backButton;
    QLineEdit* m_addressBar;
    QTabBar* m_tabBar;
    QFont* m_font;
};
