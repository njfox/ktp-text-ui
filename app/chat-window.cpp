/*
    Copyright (C) 2013  Daniel Cohen    <analoguecolour@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "chat-window.h"
#include "chatTabWidget.h"
#include "kxmlguiclient.h"

#include <kservice.h>
#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KDebug>

#include <KParts/PartManager>

ChatWindow::ChatWindow() :
    KParts::MainWindow(0, 0),
    m_partManager(new KParts::PartManager(this))
{
    setXMLFile(QLatin1String("chat-window.rc"));
//     setupActions();

    connect( m_partManager, SIGNAL( activePartChanged( KParts::Part * ) ),
            this, SLOT( createGUI( KParts::Part * ) ) );

    // Creates the GUI with a null part to make appear the main app menus and tools
    createGUI(0);
    setupGUI();

    currTab = 0;
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::setupWindow()
{
   partTabWidget->setDocumentMode(true);
   setCentralWidget(partTabWidget);
   QObject::connect(partTabWidget, SIGNAL(tabCloseRequested(int)), partTabWidget, SLOT(removeTab(int)));
   show();
   connect(partTabWidget, SIGNAL(currentChanged(int)), SLOT(onTabStateChanged()));
}

void ChatWindow::addTab(QVariantList args, QString channelalias)
{
    KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("KTpTextChatPart.desktop"));
    KTpTextChatPart* part = static_cast<KTpTextChatPart*>(service->createInstance<KParts::Part>(this,  args));
    Q_ASSERT(part);
    createGUI(part);

    m_partManager->addPart(part, true);
    m_tabsPartsMap[part->widget()] = part;
    partTabWidget->addTab(part->widget(), channelalias);
    setupActions(part);
}

void ChatWindow::setupActions(KTpTextChatPart* part)
{
    ChatTabWidget* widget = static_cast<ChatTabWidget*>(part->widget());

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(showNotificationsDialog()), actionCollection());

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    m_zoomFactor = group.readEntry("zoomFactor", (qreal) 1.0);
    widget->setZoomFactor(m_zoomFactor);
    KStandardAction::zoomIn(this, SLOT(onZoomIn()), actionCollection());
    KStandardAction::zoomOut(this, SLOT(onZoomOut()), actionCollection());
    connect(widget, SIGNAL(zoomFactorChanged(qreal)), this, SLOT(onZoomFactorChanged(qreal)));
    kDebug() << "startstart" << partTabWidget->currentWidget() << "end";
    kDebug() << "childverifyfirst" << childClients();
}

void ChatWindow::onTabStateChanged()
{
    kDebug() << "onTabStateChanged is being called";
//     kDebug() << "startremove" << partTabWidget->widget(currTab) << "end";
//     ChatTabWidget* prevWidget = static_cast<ChatTabWidget*>(partTabWidget->widget(currTab));
//     kDebug() << "starttt" << currTab << "end";
//     kDebug() << "start" << prevWidget << "end";
//     if (prevWidget != 0)
//     {
//         removeChildClient(prevWidget);
//     }
//     kDebug() << "childverifyremove" << childClients();
//     KTpTextChatPart* widget = static_cast<KTpTextChatPart*>(partTabWidget->currentWidget()->parent());
//     m_partManager->setSelectedPart(widget);

    qDebug() << m_tabsPartsMap;
    qDebug() << partTabWidget->currentWidget();

    m_partManager->setActivePart(m_tabsPartsMap[partTabWidget->currentWidget()]);

//     insertChildClient(widget);
//     currTab = partTabWidget->currentIndex();
    kDebug() << "childverifyadd" << childClients();
}

void ChatWindow::onZoomIn()
{
    onZoomFactorChanged(m_zoomFactor + 0.1);
}

void ChatWindow::onZoomOut()
{
    onZoomFactorChanged(m_zoomFactor - 0.1);
}

void ChatWindow::onZoomFactorChanged(qreal zoom)
{
    m_zoomFactor = zoom;

    for (int i = 0; i < partTabWidget->count(); i++) {
        ChatTabWidget* widget = static_cast<ChatTabWidget*>(partTabWidget->widget(i));
        if (!widget) {
            continue;
        }
        widget->setZoomFactor(zoom);
    }

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    group.writeEntry("zoomFactor", m_zoomFactor);
}

#include "chat-window.moc"
