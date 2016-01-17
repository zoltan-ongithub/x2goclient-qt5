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
#ifndef FOLDEREXPLORER_H
#define FOLDEREXPLORER_H
#include "ui_folderexplorer.h"
class SessionExplorer;
class FolderExplorer: public QDialog, Ui_FolderExplorer
{
    Q_OBJECT
public:
    FolderExplorer(QString path, SessionExplorer* explorer, QWidget* parent=0);
    QString getCurrentPath()
    {
        return currentPath;
    }
private:
    SessionExplorer* explorer;
    QTreeWidgetItem* root;
    QTreeWidgetItem* menuItem;
    QString currentPath;
    void initFolders(QTreeWidgetItem* parent, QString path);
private slots:
    void slotContextMenu(QPoint p);
    void slotItemSelected(QTreeWidgetItem* it, int);
    void slotNewFolder();
    void slotChangeName();
    void slotChangeIcon();
    void slotDeleteFolder();
};
#endif
