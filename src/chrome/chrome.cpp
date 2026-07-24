#include "chrome.h"

#include <QVBoxLayout>
#include <iostream>

Chrome::Chrome(QWidget* parent) : QWidget(parent) 
{
    m_font = new QFont("Arial", 20);

    createTabBar();
    createAddressBar();

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(m_tabBarLayout);
    layout->addLayout(m_addressBarLayout);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);
}

void Chrome::createTabBar()
{
    m_tabBarLayout = new QHBoxLayout();

    m_addTabButton = new QPushButton("+");
    m_addTabButton->setFont(*m_font);

    m_tabBar = new QTabBar(this);

    m_tabBarLayout->addWidget(m_addTabButton);
    m_tabBarLayout->addWidget(m_tabBar);
    m_tabBarLayout->addStretch();
    m_tabBarLayout->setSpacing(10);

    connect(m_addTabButton, &QPushButton::clicked, this, &Chrome::newTabRequested);
    connect(m_tabBar, &QTabBar::currentChanged, this, &Chrome::handleTabChanged);
}

void Chrome::createAddressBar()
{
    m_addressBarLayout = new QHBoxLayout();

    m_backButton = new QPushButton("<");
    m_backButton->setFont(*m_font);

    m_addressBar = new QLineEdit(this);
    m_addressBar->setFont(*m_font);

    m_addressBarLayout->addWidget(m_backButton);
    m_addressBarLayout->addWidget(m_addressBar);
    m_addressBarLayout->setSpacing(10);

    connect(m_addressBar, &QLineEdit::returnPressed, this, &Chrome::handleReturnPressed);
}

void Chrome::handleTabChanged(int index)
{
    tabChanged(index);
}

void Chrome::handleReturnPressed()
{
    addressChanged(m_addressBar->text());
}

void Chrome::addTab(QString tabName, QString address, size_t index)
{
    m_tabBar->addTab(tabName);
    m_addressBar->setText(address);
    m_tabBar->setCurrentIndex(index);
}

void Chrome::setAddress(QString address)
{
    m_addressBar->setText(address);
}
