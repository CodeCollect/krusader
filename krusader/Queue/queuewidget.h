/*****************************************************************************
 * Copyright (C) 2008-2009 Csaba Karai <cskarai@freemail.hu>                 *
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

#ifndef QUEUEWIDGET_H
#define QUEUEWIDGET_H

#include <QtCore/QPointer>

// TODO KF5 - these headers are from deprecated KDE4LibsSupport : remove them
#include <KDE/KTabWidget>

#include "queue.h"
#include "../GUI/krlistwidget.h"

class KrQueueListWidget;

class QueueWidget: public KTabWidget
{
    Q_OBJECT
public:
    QueueWidget(QWidget * parent = 0);
    ~QueueWidget();

    void deleteCurrent();

protected slots:
    void slotQueueAdded(Queue *);
    void slotQueueDeleted(Queue *);
    void slotCurrentChanged(Queue *);
    void slotCurrentChanged(int);

signals:
    void currentChanged();

private:
    QMap<QString, KrQueueListWidget*> _queueWidgets;
};

class KrQueueListWidget : public KrListWidget
{
    Q_OBJECT
public:
    KrQueueListWidget(Queue * queue, QWidget * parent);
    Queue * queue() {
        return _queue;
    }
    void deleteItem(QListWidgetItem * item);

public slots:
    void slotChanged();
    void slotItemRightClicked(QListWidgetItem *);

signals:
    void stateChanged();

private:
    QPointer<Queue> _queue;
};


#endif // QUEUE_WIDGET_H
