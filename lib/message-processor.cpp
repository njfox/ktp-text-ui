/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "message-processor.h"
#include "filters.h"

#include <QMutex>

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginFactory>

MessageProcessor* MessageProcessor::s_instance = 0;
static const QString serviceType = QLatin1String("KTpTextUi/MessageFilter");

MessageProcessor* MessageProcessor::instance()
{
    kDebug();

    static QMutex mutex;
    if (!s_instance)
    {
        mutex.lock();
        if (!s_instance) {
            s_instance = new MessageProcessor;
        }
        mutex.unlock();
    }
    return s_instance;
}


MessageProcessor::MessageProcessor()
{
    m_filters.append(new EscapeFilter(this));
    m_filters.append(new UrlFilter(this));

    loadPlugins();
}


MessageProcessor::~MessageProcessor()
{
}

Message MessageProcessor::processIncomingMessage(const Tp::ReceivedMessage &receivedMessage)
{
    Message message(receivedMessage);
    Q_FOREACH(AbstractMessageFilter *filter, MessageProcessor::m_filters) {
        kDebug() << "running filter :" << filter->metaObject()->className();
        filter->filterMessage(message);
    }
    return message;
}

Message MessageProcessor::processOutgoingMessage(const Tp::Message &sentMessage)
{
    Message message(sentMessage);
    Q_FOREACH(AbstractMessageFilter *filter, MessageProcessor::m_filters) {
        filter->filterMessage(message);
    }
    return message;
}

void MessageProcessor::loadPlugins() {
    kDebug() << "Starting loading filters...";

    Q_FOREACH (const KPluginInfo &plugin, pluginList()) {
        kDebug() << "found filter :" << plugin.pluginName();

        if (plugin.isPluginEnabled()) {
            kDebug() << "it is enabled";
            KService::Ptr service = plugin.service();

            KPluginFactory *factory = KPluginLoader(service->library()).factory();
            if(factory) {
                kDebug() << "loaded factory :" << factory;
                AbstractMessageFilter *filter = factory->create<AbstractMessageFilter>(this);

                if(filter) {
                    kDebug() << "loaded message filter : " << filter;
                    m_filters.append(filter);
                }
            } else {
                kError() << "error loading plugin :" << service->library();
            }
        }
    }
}

KPluginInfo::List MessageProcessor::pluginList()
{
    KService::List offers = KServiceTypeTrader::self()->query(serviceType);
    KConfigGroup config = KConfig(QLatin1String("ktelepathyrc")).group("Filters");

    return KPluginInfo::fromServices(offers, config);
}
