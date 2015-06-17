/***************************************************************************
                      filterdialog.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filterdialog.h"
#include "filtertabs.h"
#include "generalfilter.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>

#include <KI18n/KLocalizedString>

FilterDialog::FilterDialog(QWidget *parent, QString caption, QStringList extraOptions, bool modal)
        : QDialog(parent)
{
    setWindowTitle(caption.isNull() ? i18n("Krusader::Choose Files") : caption);
    setModal(modal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QTabWidget *filterWidget = new QTabWidget;

    filterTabs = FilterTabs::addTo(filterWidget, FilterTabs::HasProfileHandler, extraOptions);
    generalFilter = static_cast<GeneralFilter*> (filterTabs->get("GeneralFilter"));

    mainLayout->addWidget(filterWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), SLOT(slotOk()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(slotReset()));
    connect(filterTabs, SIGNAL(closeRequest(bool)), this, SLOT(slotCloseRequest(bool)));

    generalFilter->searchFor->setFocus();

    if(modal)
        exec();
}

void FilterDialog::applySettings(const FilterSettings &s)
{
    filterTabs->applySettings(s);
}

KRQuery FilterDialog::getQuery()
{
    return settings.toQuery();
}

bool FilterDialog::isExtraOptionChecked(QString name)
{
    return filterTabs->isExtraOptionChecked(name);
}

void FilterDialog::checkExtraOption(QString name, bool check)
{
    filterTabs->checkExtraOption(name, check);
}

void FilterDialog::slotCloseRequest(bool doAccept)
{
    if (doAccept) {
        slotOk();
        accept();
    } else
        reject();
}

void FilterDialog::slotReset()
{
    filterTabs->reset();
    generalFilter->searchFor->setFocus();
}

void FilterDialog::slotOk()
{
    settings = filterTabs->getSettings();
    if(settings.isValid())
        accept();
}

#include "filterdialog.moc"
