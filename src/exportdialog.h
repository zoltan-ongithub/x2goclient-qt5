/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "x2goclientconfig.h"
#include <QDialog>
class QListView;
class QPushButton;
class QModelIndex;
class ONMainWindow;


/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(QString sid,QWidget * par, Qt::WFlags f = 0);

    ~ExportDialog();
    QString getExport(){return directory;}
private:
    QListView* sessions;
    QPushButton* editSession;
    QPushButton* exportDir;
    QPushButton* newDir;
    QString directory;
    ONMainWindow* parent;
    void loadSessions();
    QString sessionId;
private slots:
    void slot_activated(const QModelIndex& index);
    void slotNew();
    void slot_edit();
    void slot_dclicked(const QModelIndex& index);
    void slot_accept();
};

#endif
