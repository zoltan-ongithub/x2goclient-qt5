/**************************************************************************
*   Copyright (C) 2005-2014 by Oleksandr Shneyder                         *
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

#ifndef SESSIONEXPLORER_H
#define SESSIONEXPLORER_H

#include <QObject>
#include <QList>
#include <QStringList>

class SessionButton;
class FolderButton;
class ONMainWindow;
class QToolButton;
class QLabel;
class QHBoxLayout;

class SessionExplorer: public QObject
{
    Q_OBJECT
public:
    SessionExplorer(ONMainWindow *p);
    ~SessionExplorer();
    QList<SessionButton*> * getSessionsList()
    {
        return &sessions;
    }
    SessionButton* getLastSession() {
        return lastSession;
    }
    void setLastSession(SessionButton* s)
    {
        lastSession=s;
    }
    void cleanSessions();
    SessionButton* createBut ( const QString& id );
    void placeButtons();
    QHBoxLayout* getNavigationLayout()
    {
        return navigationLayout;
    }
    void resize();
//vars
private:
    QList<SessionButton*> sessions;
    QList<FolderButton*> folders;
    SessionButton* lastSession;
    ONMainWindow* parent;
    QToolButton* backButton;
    QLabel* pathLabel;
    QHBoxLayout* navigationLayout;
    QString currentPath;

//functions
private:
    void setNavigationVisible(bool value);
    int findFolder(QString path);
    void createFolder(QString path);

public slots:
    void slotDeleteButton ( SessionButton * bt );
    void slotEdit ( SessionButton* );
    void slotCreateDesktopIcon ( SessionButton* bt );
    void exportsEdit ( SessionButton* bt );
private slots:
    void slotFolderSelected(FolderButton* bt);
    void slotLevelUp();
    QStringList getFolderChildren(FolderButton* folder);
};

#endif
