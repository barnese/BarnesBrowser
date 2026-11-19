#include "chrome.h"

#include "helpers/file_helpers.h"

#include <QKeyEvent>
#include <QVBoxLayout>

Chrome::Chrome(QWidget* parent) : QWidget(parent) 
{
    m_font = new QFont("Arial", 20);

    createTabBar();
    createAddressBar();

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(m_tabBarLayout);
    layout->addLayout(m_addressBarLayout);
    layout->setContentsMargins(10, 8, 10, 0);
    layout->setSpacing(0);
}

void Chrome::createTabBar()
{
    m_tabBarLayout = new QHBoxLayout();
    m_tabBarLayout->setContentsMargins(0, 0, 0, 4);

    m_addTabButton = new QPushButton("+");
    m_addTabButton->setFont(*m_font);
    m_addTabButton->setProperty("class", "navButton");

    m_tabBar = new QTabBar(this);
    // Tabs are fixed-width, so elide long titles in the space
    // left of the close button rather than drawing under it.
    m_tabBar->setElideMode(Qt::ElideRight);
    m_tabBar->setExpanding(false);

    m_tabBarLayout->addWidget(m_addTabButton);
    m_tabBarLayout->addWidget(m_tabBar);
    m_tabBarLayout->addStretch();
    m_tabBarLayout->setSpacing(10);

    connect(m_addTabButton, &QPushButton::clicked, this, &Chrome::newTabRequested);
    connect(m_tabBar, &QTabBar::tabBarClicked, this, &Chrome::handleTabChanged);
}

void Chrome::createAddressBar()
{
    m_addressBarLayout = new QHBoxLayout();

    m_backButton = new QPushButton("←");
    m_backButton->setFont(*m_font);
    m_backButton->setProperty("class", "navButton");

    m_addressBar = new QLineEdit(this);
    m_addressBar->setFont(*m_font);
    m_addressBar->setFocusPolicy(Qt::ClickFocus);
    // Catch browser shortcuts (e.g. Command+W) while the address bar has focus,
    // since BrowserView only receives key events when the page is focused.
    m_addressBar->installEventFilter(this);

    m_addressBarLayout->addWidget(m_backButton);
    m_addressBarLayout->addWidget(m_addressBar);
    m_addressBarLayout->setSpacing(10);

    connect(m_backButton, &QPushButton::clicked, this, &Chrome::navigateBackRequested);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &Chrome::handleReturnPressed);
}

bool Chrome::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_addressBar && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        // Qt maps ControlModifier to the Command key on macOS.
        if ((keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_W)
        {
            emit closeActiveTabRequested();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Chrome::handleTabChanged(int index)
{
    // tabBarClicked reports -1 when the click misses every tab.
    if (index < 0) return;
    restoreAddress(index);
    tabChanged(index);
}

void Chrome::handleCloseButtonClicked(QPushButton* button)
{
    // Look the index up at click time, since closing other
    // tabs shifts the indices of the tabs after them.
    for (int i = 0; i < m_tabBar->count(); i++)
    {
        if (m_tabBar->tabButton(i, QTabBar::RightSide) == button)
        {
            emit tabCloseRequested(i);
            return;
        }
    }
}

void Chrome::restoreAddress(int index)
{
    if (index < 0 || index >= m_tabBar->count()) return;
    setAddress(m_tabBar->tabData(index).toString());
}

void Chrome::handleReturnPressed()
{
    addressChanged(m_addressBar->text());
}

void Chrome::addTab(QString tabName, QString address, size_t index)
{
    int tabIndex = m_tabBar->addTab(tabName);
    m_tabBar->setTabData(tabIndex, address);

    auto* closeButton = new QPushButton("✕");
    closeButton->setProperty("class", "tabCloseButton");
    closeButton->setFixedSize(24, 18);
    closeButton->setFocusPolicy(Qt::NoFocus);
    closeButton->setAccessibleName("Close tab");
    closeButton->setToolTip("Close tab (⌘W)");
    connect(closeButton, &QPushButton::clicked, this, [this, closeButton]() {
        handleCloseButtonClicked(closeButton);
    });
    m_tabBar->setTabButton(tabIndex, QTabBar::RightSide, closeButton);

    setAddress(address);
    m_tabBar->setCurrentIndex(index);
}

void Chrome::setTabAddress(size_t index, QString address)
{
    if (index >= static_cast<size_t>(m_tabBar->count())) return;
    m_tabBar->setTabData(index, address);
    if (static_cast<int>(index) == m_tabBar->currentIndex())
    {
        setAddress(address);
    }
}

void Chrome::removeTab(size_t index)
{
    if (index >= static_cast<size_t>(m_tabBar->count())) return;
    m_tabBar->removeTab(index);
}

void Chrome::setAddress(QString address)
{
    auto newTabHtmlPath = getNewTabHtmlFilePath();

    // Don't show the URL in the address bar for the New Tab HTML page.
    m_addressBar->setText(address == newTabHtmlPath ? "" : address);
}

void Chrome::setTabName(QString tabName, size_t index)
{
    m_tabBar->setTabText(index, tabName);
}

void Chrome::focusAddressBar()
{
    m_addressBar->setFocus();
}

void Chrome::setActiveTab(size_t index)
{
    m_tabBar->setCurrentIndex(index);
    restoreAddress(index);
}
