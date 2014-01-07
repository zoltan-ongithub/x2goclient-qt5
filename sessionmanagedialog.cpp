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

#include "x2goclientconfig.h"
#include "sessionmanagedialog.h"
#include "onmainwindow.h"

#include <QPushButton>
#include <QDir>
#include <QFrame>
#include <QBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QShortcut>
#include "sessionbutton.h"



SessionManageDialog::SessionManageDialog ( QWidget * parent,
        bool onlyCreateIcon, Qt::WFlags f )
		: QDialog ( parent, f )
{
	QVBoxLayout* ml=new QVBoxLayout ( this );
	QFrame *fr=new QFrame ( this );
	QHBoxLayout* frLay=new QHBoxLayout ( fr );

	QPushButton* ok=new QPushButton ( tr ( "E&xit" ),this );
	QHBoxLayout* bLay=new QHBoxLayout();

	sessions=new QListView ( fr );
	frLay->addWidget ( sessions );

	QPushButton* newSession=new QPushButton ( tr ( "&New session" ),fr );
	editSession=new QPushButton ( tr ( "&Session preferences" ),fr );
	removeSession=new QPushButton ( tr ( "&Delete session" ),fr );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
	if ( !ONMainWindow::getPortable() )
		createSessionIcon=new QPushButton (
		    tr ( "&Create session icon on desktop..." ),fr );
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
	connect ( sessions,SIGNAL ( clicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_activated ( const QModelIndex& ) ) );
	connect ( sessions,SIGNAL ( doubleClicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_dclicked ( const QModelIndex& ) ) );
}


SessionManageDialog::~SessionManageDialog()
{}


void SessionManageDialog::loadSessions()
{
	QStringListModel *model= ( QStringListModel* ) sessions->model();
	if ( !model )
		model=new QStringListModel();
	sessions->setModel ( model );
	QStringList lst;
	model->setStringList ( lst );

	const QList<SessionButton*> *sess=par->getSessionsList();

	for ( int i=0;i<sess->size();++i )
		lst<<sess->at ( i )->name();

	model->setStringList ( lst );
	removeSession->setEnabled ( false );
	editSession->setEnabled ( false );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
	if ( !ONMainWindow::getPortable() )
		createSessionIcon->setEnabled ( false );
#endif
	sessions->setEditTriggers ( QAbstractItemView::NoEditTriggers );
}


void SessionManageDialog::slot_activated ( const QModelIndex& )
{
	removeSession->setEnabled ( true );
	editSession->setEnabled ( true );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
	if ( !ONMainWindow::getPortable() )
		createSessionIcon->setEnabled ( true );
#endif
}

void SessionManageDialog::slot_dclicked ( const QModelIndex& )
{
	slot_edit();
}


void SessionManageDialog::slotNew()
{
	par->slotNewSession();
	loadSessions();
}


void SessionManageDialog::slot_edit()
{
	int ind=sessions->currentIndex().row();
	if ( ind<0 )
		return;
	par->slotEdit ( par->getSessionsList()->at ( ind ) );
	loadSessions();
}

void SessionManageDialog::slot_createSessionIcon()
{
	int ind=sessions->currentIndex().row();
	if ( ind<0 )
		return;
	par->slotCreateDesktopIcon ( par->getSessionsList()->at ( ind ) );
}


void SessionManageDialog::slot_delete()
{
	int ind=sessions->currentIndex().row();
	if ( ind<0 )
		return;
	par->slotDeleteButton ( par->getSessionsList()->at ( ind ) );
	loadSessions();
}
