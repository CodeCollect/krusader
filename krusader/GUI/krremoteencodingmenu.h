/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <cskarai@freemail.hu>                      *
 * based on KRemoteEncodingPlugin from Dawit Alemayehu <adawit@kde.org>      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRREMOTEENCODINGMENU_H
#define KRREMOTEENCODINGMENU_H

#include <QtCore/QStringList>
#include <QtWidgets/QAction>

# include <KWidgetsAddons/KActionMenu>

class KActionCollection;

class KrRemoteEncodingMenu: public KActionMenu
{
    Q_OBJECT
public:
    KrRemoteEncodingMenu(const QString &text, const QString &icon, KActionCollection *parent = 0);

protected slots:

    void slotAboutToShow();

    void slotReload();
    void slotTriggered(QAction *);

    virtual void chooseDefault();
    virtual void chooseEncoding(QString encoding);

protected:
    virtual QString currentCharacterSet();

private:

    void loadSettings();
    void updateKIOSlaves();

    QStringList encodingNames;
    bool        settingsLoaded;
};

#endif /* REMOTEENCODING_MENU_H */
