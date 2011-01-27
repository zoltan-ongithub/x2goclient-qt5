//
// C++ Implementation: connectionwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "connectionwidget.h"

#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QStringList>
#include <QGroupBox>
#include <QBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "onmainwindow.h"
ConnectionWidget::ConnectionWidget ( QString id, ONMainWindow * mw,
                                     QWidget * parent, Qt::WindowFlags f )
		: ConfigWidget ( id,mw,parent,f )
{
	QVBoxLayout *connLay=new QVBoxLayout ( this );
#ifndef Q_WS_HILDON
	QGroupBox* netSpd=new QGroupBox ( tr ( "&Connection speed" ),this );
	QVBoxLayout *spdLay=new QVBoxLayout ( netSpd );
#else
	QFrame* netSpd=this ;
	QVBoxLayout *spdLay=new QVBoxLayout ();
	spdLay->addWidget ( new QLabel ( tr ( "Connection speed:" ),netSpd ) );
#endif
	spd=new QSlider ( Qt::Horizontal,netSpd );
	spd->setMinimum ( 0 );
	spd->setMaximum ( 4 );
	spd->setTickPosition ( QSlider::TicksBelow );
	spd->setTickInterval ( 1 );
	spd->setSingleStep ( 1 );
	spd->setPageStep ( 1 );

	QHBoxLayout *tickLay=new QHBoxLayout();
	QHBoxLayout *slideLay=new QHBoxLayout();
	slideLay->addWidget ( spd );
	QLabel* mlab= new QLabel ( "MODEM",netSpd );
	tickLay->addWidget ( mlab );
	tickLay->addStretch();
	tickLay->addWidget ( new QLabel ( "ISDN",netSpd ) );
	tickLay->addStretch();
	tickLay->addWidget ( new QLabel ( "ADSL",netSpd ) );
	tickLay->addStretch();
	tickLay->addWidget ( new QLabel ( "WAN",netSpd ) );
	tickLay->addStretch();
	tickLay->addWidget ( new QLabel ( "LAN",netSpd ) );
	spdLay->addLayout ( slideLay );
	spdLay->addLayout ( tickLay );
	QFontMetrics fm ( mlab->font() );
	slideLay->insertSpacing ( 0,fm.width ( "MODEM" ) /2 );
	slideLay->addSpacing ( fm.width ( "LAN" ) /2 );

#ifndef Q_WS_HILDON
	QGroupBox* compr=new QGroupBox ( tr ( "C&ompression" ),this );
	QHBoxLayout* comprLay=new QHBoxLayout ( compr );
#else
	QFrame* compr=this;
	QHBoxLayout* comprLay=new QHBoxLayout ();
#endif
	packMethode = new QComboBox ( this );
	quali= new QSpinBox ( this );
	quali->setRange ( 0,9 );
#ifdef Q_WS_HILDON
	quali->setFixedHeight ( int ( quali->sizeHint().height() *1.5 ) );
#endif

	QVBoxLayout* colLay=new QVBoxLayout();
	QVBoxLayout* cowLay=new QVBoxLayout();
	QHBoxLayout* spbl=new QHBoxLayout();
#ifndef Q_WS_HILDON
	colLay->addWidget ( new QLabel ( tr ( "Method:" ),compr ) );
#else
	colLay->addWidget ( new QLabel ( tr ( "Compression method:" ),compr ) );
#endif
	colLay->addWidget ( qualiLabel=new QLabel ( tr ( "Image quality:" ),
	        compr ) );
	cowLay->addWidget ( packMethode );
	spbl->addWidget ( quali );
	spbl->addStretch();
	cowLay->addLayout ( spbl );
	comprLay->addLayout ( colLay );
	comprLay->addLayout ( cowLay );
#ifndef Q_WS_HILDON
	connLay->addWidget ( netSpd );
	connLay->addWidget ( compr );
#else
	connLay->addLayout ( spdLay );
	connLay->addLayout ( comprLay );
#endif
	connLay->addStretch();

	connect ( packMethode,SIGNAL ( activated ( const QString& ) ),this,
	          SLOT ( slot_changePack ( const QString& ) ) );
	readConfig();
}


ConnectionWidget::~ConnectionWidget()
{
}

void ConnectionWidget::loadPackMethods()
{
	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );
			qualiList<<pc;
		}
		packMethode->addItem ( pc );
	}
	file.close();
}


void ConnectionWidget::slot_changePack ( const QString& pc )
{
	bool ct=qualiList.contains ( pc );
	quali->setEnabled ( ct );
	qualiLabel->setEnabled ( ct );
}

void ConnectionWidget::readConfig()
{

	loadPackMethods();
#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	spd->setValue ( st.value (
	                    sessionId+"/speed",
	                    ( QVariant ) mainWindow->getDefaultLink()
	                ).toInt() );
	QString mt=st.value (
	               sessionId+"/pack",
	               ( QVariant ) mainWindow->getDefaultPack() ).toString();


	packMethode->setCurrentIndex ( packMethode->findText ( mt ) );
	quali->setValue ( st.value ( sessionId+"/quality",
	                             mainWindow->getDefaultQuality() ).toInt() );
	slot_changePack ( mt );
}


void ConnectionWidget::setDefaults()
{
	spd->setValue ( 2 );
	packMethode->setCurrentIndex (
	    packMethode->findText ( "16m-jpeg" ) );
	quali->setValue ( 9 );
	slot_changePack ( "16m-jpeg" );
}

void ConnectionWidget::saveSettings()
{

#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	st.setValue ( sessionId+"/speed", ( QVariant ) spd->value() );
	st.setValue ( sessionId+"/pack",
	              ( QVariant ) packMethode->currentText() );
	st.setValue ( sessionId+"/quality", ( QVariant ) quali->value() );
	st.sync();
}
