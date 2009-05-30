/***************************************************************************
                   krinterviewitemdelegate.h  -  description
                             -------------------
    begin                : Sat Feb 14 2009
    copyright            : (C) 2009+ by Csaba Karai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KR_INTERVIEW_ITEM_DELEGATE__
#define __KR_INTERVIEW_ITEM_DELEGATE__

#include <QItemDelegate>

class KrInterViewItemDelegate : public QItemDelegate
{
public:
	KrInterViewItemDelegate( QObject *parent = 0 );
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void drawDisplay ( QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text ) const;
	QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	bool eventFilter(QObject *object, QEvent *event);
	
private:
	mutable int _currentlyEdited;
	mutable bool _dontDraw;
};

#endif // __KR_INTERVIEW_ITEM_DELEGATE__