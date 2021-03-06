/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef ABSTRACTPANELMANAGER_H
#define ABSTRACTPANELMANAGER_H

// QtCore
#include <QUrl>

class KrPanel;

class AbstractPanelManager
{
public:
    virtual ~AbstractPanelManager() {}
    virtual bool isLeft() const = 0;
    virtual AbstractPanelManager *otherManager() const = 0;
    virtual KrPanel *currentPanel() const = 0;
    virtual void newTab(const QUrl&, KrPanel *nextTo = 0) = 0;
};


#endif // ABSTRACTPANELMANAGER_H
