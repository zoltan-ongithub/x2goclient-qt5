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

#include "editconnectiondialog.h"
#include "x2goclientconfig.h"
#include "x2gologdebug.h"

#include "onmainwindow.h"
#include <QBoxLayout>
#include <QTabWidget>

#include "sessionwidget.h"
#include "sharewidget.h"
#include "connectionwidget.h"
#include "settingswidget.h"
#include "mediawidget.h"

EditConnectionDialog::EditConnectionDialog ( bool newSession, QString id, QWidget * par,
        int ind,Qt::WindowFlags f )
        : QDialog ( par,f )
{
    QVBoxLayout* ml=new QVBoxLayout ( this );
#ifdef Q_WS_HILDON
    ml->setMargin ( 2 );
#endif
    fr=new QTabWidget ( this );
    ml->addWidget ( fr );
    ONMainWindow* parent= ( ONMainWindow* ) par;

    QFont fnt=font();
    if ( parent->retMiniMode() )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    setFont ( fnt );

    sessSet=new SessionWidget ( newSession, id,parent );
    conSet=new ConnectionWidget ( id,parent );
    otherSet=new SettingsWidget ( id,parent );
    mediaSet=new MediaWidget ( id,parent );
    exportDir=new ShareWidget ( id,parent );

    fr->addTab ( sessSet,tr ( "&Session" ) );
    fr->addTab ( conSet,tr ( "&Connection" ) );
    fr->addTab ( otherSet,tr ( "&Input/Output" ) );
    fr->addTab ( mediaSet,tr ( "&Media" ) );
    if (! parent->getHideFolderSharing())
        fr->addTab ( exportDir,tr ( "&Shared folders" ) );

    QPushButton* ok=new QPushButton ( tr ( "&OK" ),this );
    QPushButton* cancel=new QPushButton ( tr ( "&Cancel" ),this );
    QPushButton* def=new QPushButton ( tr ( "Defaults" ),this );
    QHBoxLayout* bLay=new QHBoxLayout();
    bLay->setSpacing ( 5 );
    bLay->addStretch();
    bLay->addWidget ( ok );
    bLay->addWidget ( cancel );
    bLay->addWidget ( def );
    ml->addLayout ( bLay );
#ifdef Q_WS_HILDON
    bLay->setMargin ( 2 );
#endif

    setSizeGripEnabled ( true );
    setWindowIcon ( QIcon ( parent->iconsPath ( "/32x32/edit.png" ) ) );
    connect ( ok,SIGNAL ( clicked() ),this,SLOT ( accept() ) );
    connect ( cancel,SIGNAL ( clicked() ),this,SLOT ( reject() ) );
    connect ( def,SIGNAL ( clicked() ),this,SLOT ( slot_default() ) );
    connect ( sessSet,SIGNAL ( nameChanged ( const QString & ) ),this,
              SLOT ( slot_changeCaption ( const QString& ) ) );
    connect ( this,SIGNAL ( accepted() ),this,SLOT ( slot_accepted() ) );
    connect (sessSet, SIGNAL(directRDP(bool)), this, SLOT(slot_directRDP(bool)));

    connect (sessSet,
	     SIGNAL(settingsChanged(QString,QString,QString)), otherSet,
 	     SLOT(setServerSettings(QString,QString,QString)));

    ok->setDefault ( true );
#ifdef Q_WS_HILDON
    QSize sz=ok->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    ok->setFixedSize ( sz );
    sz=cancel->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    cancel->setFixedSize ( sz );
    sz=def->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    def->setFixedSize ( sz );
#endif
    if ( parent->retMiniMode() )
        setContentsMargins ( 3,3,3,3 );
    fr->setCurrentIndex ( ind );
    slot_changeCaption(sessSet->sessionName());
#ifdef Q_OS_LINUX
    sessSet->slot_rdpDirectClicked();
#endif
}


EditConnectionDialog::~EditConnectionDialog()
{}



void EditConnectionDialog::slot_changeCaption ( const QString& newName )
{
    setWindowTitle ( tr ( "Session preferences - " ) +newName );
}


void EditConnectionDialog::slot_accepted()
{
    conSet->saveSettings();
    exportDir->saveSettings();
    otherSet->saveSettings();
    mediaSet->saveSettings();
    sessSet->saveSettings();
}


void EditConnectionDialog::slot_default()
{
    switch ( fr->currentIndex() )
    {
    case 0:
    {
        sessSet->setDefaults();
    }
    break;
    case 1:
    {
        conSet->setDefaults();
    }
    break;
    case 2:
    {
        otherSet->setDefaults();
    }
    break;
    case 3:
    {
        mediaSet->setDefaults();
    }
    break;
    case 4:
    {
        exportDir->setDefaults();
    }
    break;
    }
}

#ifdef Q_OS_LINUX
void EditConnectionDialog::slot_directRDP(bool direct)
{
    fr->setTabEnabled(1,!direct);
    fr->setTabEnabled(3,!direct);
    fr->setTabEnabled(4,!direct);
    otherSet->setDirectRdp(direct);
}
#endif
