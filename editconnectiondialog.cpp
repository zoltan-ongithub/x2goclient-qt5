//
// C++ Implementation: editconnectiondialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

EditConnectionDialog::EditConnectionDialog ( QString id, QWidget * par,
        int ind,Qt::WFlags f )
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

	sessSet=new SessionWidget ( id,parent );
	conSet=new ConnectionWidget ( id,parent );
	otherSet=new SettingsWidget ( id,parent );
	exportDir=new ShareWidget ( id,parent );


	fr->addTab ( sessSet,tr ( "&Session" ) );
	fr->addTab ( conSet,tr ( "&Connection" ) );
	fr->addTab ( otherSet,tr ( "&Settings" ) );
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
			exportDir->setDefaults();
		}
		break;
	}
}

