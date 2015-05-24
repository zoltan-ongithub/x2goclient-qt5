/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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

#include "x2goclientconfig.h"
#include "sessionmanagedialog.h"
#include "onmainwindow.h"

#include <QPushButton>
#include <QDir>
#include <QFrame>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStringListModel>
#include <QShortcut>
#include "sessionbutton.h"
#include "folderbutton.h"
#include "sessionexplorer.h"

#define SESSIONROLE Qt::UserRole+1
#define SESSIONIDROLE Qt::UserRole+2

SessionManageDialog::SessionManageDialog ( QWidget * parent,
        bool onlyCreateIcon, Qt::WFlags f )
    : QDialog ( parent, f )
{
    QVBoxLayout* ml=new QVBoxLayout ( this );
    QFrame *fr=new QFrame ( this );
    QHBoxLayout* frLay=new QHBoxLayout ( fr );

    currentPath="";

    QPushButton* ok=new QPushButton ( tr ( "E&xit" ),this );
    QHBoxLayout* bLay=new QHBoxLayout();

    sessions=new QTreeWidget ( fr );
    frLay->addWidget ( sessions );

    QPushButton* newSession=new QPushButton ( tr ( "&New session" ),fr );
    editSession=new QPushButton ( tr ( "&Session preferences" ),fr );
    removeSession=new QPushButton ( tr ( "&Delete session" ),fr );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        createSessionIcon=new QPushButton (
            tr ( "&Create session icon on desktop ..." ),fr );
#endif
    par= ( ONMainWindow* ) parent;
    newSession->setIcon ( QIcon (
                              par->iconsPath ( "/16x16/new_file.png" ) ) );
    editSession->setIcon ( QIcon (
                               par->iconsPath ( "/16x16/edit.png" ) ) );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        createSessionIcon->setIcon (
            QIcon ( par->iconsPath ( "/16x16/create_file.png" ) ) );
#endif
    removeSession->setIcon (
        QIcon ( par->iconsPath ( "/16x16/delete.png" ) ) );

    QVBoxLayout* actLay=new QVBoxLayout();
    actLay->addWidget ( newSession );
    actLay->addWidget ( editSession );
    actLay->addWidget ( removeSession );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        actLay->addWidget ( createSessionIcon );
#endif
    actLay->addStretch();
    frLay->addLayout ( actLay );

    if ( onlyCreateIcon )
    {
        newSession->hide();
        editSession->hide();
        removeSession->hide();
    }

    QShortcut* sc=new QShortcut (
        QKeySequence ( tr ( "Delete","Delete" ) ),this );
    connect ( ok,SIGNAL ( clicked() ),this,SLOT ( close() ) );
    connect (
        sc,SIGNAL ( activated() ),removeSession,SIGNAL ( clicked() ) );
    connect (
        removeSession,SIGNAL ( clicked() ),this,SLOT ( slot_delete() ) );
    connect ( editSession,SIGNAL ( clicked() ),this,SLOT ( slot_edit() ) );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        connect ( createSessionIcon,SIGNAL ( clicked() ),
                  this,SLOT ( slot_createSessionIcon() ) );
#endif
    connect ( newSession,SIGNAL ( clicked() ),this,SLOT ( slotNew() ) );
    bLay->setSpacing ( 5 );
    bLay->addStretch();
    bLay->addWidget ( ok );
    ml->addWidget ( fr );
    ml->addLayout ( bLay );

    fr->setFrameStyle ( QFrame::StyledPanel | QFrame::Raised );
    fr->setLineWidth ( 2 );

    setSizeGripEnabled ( true );
    setWindowIcon (
        QIcon (
            ( ( ONMainWindow* ) parent )->iconsPath (
                "/32x32/edit.png" ) ) );

    setWindowTitle ( tr ( "Session management" ) );
    loadSessions();
    connect ( sessions, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
              this,SLOT(slot_activated(QTreeWidgetItem*,int)) );
    connect ( sessions,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
              this,SLOT(slot_dclicked(QTreeWidgetItem*,int)) );
}


SessionManageDialog::~SessionManageDialog()
{}


void SessionManageDialog::loadSessions()
{
    sessions->clear();

    /*const QList<SessionButton*> *sess=par->getSessionExplorer()->getSessionsList();*/
    /*const QList<FolderButton*> *folders=par->getSessionExplorer()->getFoldersList();*/

    removeSession->setEnabled ( false );
    editSession->setEnabled ( false );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        createSessionIcon->setEnabled ( false );
#endif

    QTreeWidgetItem* root;

    root=new QTreeWidgetItem(sessions);
    root->setText(0,"/");
    root->setIcon(0,QIcon(":/img/icons/128x128/folder.png"));
    initFolders(root, "");
    root->setExpanded(true);
    root->setData(0, SESSIONROLE , false);

    sessions->setRootIsDecorated(false);
    sessions->setHeaderHidden(true);
}

void SessionManageDialog::initFolders(QTreeWidgetItem* parent, QString path)
{
    FolderButton* b;
    foreach(b, *(par->getSessionExplorer()->getFoldersList()))
    {
        if(b->getPath()==path)
        {
            QTreeWidgetItem* it=new QTreeWidgetItem(parent);
            it->setText(0,b->getName());
            it->setIcon(0, QIcon(*(b->folderIcon())));
            QString normPath=(b->getPath()+"/"+b->getName()).split("/",QString::SkipEmptyParts).join("/");
            it->setData(0,Qt::UserRole, normPath+"/");
            it->setData(0, SESSIONROLE , false);
            initFolders(it, normPath);
        }
    }
    for(int i=0; i< par->getSessionExplorer()->getSessionsList()->count(); ++i)
    {
        SessionButton* s=par->getSessionExplorer()->getSessionsList()->at(i);
        if(s->getPath()==path)
        {
            QTreeWidgetItem* it=new QTreeWidgetItem(parent);
            it->setText(0,s->name());
            it->setIcon(0, QIcon(*(s->sessIcon())));
            QString normPath=(s->getPath()+"/"+s->name()).split("/",QString::SkipEmptyParts).join("/");
            it->setData(0,Qt::UserRole, normPath+"/");
            it->setData(0, SESSIONROLE, true);
            it->setData(0, SESSIONIDROLE, i);
            initFolders(it, normPath);
        }
    }
}


void SessionManageDialog::slot_activated ( QTreeWidgetItem* item, int )
{
    bool isSess=item->data(0, SESSIONROLE).toBool();
    if(!isSess)
    {
        currentPath=item->data(0,Qt::UserRole).toString().split("/",QString::SkipEmptyParts).join("/");
    }
    removeSession->setEnabled ( isSess );
    editSession->setEnabled ( isSess );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
    if ( !ONMainWindow::getPortable() )
        createSessionIcon->setEnabled ( isSess );
#endif
}

void SessionManageDialog::slot_dclicked ( QTreeWidgetItem* item, int )
{
    if(item->data(0, SESSIONROLE).toBool())
        slot_edit();
}


void SessionManageDialog::slotNew()
{
    par->getSessionExplorer()->setCurrrentPath(currentPath);
    par->slotNewSession();
    loadSessions();
}


void SessionManageDialog::slot_edit()
{
    if((! sessions->currentItem()) || (! sessions->currentItem()->data(0, SESSIONROLE).toBool()))
        return;
    int ind=sessions->currentItem()->data(0, SESSIONIDROLE).toInt();
    par->getSessionExplorer()->slotEdit ( par->getSessionExplorer()->getSessionsList()->at ( ind ) );
    loadSessions();
}

void SessionManageDialog::slot_createSessionIcon()
{
    if((! sessions->currentItem()) || (! sessions->currentItem()->data(0, SESSIONROLE).toBool()))
        return;
    int ind=sessions->currentItem()->data(0, SESSIONIDROLE).toInt();
    par->getSessionExplorer()->slotCreateDesktopIcon ( par->getSessionExplorer()->getSessionsList()->at ( ind ) );
}


void SessionManageDialog::slot_delete()
{
    if((! sessions->currentItem()) || (! sessions->currentItem()->data(0, SESSIONROLE).toBool()))
        return;
    int ind=sessions->currentItem()->data(0, SESSIONIDROLE).toInt();
    par->getSessionExplorer()->slotDeleteButton ( par->getSessionExplorer()->getSessionsList()->at ( ind ) );
    loadSessions();
}
