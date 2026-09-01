#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTabBar>
#include <QWidget>

class Chrome : public QWidget
{
    Q_OBJECT

public:
    Chrome(QWidget* parent = nullptr);
    void addTab(QString tabName, QString address, size_t index);
    void setAddress(QString address);
    void focusAddressBar();
    void setActiveTab(size_t index);

signals:
    void newTabRequested();
    void tabChanged(int index);
    void addressChanged(QString newAddress);
    void navigateBackRequested();

private:
    void createTabBar();
    void createAddressBar();

    void handleTabChanged(int index);
    void handleReturnPressed();

    QHBoxLayout* m_tabBarLayout;
    QHBoxLayout* m_addressBarLayout;
    QPushButton* m_addTabButton;
    QPushButton* m_backButton;
    QLineEdit* m_addressBar;
    QTabBar* m_tabBar;
    QFont* m_font;
};
