/***************************************************************************
                          dirhistorybutton.h  -  description
                             -------------------
    begin                : Sun Jan 4 2004
    copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
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

#ifndef DIRHISTORYBUTTON_H
#define DIRHISTORYBUTTON_H

#include <qwidget.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <kurl.h>

class Q3PopupMenu;
class DirHistoryQueue;

/**
  *@author Shie Erlich & Rafi Yanai
  */

class DirHistoryButton : public QToolButton  {
   Q_OBJECT
public: 
  DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent=0);
  ~DirHistoryButton();

  void openPopup();

private:
  Q3PopupMenu* popupMenu;
  DirHistoryQueue* historyQueue;
  
public slots: // Public slots
  /** No descriptions */
  void slotPopup();
  /** No descriptions */
  void slotAboutToShow();
  /** No descriptions */
  void slotPopupActivated(int id);
signals: // Signals
  /** No descriptions */
  void openUrl(const KUrl&);
};

#endif
