/*
 * Contact Delegate - compact version
 *
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2012 Dominik Cermak <d.cermak@arcor.de>
 * Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "contact-delegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <QApplication>
#include <QStyle>
#include <QHelpEvent>
#include <QFontDatabase>
#include <QStyleOptionViewItem>

#include <KIconLoader>

#include <KTp/types.h>

#include "channel-contact-model.h"

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_spacing = 4;
    m_avatarSize = IconSize(KIconLoader::Toolbar);
    m_presenceIconSize = IconSize(KIconLoader::Small);
    m_clientTypeIconSize = IconSize(KIconLoader::Small);
}

ContactDelegate::~ContactDelegate()
{

}

void ContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(opt.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter);

    QRect iconRect = opt.rect;
    iconRect.setSize(QSize(m_avatarSize, m_avatarSize));
    iconRect.moveTo(QPoint(iconRect.x() + m_spacing, iconRect.y() + m_spacing));

    QPixmap avatar;

    if (index.data(ChannelContactModel::IsTypingRole).toBool()) {
        avatar = QIcon::fromTheme(QStringLiteral("document-edit")).pixmap(KIconLoader::SizeSmallMedium);
    } else {
        avatar = qvariant_cast<QPixmap>(index.data(KTp::ContactAvatarPixmapRole));
    }

    if (!avatar.isNull()) {
        style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, avatar.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // This value is used to set the correct width for the username and the presence message.
    int rightIconsWidth = m_presenceIconSize + m_spacing;

    QPixmap icon = QIcon::fromTheme(index.data(KTp::ContactPresenceIconRole).toString()).pixmap(KIconLoader::SizeSmallMedium);

    QRect statusIconRect = opt.rect;

    statusIconRect.setSize(QSize(m_presenceIconSize, m_presenceIconSize));
    statusIconRect.moveTo(QPoint(opt.rect.right() - (rightIconsWidth),
                                 opt.rect.top() + (opt.rect.height() - m_presenceIconSize) / 2));

    painter->drawPixmap(statusIconRect, icon);

    // Right now we only check for 'phone', as that's the most interesting type.
    if (index.data(KTp::ContactClientTypesRole).toStringList().contains(QLatin1String("phone"))) {
        // Additional space is needed for the icons, don't add too much spacing between the two icons
        rightIconsWidth += m_clientTypeIconSize + m_spacing;

        QPixmap phone = QIcon::fromTheme(QLatin1String("phone")).pixmap(m_clientTypeIconSize);
        QRect phoneIconRect = opt.rect;
        phoneIconRect.setSize(QSize(m_clientTypeIconSize, m_clientTypeIconSize));
        phoneIconRect.moveTo(QPoint(opt.rect.right() - rightIconsWidth,
                                    opt.rect.top() + (opt.rect.height() - m_clientTypeIconSize) / 2));
        painter->drawPixmap(phoneIconRect, phone);
    }

    QFont nameFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    const QFontMetrics nameFontMetrics(nameFont);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->setFont(nameFont);

    QRect userNameRect = opt.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + m_spacing * 2);
    userNameRect.setY(userNameRect.y() + (userNameRect.height()/2 - nameFontMetrics.height()/2));
    userNameRect.setWidth(userNameRect.width() - rightIconsWidth);

    QString nameText = index.data(Qt::DisplayRole).toString();

    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(nameText, Qt::ElideRight, userNameRect.width()));

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(0, qMax(m_avatarSize + 2 * m_spacing, QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont).pixelSize() + m_spacing));
}
