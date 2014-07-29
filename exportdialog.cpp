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
#include "exportdialog.h"
#include "editconnectiondialog.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include "x2gosettings.h"
#include <QListView>
#include <QDir>
#include <QStringListModel>
#include <QShortcut>
#include "sessionbutton.h"
#include "onmainwindow.h"
#include <QFileDialog>
#include "sessionexplorer.h"

ExportDialog::ExportDialog ( QString sid,QWidget * par, Qt::WFlags f )
    : QDialog ( par,f )
{
    sessionId=sid;
    QVBoxLayout* ml=new QVBoxLayout ( this );
    QFrame *fr=new QFrame ( this );
    QHBoxLayout* frLay=new QHBoxLayout ( fr );

    parent= ( ONMainWindow* ) par;

    QPushButton* cancel=new QPushButton ( tr ( "&Cancel" ),this );
    QHBoxLayout* bLay=new QHBoxLayout();

    sessions=new QListView ( fr );
    frLay->addWidget ( sessions );

    exportDir=new QPushButton ( tr ( "&share" ),fr );
    editSession=new QPushButton ( tr ( "&Preferences ..." ),fr );
    newDir=new QPushButton ( tr ( "&Custom folder ..." ),fr );


    QVBoxLayout* actLay=new QVBoxLayout();
    actLay->addWidget ( exportDir );
    actLay->addWidget ( editSession );
    actLay->addWidget ( newDir );
    actLay->addStretch();
    frLay->addLayout ( actLay );

    QShortcut* sc=new QShortcut ( QKeySequence ( tr ( "Delete","Delete" ) ),
                                  this );
    connect ( cancel,SIGNAL ( clicked() ),this,SLOT ( close() ) );
    connect ( sc,SIGNAL ( activated() ),exportDir,SIGNAL ( clicked() ) );
    connect ( editSession,SIGNAL ( clicked() ),this,SLOT ( slot_edit() ) );
    connect ( newDir,SIGNAL ( clicked() ),this,SLOT ( slotNew() ) );
    connect ( exportDir,SIGNAL ( clicked() ),this,SLOT ( slot_accept() ) );
    bLay->setSpacing ( 5 );
    bLay->addStretch();
    bLay->addWidget ( cancel );
    ml->addWidget ( fr );
    ml->addLayout ( bLay );

    fr->setFrameStyle ( QFrame::StyledPanel | QFrame::Raised );
    fr->setLineWidth ( 2 );

    setSizeGripEnabled ( true );
    setWindowTitle ( tr ( "share folders" ) );
    connect ( sessions,SIGNAL ( clicked ( const QModelIndex& ) ),
              this,SLOT ( slot_activated ( const QModelIndex& ) ) );
    connect ( sessions,SIGNAL ( doubleClicked ( const QModelIndex& ) ),
              this,SLOT ( slot_dclicked ( const QModelIndex& ) ) );
    loadSessions();
}


ExportDialog::~ExportDialog()
{}

void ExportDialog::loadSessions()
{
    QStringListModel *model= ( QStringListModel* ) sessions->model();
    if ( !model )
        model=new QStringListModel();
    sessions->setModel ( model );

    QStringList dirs;
    model->setStringList ( dirs );

    X2goSettings st ( "sessions" );


    QString exports=st.setting()->value ( sessionId+"/export",
                                          ( QVariant ) QString::null ).toString();

    QStringList lst=exports.split ( ";",QString::SkipEmptyParts );
    for ( int i=0; i<lst.size(); ++i )
    {
#ifndef Q_OS_WIN
        QStringList tails=lst[i].split ( ":",QString::SkipEmptyParts );
#else
        QStringList tails=lst[i].split ( "#",QString::SkipEmptyParts );
#endif
        dirs<<tails[0];
    }


    model->setStringList ( dirs );


    //     removeSession->setEnabled(false);
    exportDir->setEnabled ( false );
    sessions->setEditTriggers ( QAbstractItemView::NoEditTriggers );
}


void ExportDialog::slot_activated ( const QModelIndex& )
{
    //     removeSession->setEnabled(true);
    exportDir->setEnabled ( true );
}

void ExportDialog::slot_dclicked ( const QModelIndex& )
{
    slot_accept();
}


void ExportDialog::slotNew()
{
    directory=QString::null;
    directory= QFileDialog::getExistingDirectory (
                   this,
                   tr ( "Select folder" ),
                   QDir::homePath() );

    if ( directory!=QString::null )
        accept();

}


void ExportDialog::slot_edit()
{
    const QList<SessionButton*>* sess=parent->getSessionExplorer()->getSessionsList();
    for ( int i=0; i< sess->size(); ++i )
    {
        if ( sess->at ( i )->id() ==sessionId )
        {
            parent->getSessionExplorer()->exportsEdit ( sess->at ( i ) );
            break;
        }
    }
    loadSessions();
}



void ExportDialog::slot_accept()
{
    int ind=sessions->currentIndex().row();
    if ( ind<0 )
        return;
    QStringListModel *model= ( QStringListModel* ) sessions->model();
    directory=model->stringList() [ind];
    accept();
}
