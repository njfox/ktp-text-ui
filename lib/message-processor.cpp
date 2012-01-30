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

QList<AbstractMessageFilter*> MessageProcessor::m_filters;

AbstractMessageFilter::~AbstractMessageFilter()
{

}


MessageProcessor::MessageProcessor(QObject* parent): QObject(parent)
{
    m_filters << new UrlFilter();
}

KTp::Message MessageProcessor::processIncommingMessage(Tp::ReceivedMessage receivedMessage)
{
    KTp::Message message(receivedMessage);
    Q_FOREACH(AbstractMessageFilter *filter, MessageProcessor::m_filters) {
        filter->filterMessage(message);
    }
    return message;
}

MessageProcessor::~MessageProcessor()
{

}
