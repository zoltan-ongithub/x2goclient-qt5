//
// C++ Implementation: sessionbutton
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "sessionbutton.h"
#include "x2goclientconfig.h"

#include <QFont>
#include <QPixmap>
#include <QLabel>
#include <QSettings>
#include <QDir>
#include <QLayout>
#include <QComboBox>
#include <QMouseEvent>
#include <QPushButton>
#include "onmainwindow.h"
#include "x2gologdebug.h"


SessionButton::SessionButton ( ONMainWindow* mw,QWidget *parent, QString id )
		: SVGFrame ( ":/svg/sessionbut.svg",false,parent )
{
	QFont fnt=font();
	if ( mw->retMiniMode() )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	setFont ( fnt );
	setMouseTracking ( true );
	setFocusPolicy ( Qt::NoFocus );
	bool miniMode=mw->retMiniMode();
	if ( !miniMode )
		setFixedSize ( 340,190 );
	else
		setFixedSize ( 250,145 );


	par= mw;
	connect ( this,SIGNAL ( clicked() ),this,SLOT ( slotClicked() ) );

	sid=id;

	cmdBox=new QComboBox ( this );
	cmdBox->setMouseTracking ( true );
	cmdBox->setFrame ( false );
// 	cmdBox->setEditable ( true );
	QPalette cpal=cmdBox->palette();
	cpal.setColor ( QPalette::Button,QColor ( 255,255,255 ) );
	cpal.setColor ( QPalette::Base,QColor ( 255,255,255 ) );
	cpal.setColor ( QPalette::Window,QColor ( 255,255,255 ) );
	cmdBox->setPalette ( cpal );

	geomBox=new QComboBox ( this );
	geomBox->setMouseTracking ( true );
	geomBox->setFrame ( false );
	geomBox->setEditable ( true );
	geomBox->setEditable ( false );
	geomBox->update();
	geomBox->setPalette ( cpal );

	sessName=new QLabel ( this );
	fnt=sessName->font();
	fnt.setBold ( true );
	sessName->setFont ( fnt );
	icon=new QLabel ( this );
	cmd=new QLabel ( this );
	cmd->setMouseTracking ( true );
	serverIcon=new QLabel ( this );
	geomIcon=new QLabel ( this );
	geomIcon->setMouseTracking ( true );
	cmdIcon=new QLabel ( this );
	cmdIcon->setMouseTracking ( true );
	server=new QLabel ( this );
	geom=new QLabel ( this );
	geom->setMouseTracking ( true );

	sound=new QPushButton ( this );
	soundIcon=new QLabel ( this );
	sound->setPalette ( cpal );
	sound->setFlat ( true );
	sound->setMouseTracking ( true );
	connect ( sound,SIGNAL ( clicked() ),this,SLOT ( slot_soundClicked() ) );

	editBut=new QPushButton ( this );
	editBut->setMouseTracking ( true );
	connect ( editBut,SIGNAL ( clicked() ),this,SLOT ( slotEdit() ) );
	editBut->setIcon ( QIcon ( par->iconsPath ( "/16x16/edit.png" ) ) );
	editBut->setIconSize ( QSize ( 16,16 ) );
	editBut->setFixedSize ( 24,24 );
	editBut->setFlat ( true );
	editBut->setPalette ( cpal );

	rmBut=new QPushButton ( this );
	rmBut->setMouseTracking ( true );
	connect ( rmBut,SIGNAL ( clicked() ),this,SLOT ( slotRemove() ) );
	rmBut->setIcon ( QIcon ( par->iconsPath ( "/16x16/delete.png" ) ) );
	rmBut->setIconSize ( QSize ( 16,16 ) );
	rmBut->setFixedSize ( 24,24 );
	rmBut->setFlat ( true );
	rmBut->setPalette ( cpal );


	editBut->setToolTip ( tr ( "Session Preferences..." ) );
	rmBut->setToolTip ( tr ( "Delete Session..." ) );
	cmdBox->setToolTip ( tr ( "Select Type" ) );
	geomBox->setToolTip ( tr ( "Select Resolution" ) );
	sound->setToolTip ( tr ( "Toggle Sound support" ) );
	icon->move ( 10,10 );

	if ( !miniMode )
	{
		sessName->move ( 80,34 );
		rmBut->move ( 307,10 );
		editBut->move ( 307,156 );
		serverIcon->move ( 58,84 );
		server->move ( 80,84 );
		cmdIcon->move ( 58,108 );
		cmd->move ( 80,108 );
		cmdBox->move ( 80,108 );
		geomIcon->move ( 58,132 );
		geom->move ( 80,132 );
		geomBox->move ( 80,132 );
		soundIcon->move ( 58,156 );
		sound->move ( 80,156 );
	}
	else
	{
		rmBut->move ( 218,8 );
		editBut->move ( 218,113 );
		sessName->move ( 64,11 );
		serverIcon->move ( 66,44 );
		server->move ( 88,44 );
		cmdIcon->move ( 66,68 );
		cmd->move ( 88,68 );
		cmdBox->move ( 88,68 );
		geomIcon->move ( 66,92 );
		geom->move ( 88,92 );
		geomBox->move ( 88,92 );
		soundIcon->move ( 66,116 );
		sound->move ( 86,116 );
	}


	cmdBox->hide();
	geomBox->hide();
	QPixmap pix;
	pix.load ( par->iconsPath ( "/16x16/session.png" ) );
	serverIcon->setPixmap ( pix );
	serverIcon->setFixedSize ( 16,16 );

	pix.load ( par->iconsPath ( "/16x16/resolution.png" ) );
	geomIcon->setPixmap ( pix );
	geomIcon->setFixedSize ( 16,16 );

	pix.load ( par->iconsPath ( "/16x16/audio.png" ) );
	soundIcon->setPixmap ( pix );
	soundIcon->setFixedSize ( 16,16 );
	redraw();

	connect ( cmdBox,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slot_cmd_change ( const QString& ) ) );
	connect ( geomBox,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slot_geom_change ( const QString& ) ) );

	editBut->setFocusPolicy ( Qt::NoFocus );
	sound->setFocusPolicy ( Qt::NoFocus );
	cmdBox->setFocusPolicy ( Qt::NoFocus );
	geomBox->setFocusPolicy ( Qt::NoFocus );

}

SessionButton::~SessionButton()
{}

void SessionButton::slotClicked()
{
	emit sessionSelected ( this );
}

void SessionButton::slotEdit()
{
	editBut->setFlat ( true );
	emit signal_edit ( this );
}


void SessionButton::redraw()
{
	bool snd;
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	sessName->setText ( st.value ( sid+"/name", ( QVariant ) tr ( "New Session" ) ).toString() );
	QString sessIcon=st.value ( sid+"/icon",
	                            ( QVariant ) ":icons/128x128/x2gosession.png" ).toString();
	QPixmap pix ( sessIcon );
	if ( !par->retMiniMode() )
		icon->setPixmap ( pix.scaled ( 64,64,Qt::IgnoreAspectRatio,Qt::SmoothTransformation ) );
	else
		icon->setPixmap ( pix.scaled ( 48,48,Qt::IgnoreAspectRatio,Qt::SmoothTransformation ) );

	QString sv=st.value ( sid+"/host", ( QVariant ) QString::null ).toString();
	QString uname=st.value ( sid+"/user", ( QVariant ) QString::null ).toString();
	server->setText ( uname+"@"+sv );

	QString command=st.value ( sid+"/command",
	                           ( QVariant ) tr ( "/usr/bin/startkde" ) ).toString();
	rootless=st.value ( sid+"/rootless",
	                    false ).toBool();


	cmdBox->clear();
	cmdBox->addItem ( "KDE" );
	cmdBox->addItem ( "GNOME" );
	cmdBox->addItems ( par->transApplicationsNames() );

	if ( command=="KDE" )
	{
		pix.load ( par->iconsPath ( "/16x16/kde.png" ) );
		cmdBox->setCurrentIndex ( 0 );
	}
	else if ( command =="GNOME" )
	{
		pix.load ( par->iconsPath ( "/16x16/gnome.png" ) );
		cmdBox->setCurrentIndex ( 1 );
	}
	else
	{
		pix.load ( par->iconsPath ( "/16x16/X.png" ) );
		command=par->transAppName(command);
		int id= cmdBox->findText ( command );
		if (id ==-1 )
		{
			cmdBox->addItem ( command );
			cmdBox->setCurrentIndex(cmdBox->count()-1);
		}
		else
		   cmdBox->setCurrentIndex ( id);
	}



	cmdIcon->setPixmap ( pix );
	cmd->setText ( command );


	geomBox->clear();
	geomBox->addItem ( tr ( "fullscreen" ) );
#ifndef Q_WS_HILDON	
	geomBox->addItem ( "1440x900" );
	geomBox->addItem ( "1280x1024" );
	geomBox->addItem ( "1024x768" );
	geomBox->addItem ( "800x600" );
#else
		geomBox->addItem ( tr ( "window" ) );
#endif
	if ( st.value ( sid+"/fullscreen", ( QVariant ) false ).toBool() )
	{
		geom->setText ( tr ( "fullscreen" ) );
	}
	else
	{
#ifndef	Q_WS_HILDON
		QString g=QString::number ( st.value ( sid+"/width" ).toInt() );
		g+="x"+QString::number ( st.value ( sid+"/height" ).toInt() );
		geom->setText ( g );
		if ( geomBox->findText ( g ) ==-1 )
			geomBox->addItem ( g );
		geomBox->setCurrentIndex ( geomBox->findText ( g ) );
#else
		geom->setText ( tr ( "window" ) );
		geomBox->setCurrentIndex ( 1);
#endif		
	}




	snd=st.value ( sid+"/sound", ( QVariant ) true ).toBool();
	if ( snd )
		sound->setText ( tr ( "Enabled" ) );
	else
		sound->setText ( tr ( "Disabled" ) );
	soundIcon->setEnabled ( snd );
	QFontMetrics fm ( sound->font() );
	sound->setFixedSize ( fm.size ( Qt::TextSingleLine,sound->text() ) +QSize ( 4,4 ) );

	sessName->setMinimumSize ( sessName->sizeHint() );
	geom->setMinimumSize ( geom->sizeHint() );
	cmd->setMinimumSize ( cmd->sizeHint() );
	server->setMinimumSize ( server->sizeHint() );
}

void SessionButton::mousePressEvent ( QMouseEvent * event )
{
	SVGFrame::mousePressEvent ( event );
	loadBg ( ":/svg/sessionbut_grey.svg" );
}

void SessionButton::mouseReleaseEvent ( QMouseEvent * event )
{
	SVGFrame::mouseReleaseEvent ( event );
	int x=event->x();
	int y=event->y();
	loadBg ( ":/svg/sessionbut.svg" );
	if ( x>=0 && x< width() && y>=0 && y<height() )
		emit clicked();
}

void SessionButton::mouseMoveEvent ( QMouseEvent * event )
{

	SVGFrame::mouseMoveEvent ( event );
	if ( cmd->isVisible() )
		if ( event->x() > cmd->x() && event->x() < cmd->x() +cmd->width() &&
		        event->y() >cmd->y() && event->y() <cmd->y() +cmd->height() )
		{
			if ( cmdBox->width() <cmd->width() )
				cmdBox->setFixedWidth ( cmd->width() +20 );
			if ( cmdBox->height() !=cmd->height() )
				cmdBox->setFixedHeight ( cmd->height() );
			cmd->hide();
			cmdBox->show();
		}
	if ( cmdBox->isVisible() )
		if ( event->x() < cmdBox->x() || event->x() > cmdBox->x() +cmdBox->width() ||
		        event->y() <cmdBox->y() || event->y() >cmdBox->y() +cmdBox->height() )
		{
			cmdBox->hide();
			cmd->show();
		}


	if ( sound->isFlat() )
	{
		if ( event->x() > sound->x() && event->x() < sound->x() +sound->width() &&
		        event->y() >sound->y() && event->y() <sound->y() +sound->height() )
		{
			sound->setFlat ( false );
		}
	}
	else
	{
		if ( event->x() < sound->x() || event->x() > sound->x() +sound->width() ||
		        event->y() <sound->y() || event->y() >sound->y() +sound->height() )
		{
			sound->setFlat ( true );
		}
	}


	if ( editBut->isFlat() )
	{
		if ( event->x() > editBut->x() && event->x() < editBut->x() +editBut->width() &&
		        event->y() >editBut->y() && event->y() <editBut->y() +editBut->height() )
			editBut->setFlat ( false );
	}
	else
	{
		if ( event->x() < editBut->x() || event->x() > editBut->x() +editBut->width() ||
		        event->y() <editBut->y() || event->y() >editBut->y() +editBut->height() )
			editBut->setFlat ( true );
	}

	if ( rmBut->isFlat() )
	{
		if ( event->x() > rmBut->x() && event->x() < rmBut->x() +rmBut->width() &&
		        event->y() >rmBut->y() && event->y() <rmBut->y() +rmBut->height() )
			rmBut->setFlat ( false );
	}
	else
	{
		if ( event->x() < rmBut->x() || event->x() > rmBut->x() +rmBut->width() ||
		        event->y() <rmBut->y() || event->y() >rmBut->y() +rmBut->height() )
			rmBut->setFlat ( true );
	}


	if ( geom->isVisible() )
		if ( event->x() > geom->x() && event->x() < geom->x() +geom->width() &&
		        event->y() >geom->y() && event->y() <geom->y() +geom->height() )
		{
			if ( geomBox->width() <geom->width() )
				geomBox->setFixedWidth ( geom->width() +20 );
			if ( geomBox->height() !=geom->height() )
				geomBox->setFixedHeight ( geom->height() );
			geom->hide();
			geomBox->show();
		}
	if ( geomBox->isVisible() )
		if ( event->x() < geomBox->x() || event->x() > geomBox->x() +geomBox->width() ||
		        event->y() <geomBox->y() || event->y() >geomBox->y() +geomBox->height() )
		{
			geomBox->hide();
			geom->show();
		}
}


void SessionButton::slot_soundClicked()
{
	bool snd=!soundIcon->isEnabled();
	soundIcon->setEnabled ( snd );
	if ( snd )
		sound->setText ( tr ( "Enabled" ) );
	else
		sound->setText ( tr ( "Disabled" ) );
	QFontMetrics fm ( sound->font() );
	sound->setFixedSize ( fm.size ( Qt::TextSingleLine,sound->text() ) +QSize ( 4,4 ) );

#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	st.setValue ( sid+"/sound", ( QVariant ) snd );
	st.sync();


}

void SessionButton::slot_cmd_change ( const QString& command )
{
	cmd->setText ( command );
	QPixmap pix;
	bool newRootless=rootless;
	if ( command=="KDE" )
	{
		newRootless=false;
		pix.load ( par->iconsPath ( "/16x16/kde.png" ) );
	}
	else if ( command =="GNOME" )
	{
		newRootless=false;
		pix.load ( par->iconsPath ( "/16x16/gnome.png" ) );
	}
	else
		pix.load ( par->iconsPath ( "/16x16/X.png" ) );
	cmdIcon->setPixmap ( pix );
	QString cmd=command;

#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	if ( command=="startkde" )
	{
		cmd="KDE";
		newRootless=false;
	}
	if ( command=="gnome-session" )
	{
		cmd="GNOME";
		newRootless=false;
	}
	bool found=false;
	cmd=par->internAppName(cmd,&found);
	if(found)
		newRootless=true;
	st.setValue ( sid+"/command", ( QVariant ) cmd );
	st.setValue ( sid+"/rootless", ( QVariant ) newRootless );
	st.sync();

}


void SessionButton::slot_geom_change ( const QString& new_g )
{
	geom->setText ( new_g );
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	if ( new_g==tr ( "fullscreen" ) )
		st.setValue ( sid+"/fullscreen", ( QVariant ) true );
	else
	{
		QString new_geom=new_g;
#ifdef Q_WS_HILDON
		new_geom="800x600";
#endif
		st.setValue ( sid+"/fullscreen", ( QVariant ) false );
		QStringList lst=new_geom.split ( 'x' );
		st.setValue ( sid+"/width", ( QVariant ) lst[0] );
		st.setValue ( sid+"/height", ( QVariant ) lst[1] );
	}
	st.sync();
}

bool SessionButton::lessThen ( const SessionButton* b1, const SessionButton* b2 )
{
	return b1->sessName->text().toLower().localeAwareCompare ( b2->sessName->text().toLower() ) <0;
}



void SessionButton::slotRemove()
{
	emit ( signal_remove ( this ) );
}


QString SessionButton::name()
{
	return sessName->text();
}
