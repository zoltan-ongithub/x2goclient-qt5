/***************************************************************************
*   Copyright (C) 2005 by Oleksandr Shneyder   *
*   oleksandr.shneyder@treuchtlingen.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "version.h"
#include "x2goclientconfig.h"
#include "onmainwindow.h"
#include "userbutton.h"
#include "exportdialog.h"
#include <QApplication>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QComboBox>
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut>
#include <QSettings>
#include <QDir>
#include <unistd.h>
#include <QTreeView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QCheckBox>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QtNetwork/QTcpSocket>
#include <QPlastiqueStyle>
#include "sshprocess.h"
#include "imgframe.h"

#if !defined WINDOWS
#include <sys/mount.h>
#ifdef Q_OS_LINUX
#include <linux/fs.h>
#endif // Q_OS_LINUX
#endif // !defined WINDOWS

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QCoreApplication>

#include <QDesktopWidget>

#define ldap_SUCCESS 0
#define ldap_INITERROR 1
#define ldap_OPTERROR 2
#define ldap_BINDERROR 3
#define ldap_SEARCHERROR 4
#define ldap_NOBASE 5



//LDAP attributes
#define SESSIONID "sn"
#define USERNAME  "cn"
#define CLIENT    "registeredAddress"
#define SERVER    "postalAddress"
#define SNDPORT   "postalCode"
#define RES       "title"
#define DISPLAY   "street"
#define STATUS    "st"
#define STARTTIME "telephoneNumber"
#define CREATTIME "telexNumber"
#define SUSPTIME  "internationaliSDNNumber"

#define SESSIONCMD "o"
#define FIRSTUID "ou"
#define LASTUID "l"

#define NETSOUND "o"
#define SNDSUPPORT "sn"


#include <QDateTime>

#include "SVGFrame.h"
#include "configdialog.h"
#include "editconnectiondialog.h"
#include "sessionbutton.h"
#include "sessionmanagedialog.h"
#include "x2gologdebug.h"



void x2goSession::operator = ( const x2goSession& s )
{
	agentPid=s.agentPid;
	clientIp=s.clientIp;
	cookie=s.cookie;
	crTime=s.crTime;
	display=s.display;
	grPort=s.grPort;
	server=s.server;
	sessionId=s.sessionId;
	sndPort=s.sndPort;
	status=s.status;
}


ONMainWindow::ONMainWindow ( QWidget *parent ) :QMainWindow ( parent )
{

	hide();
	drawMenu=true;
	usePGPCard=false;
	extLogin=false;
	startMaximized=false;
	defaultUseSound=true;
	defaultSetKbd=false;
	useEsd=false;
	extStarted=false;
	defaultLink=2;
	defaultFullscreen=false;
	acceptRsa=false;
	cardStarted=false;
	cardReady=false;
	miniMode=false;
	defaultWidth=800;
	defaultHeight=600;
	defaultPack="16m-jpeg";
	defaultQuality=9;
	defaultLayout=tr ( "us" );
	defaultKbdType=tr ( "pc105/us" );
	defaultCmd="KDE";
	defaultSshPort=sshPort=clientSshPort="22";


	setWindowTitle ( tr ( "X2Go Client" ) );
	ld=0;
	tunnel=0l;
	sndTunnel=0l;
	selectSessionDlg=0l;
	nxproxy=0l;
	artsd=0l;
	scDaemon=0l;
	gpgAgent=0l;
	gpg=0l;
	restartResume=false;
	isPassShown=true;
	sessionStatusDlg=0l;
	readExportsFrom=QString::null;
	passForm=0l;
	ldapOnly=false;
	hide();
	kdeIconsPath=getKdeIconsPath();

	addToAppNames ( "WWWBROWSER",tr ( "Internet Browser" ) );
	addToAppNames ( "MAILCLIENT",tr ( "Email Client" ) );
	addToAppNames ( "OFFICE",tr ( "OpenOffice.org" ) );
	addToAppNames ( "TERMINAL",tr ( "Terminal" ) );





#ifndef Q_OS_LINUX
	widgetExtraStyle =new QPlastiqueStyle();
#endif

	QDesktopWidget wd;
	x2goDebug<<"Desktop Geometry:"<<wd.screenGeometry();
	if ( wd.screenGeometry().width() <1024 || wd.screenGeometry().height() <768 )
	{
		miniMode=true;
		x2goDebug<<"Switching to \"mini\" Mode";
	}

	agentCheckTimer=new QTimer ( this );
	connect ( agentCheckTimer,SIGNAL ( timeout() ),this,SLOT ( slot_checkAgentProcess() ) );


	loadSettings();
	QStringList args=QCoreApplication::arguments();
	for ( int i=1;i<args.size();++i )
	{
		if ( !parseParam ( args[i] ) )
		{
			close();
			exit ( -1 );
		}
	}

	if ( readExportsFrom!=QString::null )
	{
		exportTimer=new QTimer ( this );
		connect ( exportTimer,SIGNAL ( timeout() ),this,SLOT ( slot_exportTimer() ) );
	}
	if ( extLogin )
	{
		extTimer=new QTimer ( this );
		extTimer->start ( 2000 );
		connect ( extTimer,SIGNAL ( timeout() ),this,SLOT ( slotExtTimer() ) );
	}

	if ( startMaximized )
	{
		QTimer::singleShot ( 10, this, SLOT ( slot_resize() ) );
	}

	if ( usePGPCard )
	{
		QTimer::singleShot ( 10, this, SLOT ( slot_startPGPAuth() ) );
	}


	//fr=new SVGFrame(QString::null,true,this);
	fr=new IMGFrame ( ( QImage* ) 0l,this );
	setCentralWidget ( fr );

#ifndef Q_WS_HILDON
	bgFrame=new SVGFrame ( ( QString ) ":/svg/bg.svg",true,fr );
#else
	bgFrame=new SVGFrame ( ( QString ) ":/svg/bg_hildon.svg",true,fr );
#endif
	//bgFrame=new SVGFrame((QString)"/home/admin/test.svg",false,fr);


	SVGFrame* x2g=new SVGFrame ( ( QString ) ":/svg/x2gologo.svg",false,fr );

	QPalette pl=x2g->palette();
	pl.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
	pl.setColor ( QPalette::Window, QColor ( 255,255,255,0 ) );
	x2g->setPalette ( pl );

	SVGFrame* on=new SVGFrame ( ( QString ) ":/svg/onlogo.svg",false,fr );
	on->setPalette ( pl );

	if ( !miniMode )
	{
		x2g->setFixedSize ( 100,100 );
		on->setFixedSize ( 100,80 );
	}
	else
	{
		x2g->setFixedSize ( 50,50 );
		on->setFixedSize ( 50,40 );
	}

	mainL=new QHBoxLayout ( fr );

	ln=new SVGFrame ( ( QString ) ":/svg/line.svg",true,fr );
	ln->setFixedWidth ( ln->sizeHint().width() );
	uname=new QLineEdit ( bgFrame );
	setWidgetStyle ( uname );

	uname->hide();
	uname->setFrame ( false );
	u=new QLabel ( tr ( "Session:" ),bgFrame );
	u->hide();
	QFont fnt=u->font();
	fnt.setPointSize ( 16 );
#ifndef Q_WS_HILDON
	if ( miniMode )
	{
		fnt.setPointSize ( 12 );
	}
#endif

	u->setFont ( fnt );

	connect ( uname,SIGNAL ( returnPressed() ),this,SLOT ( slotUnameEntered() ) );

	QPalette pal=u->palette();
	pal.setColor ( QPalette::WindowText, QColor ( 200,200,200,255 ) );
	u->setPalette ( pal );
	uname->setFont ( fnt );
	pal=uname->palette();
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Text, QColor ( 200,200,200,255 ) );
	uname->setPalette ( pal );

	username=new QHBoxLayout();
	username->setSpacing ( 10 );
	username->addStretch();
	username->addStretch();


	QVBoxLayout* onlay=new QVBoxLayout();
	onlay->addStretch();
	onlay->addWidget ( on );

	QVBoxLayout* x2golay=new QVBoxLayout();
	x2golay->addStretch();
	x2golay->addWidget ( x2g );


	QHBoxLayout* bgLay=new QHBoxLayout ( bgFrame );
	bgLay->setSpacing ( 0 );
	bgLay->setMargin ( 0 );
	bgLay->addLayout ( onlay );
	bgLay->addStretch();
	bgLay->addLayout ( username );
	bgLay->addLayout ( x2golay );


	mainL->setSpacing ( 0 );
	mainL->setMargin ( 0 );
	mainL->addWidget ( bgFrame );
	mainL->addWidget ( ln );

	u->show();
	uname->show();

	users=new QScrollArea ( fr );
	pal=users->verticalScrollBar()->palette();
	pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
	pal.setBrush ( QPalette::Base, QColor ( 110,112,127,255 ) );
	pal.setBrush ( QPalette::Button, QColor ( 110,112,127,255 ) );
	users->verticalScrollBar()->setPalette ( pal );
	users->setFrameStyle ( QFrame::Plain );
	users->setFocusPolicy ( Qt::NoFocus );


	pal=users->palette();
	pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
	users->setPalette ( pal );
	users->setWidgetResizable ( true );

	uframe=new QFrame();
	users->setWidget ( uframe );
	mainL->addWidget ( users );


	connect ( fr,SIGNAL ( resized ( const QSize ) ),this,SLOT ( slot_resize ( const QSize ) ) );
	slot_resize ( fr->size() );


	QAction *act_exit=new QAction ( QIcon ( iconsPath ( "/32x32/exit.png" ) ),tr ( "&Quit" ),this );
	act_exit->setShortcut ( tr ( "Ctrl+Q" ) );
	act_exit->setStatusTip ( tr ( "Quit" ) );



	act_new=new QAction ( QIcon ( iconsPath ( "/32x32/new_file.png" ) ),tr ( "&New Session ..." ),this );
	act_new->setShortcut ( tr ( "Ctrl+N" ) );


	setWindowIcon ( QIcon ( ":icons/128x128/x2go.png" ) );
	act_edit=new QAction ( QIcon ( iconsPath ( "/32x32/edit.png" ) ),
	                       tr ( "Session Management..." ),this );
	act_edit->setShortcut ( tr ( "Ctrl+E" ) );


	QAction *act_set=new QAction ( QIcon ( iconsPath ( "/32x32/edit_settings.png" ) ),tr ( "&Settings ..." ),this );

	QAction *act_tb=new QAction ( tr ( "Show Toolbar" ),this );
	act_tb->setCheckable ( true );
	act_tb->setChecked ( showToolBar );


	QAction *act_abclient=new QAction ( QIcon ( ":icons/16x16/x2go.png" ),tr ( "About X2GO Client" ),this );

	QAction *act_abqt=new QAction ( tr ( "About Qt" ),this );

	connect ( act_set,SIGNAL ( triggered ( bool ) ),this,SLOT ( slot_config() ) );
	connect ( act_abclient,SIGNAL ( triggered ( bool ) ),this,SLOT ( slot_about() ) );
	connect ( act_abqt,SIGNAL ( triggered ( bool ) ),this,SLOT ( slot_about_qt() ) );
	connect ( act_new,SIGNAL ( triggered ( bool ) ),this,SLOT ( slotNewSession() ) );
	connect ( act_edit,SIGNAL ( triggered ( bool ) ),this,SLOT ( slot_manage() ) );
	connect ( act_exit,SIGNAL ( triggered ( bool ) ),this,SLOT ( close() ) );
	connect ( act_tb,SIGNAL ( toggled ( bool ) ),this,SLOT ( displayToolBar ( bool ) ) );
	QToolBar *stb=addToolBar ( tr ( "Session" ) );

	QShortcut* ex=new QShortcut ( QKeySequence ( tr ( "Ctrl+Q","exit" ) ),this );
	connect ( ex,SIGNAL ( activated() ),this,SLOT ( close() ) );

	if ( drawMenu )
	{
		QMenu* menu_sess=menuBar()->addMenu ( tr ( "&Session" ) );
		QMenu* menu_opts=menuBar()->addMenu ( tr ( "&Options" ) );

		menu_sess->addAction ( act_new );
		menu_sess->addAction ( act_edit );
		menu_sess->addSeparator();
		menu_sess->addAction ( act_exit );
		menu_opts->addAction ( act_set );
		menu_opts->addAction ( act_tb );

		QMenu* menu_help=menuBar()->addMenu ( tr ( "&Help" ) );
		menu_help->addAction ( act_abclient );
		menu_help->addAction ( act_abqt );

		stb->addAction ( act_new );
		stb->addAction ( act_edit );
		stb->addSeparator();
		stb->addAction ( act_set );

		if ( !showToolBar )
			stb->hide();
		connect ( act_tb,SIGNAL ( toggled ( bool ) ),stb,SLOT ( setVisible ( bool ) ) );
	}
	else
		stb->hide();

	QTimer::singleShot ( 1, this, SLOT ( slot_resize() ) );

#if !defined USELDAP

	useLdap=false;
#endif

	if ( useLdap )
	{
		act_new->setEnabled ( false );
		act_edit->setEnabled ( false );
		u->setText ( tr ( "Login:" ) );
		QTimer::singleShot ( 1500, this, SLOT ( readUsers() ) );
	}
	else
		QTimer::singleShot ( 1, this, SLOT ( slot_readSessions() ) );
	QTimer* t=new QTimer ( this );
	connect ( t,SIGNAL ( timeout() ),this,SLOT ( slot_rereadUsers() ) );
	t->start ( 20000 );
	hide();
}


ONMainWindow::~ONMainWindow()
{}


QString ONMainWindow::findTheme ( QString /*theme*/ )
{
	/*    QDir dr(QDir::homePath ()+"/.kde/share/icons/"+theme);
		if(dr.exists())
		return dr.absolutePath();

		QStringList lst=QProcess::systemEnvironment();
		QString kdedir;
		for (int i = 0; i < lst.size(); ++i)
		{
		if(lst.at(i).indexOf("KDEDIRS")==0)
		{
		kdedir=lst.at(i).split('=').at(1);
		break;
	}
	}
		if(!kdedir.isNull())
		{
		QStringList dirs=kdedir.split(':');
		for(int i=0; i< dirs.size(); ++i)
		{
		dr.setPath(dirs.at(i)+"/share/icons/"+theme);
		if(dr.exists())
		return dr.absolutePath();
	}
	}
		dr.setPath((QString)"/usr/share/icons/"+theme);
		if(dr.exists())
		return dr.absolutePath();

		dr.setPath((QString)"/usr/kde/3.5/share/icons/"+theme);
		if(dr.exists())
		return dr.absolutePath();

		dr.setPath((QString)"/opt/kde3/share/icons/"+theme);
		if(dr.exists())
		return dr.absolutePath();*/

	return QString::null;
}

QString ONMainWindow::getKdeIconsPath()
{
	/*    QSettings st(QDir::homePath ()+"/.kde/share/config/kdeglobals",QSettings::NativeFormat);
		QString theme="default.kde";
		if(st.contains("Icons/Theme"))
		{
		theme=st.value("Icons/Theme","default.kde").toString();
	}

		QString path=findTheme(theme);
		if(!path.isNull())
		{
		return path;
	}
		if(theme == "default.kde")
		return ":/icons";
		theme == "default.kde";
		path=findTheme(theme);
		if(!path.isNull())
		{
		return path;
	}*/
	return ":/icons";
}

void ONMainWindow::slot_resize ( const QSize sz )
{
	int height;
	int usize;
	height=sz.height();
	if ( !miniMode )
	{
 		usize=sz.width()-800;
 		if ( usize<360 )
			usize=360;
 		if ( usize>500 )
 			usize=500;
	}
	else
	{
// 		usize=sz.width()-648;
// 		if ( usize<285 )
			usize=285;
// 		if ( usize>300 )
// 			usize=300;
	}

	if ( users->width() !=usize )
	{
		users->setFixedWidth ( usize );
		if ( useLdap )
		{
			QList<UserButton*>::iterator it;
			QList<UserButton*>::iterator end=names.end();
			for ( it=names.begin();it!=end;it++ )
			{
				if ( !miniMode )
					( *it )->move ( ( usize-360 ) /2, ( *it )->pos().y() );
				else
					( *it )->move ( ( usize-250 ) /2, ( *it )->pos().y() );
			}
		}
		else
		{
			QList<SessionButton*>::iterator it;
			QList<SessionButton*>::iterator end=sessions.end();
			for ( it=sessions.begin();it!=end;it++ )
			{
				if ( !miniMode )
					( *it )->move ( ( usize-360 ) /2, ( *it )->pos().y() );
				else
					( *it )->move ( ( usize-250 ) /2, ( *it )->pos().y() );
			}
		}
	}
	u->setFixedWidth ( u->sizeHint().width() );

	int bwidth=bgFrame->width();
	int upos= ( bwidth-u->width() ) /2;
	if ( upos<0 )
		upos=0;
	int rwidth=bwidth- ( upos+u->width() +5 );
	if ( rwidth<0 )
		rwidth=1;
	uname->setMinimumWidth ( rwidth );
	u->move ( upos,height/2 );
	uname->move ( u->pos().x() +u->width() +5,u->pos().y() );

}

void ONMainWindow::closeEvent ( QCloseEvent* event )
{
	if ( !startMaximized )
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sizes",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sizes" );
#endif

		st.setValue ( "mainwindow/size",QVariant ( size() ) );
		st.setValue ( "mainwindow/pos",QVariant ( pos() ) );
		st.setValue ( "mainwindow/maximized",QVariant ( isMaximized() ) );
		st.sync();
	}
	QMainWindow::closeEvent ( event );
	if ( nxproxy!=0l )
	{
		if ( nxproxy->state() ==QProcess::Running )
			nxproxy->terminate();
	}
	if ( tunnel!=0l )
		delete tunnel;
	if ( sndTunnel!=0l )
		delete sndTunnel;
	if ( artsd )
		delete artsd;
	if ( gpgAgent!=0l )
	{
		if ( gpgAgent->state() ==QProcess::Running )
			gpgAgent->terminate();
	}
	if ( agentPid.length() >0 )
	{
		if ( checkAgentProcess() )
		{
			QStringList arg;
			arg<<"-9"<<agentPid;
			QProcess::execute ( "kill",arg );
		}
	}
}

void ONMainWindow::loadSettings()
{

#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sizes",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sizes" );
#endif

	mwSize=st.value ( "mainwindow/size", ( QVariant ) QSize ( 800,600 ) ).toSize();
	mwPos=st.value ( "mainwindow/pos", ( QVariant ) QPoint ( 20,20 ) ).toPoint();
	mwMax=st.value ( "mainwindow/maximized", ( QVariant ) false ).toBool();

#ifndef WINDOWS

	QSettings st1 ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st1 ( "Obviously Nice","x2goclient" );
	st1.beginGroup ( "settings" );
#endif

	if ( !ldapOnly )
	{
		useLdap=st1.value ( "LDAP/useldap", ( QVariant ) false ).toBool();
		ldapServer=st1.value ( "LDAP/server", ( QVariant ) "localhost" ).toString();
		ldapPort=st1.value ( "LDAP/port", ( QVariant ) 389 ).toInt();
		ldapDn=st1.value ( "LDAP/basedn", ( QVariant ) QString::null ).toString();
		ldapServer1=st1.value ( "LDAP/server1", ( QVariant ) QString::null ).toString();
		ldapPort1=st1.value ( "LDAP/port1", ( QVariant ) 0 ).toInt();
		ldapServer2=st1.value ( "LDAP/server2", ( QVariant ) QString::null ).toString();
		ldapPort2=st1.value ( "LDAP/port2", ( QVariant ) 0 ).toInt();
	}
	clientSshPort=st1.value ( "clientport", ( QVariant ) 22 ).toString();
	showToolBar=st1.value ( "toolbar/show", ( QVariant ) true ).toBool();

}
QString ONMainWindow::iconsPath ( QString fname )
{
	/*    QFile fl(this->kdeIconsPath+fname);
		if(fl.exists())
		return kdeIconsPath+fname;*/
	return ( QString ) ":/icons"+fname;
}

void ONMainWindow::displayUsers()
{

	QPixmap pix;
	if ( !miniMode )
		pix=QPixmap ( ":/png/ico.png" );
	else
		pix=QPixmap ( ":/png/ico_mini.png" );
	QPixmap foto=QPixmap ( iconsPath ( "/64x64/personal.png" ) );

	QPalette pal=palette();
	pal.setBrush ( QPalette::Window, QBrush ( pix ) );
	pal.setBrush ( QPalette::Base, QBrush ( pix ) );
	pal.setBrush ( QPalette::Button, QBrush ( pix ) );
	QFont fnt=font();
	fnt.setPointSize ( 12 );
	uframe->setFont ( fnt );
	QList<user>::iterator it;
	QList<user>::iterator end=userList.end();
	int i=0;
	for ( it=userList.begin();it!=end;it++ )
	{
		int val=i+1;
		UserButton* l;
		if ( ( *it ).foto.isNull() )
			l=new UserButton ( this, uframe, ( *it ).uid, ( *it ).name,foto,pal );
		else
			l=new UserButton ( this, uframe, ( *it ).uid, ( *it ).name, ( *it ).foto,pal );
		connect ( l,SIGNAL ( userSelected ( UserButton* ) ),this,SLOT ( slotSelectedFromList ( UserButton* ) ) );
		if ( !miniMode )
			l->move ( ( users->width()-360 ) /2,i*120+ ( val-1 ) *25+5 );
		else
			l->move ( ( users->width()-260 ) /2,i*120+ ( val-1 ) *25+5 );
		l->show();
		names.append ( l );
		i++;
	}
	int val=i;
	uframe->setFixedHeight ( val*120+ ( val-1 ) *25 );
	uname->setText ( "" );
	disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,SLOT ( slotSnameChanged ( const QString& ) ) );
	connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,SLOT ( slotUnameChanged ( const QString& ) ) );
}

void ONMainWindow::showPass ( UserButton* user )
{
	QPalette pal=users->palette();
	setUsersEnabled ( false );
	QString fullName;
	QPixmap foto;
	if ( user )
	{
		foto=user->foto();
		nick=user->username();
		fullName=user->fullName();
		user->hide();
		lastUser=user;
	}
	else
	{
		lastUser=0;
		foto.load ( iconsPath ( "/64x64/personal.png" ) );
		foto=foto.scaled ( 100,100 );
		nick=uname->text();
		fullName="User Unknown";
	}

	passForm = new SVGFrame ( ":/svg/passform.svg",false,bgFrame );
	setWidgetStyle ( passForm );

	passForm->hide();
	if ( !miniMode )
		passForm->setFixedSize ( passForm->sizeHint() );
	else
		passForm->setFixedSize ( 310,180 );


	username->addWidget ( passForm );
	pal=passForm->palette();
	pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
	passForm->setPalette ( pal );

	fotoLabel=new QLabel ( passForm );
	fotoLabel->hide();
	fotoLabel->setPixmap ( foto );

	QString text="<b>"+nick+"</b><br>("+fullName+")";
	nameLabel=new QLabel ( text,passForm );
	nameLabel->hide();
	login=new QLineEdit ( passForm );
	setWidgetStyle ( login );
	login->setText ( nick );
	login->hide();

	QFont fnt=passForm->font();
	if ( !miniMode )
		fnt.setPointSize ( 12 );
	else
		fnt.setPointSize ( 9 );
	nameLabel->setFont ( fnt );


	passPrompt=new QLabel ( tr ( "Password:" ),passForm );
	passPrompt->setFont ( fnt );

	pass=new QLineEdit ( passForm );
	setWidgetStyle ( pass );
	pass->setFrame ( false );
	fnt.setBold ( true );
	pass->setFont ( fnt );
	pass->setEchoMode ( QLineEdit::Password );
	pass->setFocus();


	passPrompt->hide();
	pass->hide();

	ok=new QPushButton ( tr ( "Ok" ),passForm );
	cancel=new QPushButton ( tr ( "Cancel" ),passForm );
	setWidgetStyle ( cancel );
	ok->hide();
	cancel->hide();

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );
	ok->setPalette ( pal );
	setWidgetStyle ( ok );
	cancel->setPalette ( pal );

	connect ( ok,SIGNAL ( clicked() ),this, SLOT ( slotPassEnter() ) );
	connect ( cancel,SIGNAL ( clicked() ),this, SLOT ( slotClosePass() ) );
	connect ( pass,SIGNAL ( returnPressed() ),this, SLOT ( slotPassEnter() ) );


	passForm->show();
	if ( !miniMode )
		fotoLabel->move ( 20,20 );
	else
		fotoLabel->move ( 10,10 );
	fotoLabel->setFixedSize ( 100,100 );
	nameLabel->setFixedSize ( nameLabel->sizeHint() );
	nameLabel->move ( 130,50 );


	ok->setFixedSize ( ok->sizeHint() );
	cancel->setFixedSize ( cancel->sizeHint() );

	ok->move ( ( passForm->width()- ( ok->width() +12+cancel->width() ) ) /2, passForm->height()-ok->height()-12 );
	cancel->move ( ok->pos().x() +ok->width() +12,ok->pos().y() );


	passPrompt->setFixedSize ( passPrompt->sizeHint() );
	int fotoB=fotoLabel->pos().y() +fotoLabel->height();
	passPrompt->move ( 20, ( ok->pos().y()-fotoB ) /2 - passPrompt->height() /2 + fotoB );
	pass->move ( passPrompt->pos().x() +10+passPrompt->width(),passPrompt->pos().y() );


	QTimer::singleShot ( 1,this,SLOT ( slot_showWidgets() ) );
}


void ONMainWindow::slot_showWidgets()
{
	passPrompt->show();
	pass->show();
	ok->show();
	cancel->show();
	fotoLabel->show();
	nameLabel->show();
	if ( !useLdap )
	{
		login->show();
		loginPrompt->show();
	}
	u->hide();
	uname->hide();
}


void ONMainWindow::slotSelectedFromList ( UserButton* user )
{
	showPass ( user );
}

void ONMainWindow::slotClosePass()
{
	passForm->close();
	u->show();
	uname->show();
	if ( useLdap )
	{
		if ( lastUser )
		{
			lastUser->show();
			uname->setText ( lastUser->username() );
		}
	}
	else
	{
		lastSession->show();
		uname->setText ( lastSession->name() );
	}
	uname->setEnabled ( true );
	u->setEnabled ( true );
	setUsersEnabled ( true );
	uname->selectAll();
	uname->setFocus();
	passForm=0;
}


void ONMainWindow::slotPassEnter()
{
#if defined ( WINDOWS ) || defined (Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
		return;
#endif
#ifdef USELDAP

	if ( ! initLdapSession() )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );

		slot_config();
		return;
	}

	passForm->setEnabled ( false );

	x2goServers.clear();

	list<string> attr;
	attr.push_back ( "cn" );
	attr.push_back ( "serialNumber" );
	list<LDAPStringEntry> res;
	QString searchBase="ou=Servers,ou=ON,"+ldapDn;

	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,"objectClass=ipHost",res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}
	if ( res.size() ==0 )
	{
		QString message=tr ( "no X2Go Server found in LDAP " );
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}


	list<LDAPStringEntry>::iterator it=res.begin();
	list<LDAPStringEntry>::iterator end=res.end();
	QString freeServer;
	for ( ;it!=end;++it )
	{
		serv server;
		server.name=LDAPSession::getStringAttrValues ( *it,"cn" ).front().c_str();
		QString sFactor="1";
		list<string> serialNumber=LDAPSession::getStringAttrValues ( *it,"serialNumber" );
		if ( serialNumber.size() >0 )
		{
			sFactor=serialNumber.front().c_str();
		}
		x2goDebug<<server.name<<": factor is "<<sFactor;
		server.factor=sFactor.toFloat();
		server.sess=0;
		server.connOk=true;
		x2goServers.append ( server );
	}
	if ( ld )
	{
		delete ld;
		ld=0;
	}
	setEnabled ( false );
	passForm->setEnabled ( false );

	QString passwd;
	if ( !extLogin )
		currentKey=QString::null;
	QString user=login->text();
	QString host=ldapServer;
	passwd=pass->text();
	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gogetservers",
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_getServers ( false,message,0 );
		return;
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_getServers ( bool, QString,sshProcess* ) ) );
	connect ( proc,SIGNAL ( sudoConfigError ( QString,sshProcess* ) ),
	          this,SLOT ( slot_sudoErr ( QString,sshProcess* ) ) );
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
		cardStarted=true;
	}

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_getServers ( false,message,0 );
		return;
	}
	return;
#endif
}


void ONMainWindow::slotUnameChanged ( const QString& text )
{
	if ( prevText==text )
		return;
	if ( text=="" )
		return;
	QList<UserButton*>::iterator it;
	QList<UserButton*>::iterator endit=names.end();
	for ( it=names.begin();it!=endit;it++ )
	{
		QString username= ( *it )->username();
		if ( username.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
		{
			QPoint pos= ( *it )->pos();
			uname->setText ( username );
			QScrollBar* bar=users->verticalScrollBar();
			int docLang=bar->maximum()-bar->minimum() +bar->pageStep();
			double position= ( double ) ( pos.y() ) / ( double ) ( uframe->height() );
			bar->setValue ( ( int ) ( docLang*position-height() /2+ ( *it )->height() /2 ) );
			uname->setSelection ( username.length(),text.length()-username.length() );
			break;
		}
	}
	prevText=text;
}

void ONMainWindow::slotUnameEntered()
{
	QString text=uname->text();
	if ( useLdap )
	{
		UserButton* user=NULL;
		QList<UserButton*>::iterator it;
		QList<UserButton*>::iterator endit=names.end();
		for ( it=names.begin();it!=endit;it++ )
		{
			QString username= ( *it )->username();
			if ( username==text )
			{
				user=*it;
				break;
			}
		}
		showPass ( user );
	}
	else
	{
		SessionButton* sess=NULL;
		QList<SessionButton*>::iterator it;
		QList<SessionButton*>::iterator endit=sessions.end();
		for ( it=sessions.begin();it!=endit;it++ )
		{
			QString name= ( *it )->name();
			if ( name==text )
			{
				sess=*it;
				break;
			}
		}
		if ( sess )
			slotSelectedFromList ( sess );
	}
}


void ONMainWindow::readUsers()
{
#ifdef USELDAP
	if ( ! initLdapSession() )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );

		slot_config();
		return;
	}


	list<string> attr;
	attr.push_back ( "uidNumber" );
	attr.push_back ( "uid" );
	attr.push_back ( "cn" );
	attr.push_back ( "jpegPhoto" );


	list<LDAPBinEntry> result;
	try
	{
		ld->binSearch ( ldapDn.toStdString(),attr,"objectClass=posixAccount",result );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,QMessageBox::NoButton );
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}

	list<LDAPBinEntry>::iterator it=result.begin();
	list<LDAPBinEntry>::iterator end=result.end();

	for ( ;it!=end;++it )
	{
		user u;
		QString uin=LDAPSession::getBinAttrValues ( *it,"uidNumber" ).front().getData();
		u.uin=uin.toUInt();
		if ( u.uin<firstUid || u.uin>lastUid )
		{
			continue;
		}
		u.uid=LDAPSession::getBinAttrValues ( *it,"uid" ).front().getData();
		u.name=u.name.fromUtf8 ( LDAPSession::getBinAttrValues ( *it,"cn" ).front().getData() );
		list<ByteArray> lst=LDAPSession::getBinAttrValues ( *it,"jpegPhoto" );
		if ( lst.size() )
		{
			u.foto.loadFromData ( ( const uchar* ) ( lst.front().getData() ),lst.front().length() );
		}
		userList.append ( u );
	}
	qSort ( userList.begin(),userList.end(),user::lessThen );
	delete ld;
	ld=0;
	displayUsers();
#endif
}


void ONMainWindow::slot_config()
{
	if ( !startMaximized )
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sizes",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sizes" );
#endif

		st.setValue ( "mainwindow/size",QVariant ( size() ) );
		st.setValue ( "mainwindow/pos",QVariant ( pos() ) );
		st.sync();
	}
	if ( ld )
		delete ld;
	ld=0;

	ConfigDialog dlg ( this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		int i;

		if ( passForm )
			slotClosePass();
		if ( sessionStatusDlg )
		{
			if ( nxproxy )
			{
				isPassShown=false;
				nxproxy->terminate();
			}
		}

		for ( i=0;i<names.size();++i )
			names[i]->close();
		for ( i=0;i<sessions.size();++i )
			sessions[i]->close();

		userList.clear();
		sessions.clear();


		loadSettings();
		if ( useLdap )
		{
			act_new->setEnabled ( false );
			act_edit->setEnabled ( false );
			u->setText ( tr ( "Login:" ) );
			QTimer::singleShot ( 1, this, SLOT ( readUsers() ) );
		}
		else
		{
			act_new->setEnabled ( true );
			act_edit->setEnabled ( true );
			u->setText ( tr ( "Session:" ) );
			QTimer::singleShot ( 1, this, SLOT ( slot_readSessions() ) );
		}
		slot_resize ( fr->size() );
	}
}

void ONMainWindow::slot_edit ( SessionButton* bt )
{
	EditConnectionDialog dlg ( bt->id(),this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		bt->redraw();
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
	}
}


void ONMainWindow::slot_readSessions()
{
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QStringList slst=st.childGroups();
	for ( int i=0;i<slst.size();++i )
		createBut ( slst[i] );
	placeButtons();
	if ( slst.size() ==0 )
		slotNewSession();
	uname->setText ( "" );
	disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,SLOT ( slotUnameChanged ( const QString& ) ) );
	connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,SLOT ( slotSnameChanged ( const QString& ) ) );
}


void ONMainWindow::slotNewSession()
{
	QString id=QDateTime::currentDateTime().toString ( "yyyyMMddhhmmsszzz" );
	EditConnectionDialog dlg ( id, this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		SessionButton* bt=createBut ( id );
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
	}
}

void ONMainWindow::slot_manage()
{
	SessionManageDialog dlg ( this );
	dlg.exec();
}

SessionButton* ONMainWindow::createBut ( const QString& id )
{
	SessionButton* l;
	l=new SessionButton ( this,uframe,id );
	sessions.append ( l );
	connect ( l,SIGNAL ( signal_edit ( SessionButton* ) ),
	          this,SLOT ( slot_edit ( SessionButton* ) ) );

	connect ( l,SIGNAL ( signal_remove ( SessionButton* ) ),
	          this,SLOT ( slotDeleteButton ( SessionButton* ) ) );

	connect ( l,SIGNAL ( sessionSelected ( SessionButton* ) ),this,
	          SLOT ( slotSelectedFromList ( SessionButton* ) ) );

	return l;
}


void ONMainWindow::placeButtons()
{
	qSort ( sessions.begin(),sessions.end(),SessionButton::lessThen );
	for ( int i=0;i<sessions.size();++i )
	{
		if ( !miniMode )
			sessions[i]->move ( ( users->width()-360 ) /2,i*220+i*25+5 );
		else
			sessions[i]->move ( ( users->width()-260 ) /2,i*155+i*20+5 );
		sessions[i]->show();
	}
	if ( sessions.size() )
	{
		if ( !miniMode )
			uframe->setFixedHeight ( sessions.size() *220+ ( sessions.size()-1 ) *25 );
		else
			uframe->setFixedHeight ( sessions.size() *155+ ( sessions.size()-1 ) *20 );
	}
}

void ONMainWindow::slotDeleteButton ( SessionButton * bt )
{
	if ( QMessageBox::warning ( this,bt->name(),tr ( "Are you sure you want to delete this Session?" ),
	                            QMessageBox::Yes,QMessageBox::No ) !=QMessageBox::Yes )
		return;
#ifndef WINDOWS

	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	st.beginGroup ( bt->id() );
	st.remove ( "" );
	st.sync();
	sessions.removeAll ( bt );
	bt->close();
	placeButtons();
	users->ensureVisible ( 0,0,50,220 );
}


void ONMainWindow::displayToolBar ( bool show )
{
#ifndef WINDOWS
	QSettings st1 ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st1 ( "Obviously Nice","x2goclient" );
	st1.beginGroup ( "settings" );
#endif

	st1.setValue ( "toolbar/show",show );
	st1.sync();
}


bool ONMainWindow::initLdapSession ( bool showError )
{
#ifdef USELDAP
	x2goDebug<<"initing LDAP Session"<<endl;
	try
	{
		ld=new LDAPSession ( ldapServer.toStdString(),ldapPort,"","",true,false );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption0 in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		x2goDebug <<message<<endl;
		if ( ldapServer1.length() )
		{
			try
			{
				ld=new LDAPSession ( ldapServer1.toStdString(),ldapPort1,"","",true,false );
			}
			catch ( LDAPExeption e )
			{
				QString message="Exeption1 in: ";
				message=message+e.err_type.c_str();
				message=message+" : "+e.err_str.c_str();
				x2goDebug <<message<<endl;
				if ( ldapServer2.length() )
				{
					try
					{
						ld=new LDAPSession ( ldapServer2.toStdString(),ldapPort2,"","",true,false );
					}
					catch ( LDAPExeption e )
					{
						QString message="Exeption2 in: ";
						message=message+e.err_type.c_str();
						message=message+" : "+e.err_str.c_str();
						x2goDebug <<message<<endl;
						x2goDebug<<"return false"<<endl;
						if ( showError )
							QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
							                        QMessageBox::NoButton );

						return false;
					}
				}
				else
				{
					x2goDebug<<"return false"<<endl;
					if ( showError )
						QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
						                        QMessageBox::NoButton );

					return false;
				}

			}
		}
		else
		{
			x2goDebug<<"return false"<<endl;
			if ( showError )
				QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
				                        QMessageBox::NoButton );

			return false;
		}

	}
	sessionCmd="/usr/bin/startkde";
	netSound="ARTS_SERVER";
	startSound=false;
	firstUid=0;
	lastUid=65535;


	list<string> attr;
	attr.push_back ( SESSIONCMD );
	attr.push_back ( FIRSTUID );
	attr.push_back ( LASTUID );

	list<LDAPStringEntry> res;
	QString searchBase="ou=Settings,ou=ON,"+ldapDn;
	QString srch="cn=session settings";
	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,srch.toStdString(),res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,QMessageBox::NoButton );
		return false;
	}

	if ( res.size() !=0 )
	{
		LDAPStringEntry entry=res.front();
		list<string> str=LDAPSession::getStringAttrValues ( entry,SESSIONCMD );
		if ( str.size() )
		{
			sessionCmd=str.front().c_str();
		}
		str=LDAPSession::getStringAttrValues ( entry,FIRSTUID );
		if ( str.size() )
		{
			firstUid= ( ( QString ) str.front().c_str() ).toInt();
		}
		str=LDAPSession::getStringAttrValues ( entry,LASTUID );
		if ( str.size() )
		{
			lastUid= ( ( QString ) str.front().c_str() ).toInt();
		}
	}

	attr.clear();
	res.clear();
	attr.push_back ( NETSOUND );
	attr.push_back ( SNDSUPPORT );

	srch="cn=sound settings";
	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,srch.toStdString(),res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,QMessageBox::NoButton );
		return false;
	}

	if ( res.size() !=0 )
	{
		LDAPStringEntry entry=res.front();
		list<string> str=LDAPSession::getStringAttrValues ( entry,NETSOUND );
		if ( str.size() )
		{
			netSound=str.front().c_str();
		}
		str=LDAPSession::getStringAttrValues ( entry,SNDSUPPORT );
		if ( str.size() )
		{
			startSound= ( str.front() =="yes" ) ?true:false;
		}
	}
#endif
	return true;

}



void ONMainWindow::slotSnameChanged ( const QString& text )
{
	if ( prevText==text )
		return;
	if ( text=="" )
		return;
	QList<SessionButton*>::iterator it;
	QList<SessionButton*>::iterator endit=sessions.end();
	for ( it=sessions.begin();it!=endit;it++ )
	{
		QString name= ( *it )->name();
		if ( name.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
		{
			QPoint pos= ( *it )->pos();
			uname->setText ( name );
			QScrollBar* bar=users->verticalScrollBar();
			int docLang=bar->maximum()-bar->minimum() +bar->pageStep();
			double position= ( double ) ( pos.y() ) / ( double ) ( uframe->height() );
			bar->setValue ( ( int ) ( docLang*position-height() /2+ ( *it )->height() /2 ) );
			uname->setSelection ( name.length(),text.length()-name.length() );
			break;
		}
	}
	prevText=text;
}


void ONMainWindow::slotSelectedFromList ( SessionButton* session )
{
	x2goDebug <<"Selected from list"<<endl;
	lastSession=session;
	QPalette pal=users->palette();
	setUsersEnabled ( false );

	session->hide();

	QString sid=session->id();
#ifndef WINDOWS

	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QString sessIcon=st.value ( sid+"/icon",
	                            ( QVariant ) ":icons/128x128/x2gosession.png" ).toString();


	QString command=st.value ( sid+"/command",
	                           ( QVariant ) tr ( "KDE" ) ).toString();
	selectedCommand=command;
	command=transAppName ( command );

	QString server=st.value ( sid+"/host", ( QVariant ) QString::null ).toString();
	QString userName=st.value ( sid+"/user", ( QVariant ) QString::null ).toString();
	sshPort=st.value ( sid+"/sshport", ( QVariant ) defaultSshPort ).toString();


	passForm = new SVGFrame ( ":/svg/passform.svg",false,bgFrame );
	QFont fnt=passForm->font();
	if ( !miniMode )
		fnt.setPointSize ( 12 );
	else
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	passForm->setFont ( fnt );
	setWidgetStyle ( passForm );
	passForm->hide();
	if ( !miniMode )
		passForm->setFixedSize ( passForm->sizeHint() );
	else
		passForm->setFixedSize ( 310,180 );

	username->addWidget ( passForm );
	pal=passForm->palette();
	pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
	passForm->setPalette ( pal );

	QPixmap pix ( sessIcon );
	fotoLabel=new QLabel ( passForm );
	fotoLabel->hide();
	if ( !miniMode )
	{
		fotoLabel->setPixmap ( pix.scaled ( 64,64,Qt::IgnoreAspectRatio,Qt::SmoothTransformation ) );
		fotoLabel->setFixedSize ( 64,64 );
	}
	else
	{
		fotoLabel->setPixmap ( pix.scaled ( 48,48,Qt::IgnoreAspectRatio,Qt::SmoothTransformation ) );
		fotoLabel->setFixedSize ( 48,48 );
	}



	QString text="<b>"+session->name() +"</b><br>"+command+tr ( " on " ) +server;
	nameLabel=new QLabel ( text,passForm );
	nameLabel->hide();


	loginPrompt=new QLabel ( tr ( "Login:" ),passForm );
	passPrompt=new QLabel ( tr ( "Password:" ),passForm );


	login=new QLineEdit ( passForm );
	setWidgetStyle ( login );
	login->setFrame ( false );

	login->hide();
	loginPrompt->hide();

	pass=new QLineEdit ( passForm );
	setWidgetStyle ( pass );
	pass->setFrame ( false );
	fnt.setBold ( true );
	pass->setFont ( fnt );
	pass->setEchoMode ( QLineEdit::Password );
	pass->setFocus();


	pass->hide();
	passPrompt->hide();


	ok=new QPushButton ( tr ( "Ok" ),passForm );
	setWidgetStyle ( ok );
	cancel=new QPushButton ( tr ( "Cancel" ),passForm );
	setWidgetStyle ( cancel );
	ok->hide();
	cancel->hide();

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );
	ok->setPalette ( pal );
	cancel->setPalette ( pal );


	fotoLabel->move ( 20,20 );
	nameLabel->setFixedSize ( nameLabel->sizeHint() );
	if ( !miniMode )
		nameLabel->move ( 100,30 );
	else
		nameLabel->move ( 80,20 );



#ifndef Q_WS_HILDON
	ok->setFixedSize ( ok->sizeHint() );
	cancel->setFixedSize ( cancel->sizeHint() );
#else
	QSize sz=cancel->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	cancel->setFixedSize ( sz );
	sz=ok->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	ok->setFixedSize ( sz );
#endif

	ok->move ( ( passForm->width()- ( ok->width() +12+cancel->width() ) ) /2,
	           passForm->height()-ok->height()-12 );
	cancel->move ( ok->pos().x() +ok->width() +12,ok->pos().y() );


	int fotoB=fotoLabel->pos().y() +fotoLabel->height();
	int d=ok->y()-fotoB;

	loginPrompt->setFixedSize ( loginPrompt->sizeHint() );
	loginPrompt->move ( 20,fotoB+ ( d- ( loginPrompt->height() *2 ) ) /2 );

	int lB=loginPrompt->pos().y() +loginPrompt->height();

	passPrompt->setFixedSize ( passPrompt->sizeHint() );
	if ( !miniMode )
		passPrompt->move ( 20,lB+7 );
	else
		passPrompt->move ( 20,lB+3 );

	int editPos=loginPrompt->pos().x() +10;
	editPos+= ( loginPrompt->width() >passPrompt->width() ) ?loginPrompt->width() :passPrompt->width();

	login->move ( editPos,loginPrompt->pos().y() );
	pass->move ( editPos,passPrompt->pos().y() );

	login->setFixedHeight ( loginPrompt->height() );
	pass->setFixedHeight ( loginPrompt->height() );

	if ( !miniMode )
	{
		login->setFixedWidth ( passForm->width()-120-login->x() );
		pass->setFixedWidth ( passForm->width()-120-login->x() );
	}
	else
	{
		login->setFixedWidth ( passForm->width()-60-login->x() );
		pass->setFixedWidth ( passForm->width()-60-login->x() );
	}


	pal.setColor ( QPalette::Base, QColor ( 239,239,239,255 ) );
	login->setPalette ( pal );
	pass->setPalette ( pal );

	login->setText ( userName );


	passForm->show();

	connect ( ok,SIGNAL ( clicked() ),this, SLOT ( slotSessEnter() ) );
	connect ( cancel,SIGNAL ( clicked() ),this, SLOT ( slotClosePass() ) );
	connect ( pass,SIGNAL ( returnPressed() ),this, SLOT ( slotSessEnter() ) );
	connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( selectAll() ) );
	connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( setFocus() ) );

	QTimer::singleShot ( 1,this,SLOT ( slot_showWidgets() ) );

	currentKey=st.value ( sid+"/key",
	                      ( QVariant ) QString::null ).toString();
	if ( currentKey != QString::null && currentKey != "" )
		slotSessEnter();
	if ( cardReady )
	{
		login->setText ( cardLogin );
		slotSessEnter();
	}
}


void ONMainWindow::slotSessEnter()
{
#if defined ( WINDOWS ) || defined (Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
		return;
#endif

	QString sid=lastSession->id();
	startSession ( sid );
}


bool ONMainWindow::startSession ( const QString& sid )
{
	setEnabled ( false );
	passForm->setEnabled ( false );
#ifndef WINDOWS

	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QString passwd;
	QString user=login->text();
	QString host=st.value ( sid+"/host",
	                        ( QVariant ) QString::null ).toString();
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}

	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "export HOSTNAME && x2golistsessions",
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_listSessions ( false,message,0 );
		return false;
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_listSessions ( bool, QString,sshProcess* ) ) );
	connect ( proc,SIGNAL ( sudoConfigError ( QString,sshProcess* ) ),
	          this,SLOT ( slot_sudoErr ( QString,sshProcess* ) ) );

	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
		cardStarted=true;
	}
	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_listSessions ( false,message,0 );
		return false;
	}
	return true;
}


void ONMainWindow::slot_listSessions ( bool result,QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		pass->setFocus();
		pass->selectAll();
		return;
	}

	passForm->hide();
	setUsersEnabled ( false );
	uname->setEnabled ( false );
	u->setEnabled ( false );
	QStringList sessions=output.trimmed().split ( '\n' );
	if ( sessions.size() ==1&&sessions[0].length() <5 )
		startNewSession();
	else if ( sessions.size() ==1 )
	{
		x2goSession s=getSessionFromString ( sessions[0] );
		QDesktopWidget wd;
		if ( s.status=="S" && isColorDepthOk ( wd.depth(),s.colorDepth )
		        &&s.command == selectedCommand )
			resumeSession ( s );
		else
			selectSession ( sessions );
	}
	else
		selectSession ( sessions );
}


x2goSession ONMainWindow::getSessionFromString ( const QString& string )
{
	QStringList lst=string.split ( '|' );
	x2goSession s;
	s.agentPid=lst[0];
	s.sessionId=lst[1];
	s.display=lst[2];
	s.server=lst[3];
	s.status=lst[4];
	s.crTime=lst[5];
	s.cookie=lst[6];
	s.clientIp=lst[7];
	s.grPort=lst[8];
	s.sndPort=lst[9];
	s.colorDepth=0;
	if ( s.sessionId.indexOf ( "_dp" ) !=-1 )
	{
		s.colorDepth=s.sessionId.split ( "_dp" ) [1].toInt();
	}
	s.sessionType=x2goSession::DESKTOP;
	s.command=tr ( "unknown" );
	if ( s.sessionId.indexOf ( "_st" ) !=-1 )
	{
		QString cmdinfo=s.sessionId.split ( "_st" ) [1];
		cmdinfo=cmdinfo.split ( "_" ) [0];
		QChar st=cmdinfo[0];
		if ( st=='R' )
			s.sessionType=x2goSession::ROOTLESS;
		if ( st=='S' )
			s.sessionType=x2goSession::SHADOW;
		QString command=cmdinfo.mid ( 1 );
		if ( command.length() >0 )
			s.command=command;
	}
	return s;
}


void ONMainWindow::startNewSession()
{
	newSession=true;
	QString passwd=pass->text();
	QString user=login->text();

	QString pack;
	bool fullscreen;
	int height;
	int width;
	int quality;
	int speed;
	bool usekbd;
	bool rootless=false;
	QString layout;
	QString type;
	QString command;
	QString host=QString::null;

	if ( useLdap )
	{
		pack=defaultPack;
		fullscreen=defaultFullscreen;
		height=defaultHeight;
		width=defaultWidth;
		quality=defaultQuality;
		speed=defaultLink;
		usekbd=defaultSetKbd;
		layout=defaultLayout;
		type=defaultKbdType;
		command=defaultCmd;
		for ( int j=0;j<x2goServers.size();++j )
		{
			if ( x2goServers[j].connOk )
			{
				host=x2goServers[j].name;
				break;
			}
		}
		if ( host==QString::null )
		{
			QMessageBox::critical ( 0l,tr ( "Error" ),
			                        tr ( "No Server availabel" ),QMessageBox::Ok,
			                        QMessageBox::NoButton );
			return;
		}
	}
	else
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		command=st.value ( sid+"/command",
		                   ( QVariant ) defaultCmd ).toString();
		pack=st.value ( sid+"/pack",
		                ( QVariant ) defaultPack ).toString();
		host=st.value ( sid+"/host",
		                ( QVariant ) ( QString ) "localhost" ).toString();

		fullscreen=st.value ( sid+"/fullscreen",
		                      ( QVariant ) defaultFullscreen ).toBool();
		height=st.value ( sid+"/height",
		                  ( QVariant ) defaultHeight ).toInt();
		width=st.value ( sid+"/width",
		                 ( QVariant ) defaultWidth ).toInt();
		quality=st.value ( sid+"/quality",
		                   ( QVariant ) defaultQuality ).toInt();
		speed=st.value ( sid+"/speed",
		                 ( QVariant ) defaultLink ).toInt();
		usekbd=st.value ( sid+"/usekbd",
		                  ( QVariant ) defaultSetKbd ).toBool();
		layout=st.value ( sid+"/layout",
		                  ( QVariant ) defaultLayout ).toString();
		type=st.value ( sid+"/type",
		                ( QVariant ) defaultKbdType ).toString();
		rootless=st.value ( sid+"/rootless",
		                    ( QVariant ) false ).toBool();
	}



	resumingSession.server=host;

	QString geometry;
	if ( fullscreen )
		geometry="fullscreen";
	else
		geometry=QString::number ( width ) +"x"+QString::number ( height );
	QString link;
	switch ( speed )
	{
		case 0:
			link="modem";
			break;
		case 1:
			link="isdn";
			break;
		case 2:
			link="adsl";
			break;
		case 3:
			link="wan";
			break;
		case 4:
			link="lan";
			break;
	}

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
			if ( pc==pack )
			{
				pack+="-"+QString::number ( quality );
				break;
			}
		}
	}
	file.close();


	if ( selectSessionDlg )
	{
		slotCloseSelectDlg();
	}
	QDesktopWidget wd;
	QString depth=QString::number ( wd.depth() );
#ifdef Q_OS_DARWIN
	usekbd=0;
	type="query";
#endif
	QString sessTypeStr="D ";
	if ( rootless )
		sessTypeStr="R ";
	QString cmd="x2gostartagent "+geometry+" "+link+" "+pack+
	            " unix-kde-depth_"+depth+" "+layout+" "+type+" ";
	if ( usekbd )
		cmd += "1 ";
	else
		cmd += "0 ";
	QFileInfo f ( command );
	cmd+=sessTypeStr+f.fileName();

	sshProcess* proc=0l;

	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}

	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
	}
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retResumeSess ( bool, QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
		return;
	}
	passForm->hide();
}



void ONMainWindow::resumeSession ( const x2goSession& s )
{
	newSession=false;

	QString passwd=pass->text();
	QString user=login->text();
	QString host=s.server;

	QString pack;
	bool fullscreen;
	int height;
	int width;
	int quality;
	int speed;
	bool usekbd;
	QString layout;
	QString type;

	if ( useLdap )
	{
		pack=defaultPack;
		fullscreen=defaultFullscreen;
		height=defaultHeight;
		width=defaultWidth;
		quality=defaultQuality;
		speed=defaultLink;
		usekbd=defaultSetKbd;
		layout=defaultLayout;
		type=defaultKbdType;

	}
	else
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		pack=st.value ( sid+"/pack",
		                ( QVariant ) defaultPack ).toString();

		fullscreen=st.value ( sid+"/fullscreen",
		                      ( QVariant ) defaultFullscreen ).toBool();
		height=st.value ( sid+"/height",
		                  ( QVariant ) defaultHeight ).toInt();
		width=st.value ( sid+"/width",
		                 ( QVariant ) defaultWidth ).toInt();
		quality=st.value ( sid+"/quality",
		                   ( QVariant ) defaultQuality ).toInt();
		speed=st.value ( sid+"/speed",
		                 ( QVariant ) defaultLink ).toInt();
		usekbd=st.value ( sid+"/usekbd",
		                  ( QVariant ) defaultSetKbd ).toBool();
		layout=st.value ( sid+"/layout",
		                  ( QVariant ) defaultLayout ).toString();
		type=st.value ( sid+"/type",
		                ( QVariant ) defaultKbdType ).toString();
		host=st.value ( sid+"/host",
		                ( QVariant ) s.server ).toString();
	}
	QString geometry;
	if ( fullscreen )
		geometry="fullscreen";
	else
		geometry=QString::number ( width ) +"x"+QString::number ( height );
	QString link;
	switch ( speed )
	{
		case 0:
			link="modem";
			break;
		case 1:
			link="isdn";
			break;
		case 2:
			link="adsl";
			break;
		case 3:
			link="wan";
			break;
		case 4:
			link="lan";
			break;
	}

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
			if ( pc==pack )
			{
				pack+="-"+QString::number ( quality );
				break;
			}
		}
	}
	file.close();

#ifdef Q_OS_DARWIN
	usekbd=0;
	type="query";
#endif


	if ( selectSessionDlg )
	{
		slotCloseSelectDlg();
	}
	QString cmd="x2goresume-session "+s.sessionId+" "+geometry+" "+link+" "+pack+" "+layout+
	            " "+type+" ";
	if ( usekbd )
		cmd += "1";
	else
		cmd += "0";

	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
	}
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retResumeSess ( bool, QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
		return;
	}
	resumingSession=s;
	passForm->hide();
}


void ONMainWindow::selectSession ( const QStringList& sessions )
{

	selectSessionDlg = new SVGFrame ( ":/svg/passform.svg",false,bgFrame );
	setWidgetStyle ( selectSessionDlg );
	selectSessionDlg->hide();

	if ( !miniMode )
		selectSessionDlg->setFixedSize ( selectSessionDlg->sizeHint() );
	else
		selectSessionDlg->setFixedSize ( 310,180 );

	username->addWidget ( selectSessionDlg );
	QPalette pal=selectSessionDlg->palette();
	pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
	selectSessionDlg->setPalette ( pal );

	QFont fnt=selectSessionDlg->font();
	if ( miniMode )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	selectSessionDlg->setFont ( fnt );
	selectSessionLabel=new QLabel ( tr ( "Select session:" ), selectSessionDlg );
	sOk=new QPushButton ( tr ( "Resume" ),selectSessionDlg );
	setWidgetStyle ( sOk );
	sCancel=new QPushButton ( tr ( "Cancel" ),selectSessionDlg );
	setWidgetStyle ( sCancel );

	bSusp=new QPushButton ( tr ( "Suspend" ),selectSessionDlg );
	setWidgetStyle ( bSusp );
	bTerm=new QPushButton ( tr ( "Terminate" ),selectSessionDlg );
	setWidgetStyle ( bTerm );

	bNew=new QPushButton ( tr ( "New" ),selectSessionDlg );
	setWidgetStyle ( bNew );


	if ( !miniMode )
		selectSessionLabel->move ( 20,20 );
	else
		selectSessionLabel->move ( 10,10 );

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );
	sOk->setPalette ( pal );
	sCancel->setPalette ( pal );

	connect ( sCancel,SIGNAL ( clicked() ),this, SLOT ( slotCloseSelectDlg() ) );

	selectSessionDlg->show();
#ifndef Q_WS_HILDON
	sOk->setFixedSize ( ok->sizeHint() );
	sCancel->setFixedSize ( cancel->sizeHint() );
#else
	QSize sz=sCancel->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sCancel->setFixedSize ( sz );
	sz=sOk->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sOk->setFixedSize ( sz );
	sz=bSusp->sizeHint();
	if ( bTerm->sizeHint().width() > sz.width() )
		sz=bTerm->sizeHint();
	if ( bNew->sizeHint().width() > sz.width() )
		sz=bNew->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	bSusp->setFixedSize ( sz );
	bTerm->setFixedSize ( sz );
	bNew->setFixedSize ( sz );
#endif
	if ( !miniMode )
	{
		sOk->move ( ( selectSessionDlg->width()- ( sOk->width() +12+sCancel->width() ) ) /2,
		            selectSessionDlg->height()-sOk->height()-12 );
		sCancel->move ( sOk->pos().x() +sOk->width() +12,sOk->pos().y() );
	}
	else
	{
		sOk->move ( ( selectSessionDlg->width()- ( sOk->width() +8+sCancel->width() ) ) /2,
		            selectSessionDlg->height()-sOk->height()-8 );
		sCancel->move ( sOk->pos().x() +sOk->width() +8,sOk->pos().y() );
	}
	int bmaxw=bNew->size().width();
	if ( bSusp->size().width() >bmaxw )
		bmaxw=bSusp->size().width();
	if ( bTerm->size().width() >bmaxw )
		bmaxw=bTerm->size().width();

	bNew->setFixedWidth ( bmaxw );
	bSusp->setFixedWidth ( bmaxw );
	bTerm->setFixedWidth ( bmaxw );

	if ( !miniMode )
	{
		int butx=selectSessionDlg->width()-bmaxw-12;
		bSusp->move ( butx,45 );
		bTerm->move ( butx,bSusp->y() +bSusp->height() +6 );
	}
	else
	{
		int butx=selectSessionDlg->width()-bmaxw-9;
		bSusp->move ( butx,30 );
		bTerm->move ( butx,bSusp->y() +bSusp->height() +6 );
	}

	sOk->setEnabled ( true );
	sCancel->setEnabled ( true );
	selectSessionDlg->setEnabled ( true );
	setEnabled ( true );


	sessTv=new QTreeView ( selectSessionDlg );
	setWidgetStyle ( sessTv );
	setWidgetStyle ( sessTv->horizontalScrollBar() );
	sessTv->setItemsExpandable ( false );
	sessTv->setRootIsDecorated ( false );

	QStandardItemModel* model=new QStandardItemModel ( sessions.size(), 8 );

	model->setHeaderData ( S_DISPLAY,Qt::Horizontal,QVariant ( ( QString ) tr ( "Display" ) ) );
	model->setHeaderData ( S_STATUS,Qt::Horizontal,QVariant ( ( QString ) tr ( "Status" ) ) );
	model->setHeaderData ( S_COMMAND,Qt::Horizontal,QVariant ( ( QString ) tr ( "Command" ) ) );
	model->setHeaderData ( S_TYPE,Qt::Horizontal,QVariant ( ( QString ) tr ( "Type" ) ) );
	model->setHeaderData ( S_SERVER,Qt::Horizontal,QVariant ( ( QString ) tr ( "Server" ) ) );
	model->setHeaderData ( S_CRTIME,Qt::Horizontal,QVariant ( ( QString ) tr ( "Creation Time" ) ) );
	model->setHeaderData ( S_IP,Qt::Horizontal,QVariant ( ( QString ) tr ( "Client IP" ) ) );
	model->setHeaderData ( S_ID,Qt::Horizontal,QVariant ( ( QString ) tr ( "Session ID" ) ) );
	sessTv->setModel ( ( QAbstractItemModel* ) model );
	if ( !miniMode )
	{
		sessTv->move ( 20,45 );
		sessTv->setFixedSize ( bSusp->x()-24,sOk->y()-
		                       ( selectSessionLabel->y() +
		                         selectSessionLabel->height() +8 ) );
	}
	else
	{
		sessTv->move ( 10,30 );
		sessTv->setFixedSize ( bSusp->x()-13,sOk->y()-
		                       ( selectSessionLabel->y() +
		                         selectSessionLabel->height() +6 ) );
	}
	QFontMetrics fm ( sessTv->font() );
	selectedSessions.clear();
	for ( int row = 0; row < sessions.size(); ++row )
	{

		x2goSession s=getSessionFromString ( sessions[row] );
		selectedSessions.append ( s );
		QStandardItem *item;

		item= new QStandardItem ( s.display );
		model->setItem ( row,S_DISPLAY,item );

		if ( s.status=="R" )
			item= new QStandardItem ( tr ( "running" ) );
		else
			item= new QStandardItem ( tr ( "suspended" ) );
		model->setItem ( row,S_STATUS,item );

		item= new QStandardItem ( transAppName ( s.command ) );
		model->setItem ( row,S_COMMAND,item );

		QString type=tr ( "Desktop" );
		if ( s.sessionType==x2goSession::ROOTLESS )
			type=tr ( "single application" );
		if ( s.sessionType==x2goSession::SHADOW )
			type=tr ( "shadow session" );

		item= new QStandardItem ( type );
		model->setItem ( row,S_TYPE,item );

		item= new QStandardItem ( s.crTime );
		model->setItem ( row,S_CRTIME,item );
		item= new QStandardItem ( s.server );
		model->setItem ( row,S_SERVER,item );
		item= new QStandardItem ( s.clientIp );
		model->setItem ( row,S_IP,item );
		item= new QStandardItem ( s.sessionId );
		model->setItem ( row,S_ID,item );
		for ( int j=0;j<8;++j )
		{
			QString txt=model->index ( row,j ).data().toString();
			if ( sessTv->header()->sectionSize ( j ) < fm.width ( txt ) +6 )
			{
				sessTv->header()->resizeSection ( j,fm.width ( txt ) +6 );
			}
		}
	}
	sessTv->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	if ( !miniMode )
		bNew->move ( bTerm->x(),sessTv->y() +sessTv->height()-bNew->height() );
	else
		bNew->move ( bTerm->x(),sessTv->y() +sessTv->height()-bNew->height() );
	sessTv->setPalette ( pal );
	bNew->setPalette ( pal );
	bSusp->setPalette ( pal );
	bTerm->setPalette ( pal );
	//     sessTv->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
	sessTv->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
	sOk->setEnabled ( false );
	bSusp->setEnabled ( false );
	bTerm->setEnabled ( false );
	selectSessionLabel->hide();
	sOk->hide();
	sCancel->hide();
	bNew->hide();
	bSusp->hide();
	bTerm->hide();


	connect ( sessTv,SIGNAL ( clicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_activated ( const QModelIndex& ) ) );

	connect ( sessTv,SIGNAL ( doubleClicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_resumeDoubleClick ( const QModelIndex& ) ) );

	connect ( sOk,SIGNAL ( clicked() ),this, SLOT ( slotResumeSess() ) );
	connect ( bSusp,SIGNAL ( clicked() ),this, SLOT ( slotSuspendSess() ) );
	connect ( bTerm,SIGNAL ( clicked() ),this, SLOT ( slotTermSess() ) );
	connect ( bNew,SIGNAL ( clicked() ),this, SLOT ( slotNewSess() ) );


	QTimer::singleShot ( 1,this,SLOT ( slot_showSelectSessionWidgets() ) );
}

void ONMainWindow::slotCloseSelectDlg()
{
	selectSessionDlg->close();
	u->setEnabled ( true );
	uname->setEnabled ( true );
	slot_showPassForm();
	selectSessionDlg=0;
}


void ONMainWindow::slot_showSelectSessionWidgets()
{
	selectSessionLabel->show();
	sOk->show();
	sCancel->show();
	bNew->show();
	bSusp->show();
	bTerm->show();
	sessTv->show();
}

void ONMainWindow::slot_activated ( const QModelIndex& index )
{
	QString status=sessTv->model()->index ( index.row(),S_STATUS ).data().toString();
	if ( status==tr ( "running" ) )
	{
		bSusp->setEnabled ( true );
		sOk->setEnabled ( false );
	}
	else
	{
		bSusp->setEnabled ( false );
		sOk->setEnabled ( true );
	}
	bTerm->setEnabled ( true );
	if ( status==QString::null )
	{
		sOk->setEnabled ( false );
		bTerm->setEnabled ( false );
	}
}


void ONMainWindow::slotResumeSess()
{
	x2goSession s=getSelectedSession();
	QDesktopWidget wd;
	if ( isColorDepthOk ( wd.depth(),s.colorDepth ) )
		resumeSession ( s );
	else
	{
		QString depth=QString::number ( s.colorDepth );
		int res;
		if ( s.colorDepth==24 || s.colorDepth==32 )
		{
			res=QMessageBox::warning ( 0l,tr ( "Warning" ),tr ( "Your current color depth is different to the color depth of your x2go-session. This may cause problems reconnecting to this session and in most cases <b>you will loose the session</b> and have to start a new one! It's highly recommended to change the color depth of your Display to " ) +tr ( "24 or 32" ) +tr ( " bit and restart your X-server before you reconnect to this x2go-session.<br>Resume this session anyway?" ),tr ( "Yes" ),tr ( "No" ) );

		}
		else
		{
			res=QMessageBox::warning ( 0l,tr ( "Warning" ),tr ( "Your current color depth is different to the color depth of your x2go-session. This may cause problems reconnecting to this session and in most cases <b>you will loose the session</b> and have to start a new one! It's highly recommended to change the color depth of your Display to " ) +depth+tr ( " bit and restart your X-server before you reconnect to this x2go-session.<br>Resume this session anyway?" ),tr ( "Yes" ),tr ( "No" ) );
		}
		if ( res==0 )
			resumeSession ( s );
	}

}


void ONMainWindow::slotSuspendSess()
{

	QString passwd;
	QString user=login->text();
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}

	selectSessionDlg->setEnabled ( false );


	QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),S_ID ).data().toString();
	QString host=sessTv->model()->index ( sessTv->currentIndex().row(),S_SERVER ).data().toString();
	if ( !useLdap )
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		host=st.value ( sid+"/host",
		                ( QVariant ) host ).toString();
	}
	suspendSession ( user,host,passwd,currentKey,sessId );
}


void ONMainWindow::slotSuspendSessFromSt()
{
	QString passwd;
	QString user=login->text();
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}
	setStatStatus ( tr ( "suspending" ) );

	x2goDebug <<"disconnect export"<<endl;
	disconnect ( sbExp,SIGNAL ( clicked() ),this, SLOT ( slot_exportDirectory() ) );
	sbExp->setEnabled ( false );

	suspendSession ( user,resumingSession.server,passwd,currentKey,resumingSession.sessionId );
}

void ONMainWindow::slotTermSessFromSt()
{
	QString passwd;
	QString user=login->text();
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}
	x2goDebug <<"disconnect export"<<endl;
	disconnect ( sbExp,SIGNAL ( clicked() ),this, SLOT ( slot_exportDirectory() ) );
	sbExp->setEnabled ( false );

	setStatStatus ( tr ( "terminating" ) );
	termSession ( user,resumingSession.server,passwd,currentKey,resumingSession.sessionId );
}


void ONMainWindow::slot_retSuspSess ( bool result, QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	else
	{
		if ( selectSessionDlg )
		{
			( ( QStandardItemModel* ) ( sessTv->model() ) )->item ( sessTv->currentIndex().row(),
			        S_STATUS )->setData ( QVariant ( ( QString ) tr ( "suspended" ) ),Qt::DisplayRole );
			bSusp->setEnabled ( false );
			sOk->setEnabled ( true );

		}
	}
	if ( selectSessionDlg )
		selectSessionDlg->setEnabled ( true );
}



void ONMainWindow::slotTermSess()
{
	QString passwd;
	QString user=login->text();
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}

	selectSessionDlg->setEnabled ( false );


	QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),
	                                        S_ID ).data().toString();
	QString host=sessTv->model()->index ( sessTv->currentIndex().row(),
	                                      S_SERVER ).data().toString();

	if ( !useLdap )
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		host=st.value ( sid+"/host",
		                ( QVariant ) host ).toString();
	}

	termSession ( user,host,passwd,currentKey,sessId );
}


void ONMainWindow::slotNewSess()
{
	startNewSession();
}


void ONMainWindow::slot_retTermSess ( bool result,  QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	else
	{
		if ( selectSessionDlg )
		{
			sessTv->model()->removeRow ( sessTv->currentIndex().row() );
			slot_activated ( sessTv->currentIndex() );
		}
	}
	if ( selectSessionDlg )
		selectSessionDlg->setEnabled ( true );
}

void ONMainWindow::slot_retResumeSess ( bool result, QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		slot_showPassForm();
		return;
	}

	QString host;
	bool sound;

	if ( useLdap )
	{
		sound=startSound;
	}
	else
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		sound=st.value ( sid+"/sound",
		                 ( QVariant ) defaultUseSound ).toBool();
		QString sndsys=st.value ( sid+"/soundsystem",
		                          ( QVariant ) "arts" ).toString();
		if ( sndsys=="esd" )
			useEsd=true;
		else
			useEsd=false;
	}
	QString passwd=pass->text();
	QString user=login->text();

	if ( newSession )
	{
		QString sString=output.trimmed();
		sString.replace ( '\n','|' );
		host=resumingSession.server;
		resumingSession=getNewSessionFromString ( sString );
		resumingSession.server=host;
		resumingSession.crTime=QDateTime::currentDateTime().toString ( "dd.MM.yy HH:mm:ss" );
	}
	else
		host=resumingSession.server;
	if ( !useLdap )
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		host=st.value ( sid+"/host",
		                ( QVariant ) host ).toString();
		resumingSession.server=host;
	}
	try
	{
		tunnel=new sshProcess ( this,user,host,sshPort,
		                        QString::null,
		                        passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_tunnelFailed ( false,message,0 );
	}
	if ( cardReady )
	{
		QStringList env=tunnel->environment();
		env+=sshEnv;
		tunnel->setEnvironment ( env );
	}


	connect ( tunnel,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_tunnelFailed ( bool,  QString,sshProcess* ) ) );
	connect ( tunnel,SIGNAL ( sshTunnelOk() ),
	          this,SLOT ( slot_tunnelOk() ) );

	try
	{
		tunnel->startTunnel ( "localhost",resumingSession.grPort,resumingSession.grPort );
	}
	catch ( QString message )
	{
		slot_tunnelFailed ( false,message,0 );
		return;
	}
#if defined (WINDOWS) || defined (Q_OS_DARWIN)
	useEsd=true;
#endif

	if ( sound )
	{
		artsd=new QProcess ( this );
		QString acmd="artsd",ecmd="esd";
#ifdef Q_OS_DARWIN
		QStringList env = artsd->environment();
		QDir dir ( QApplication::applicationDirPath() );
		dir.cdUp();
		dir.cd ( "esd" );
		env.insert ( 0,"DYLD_LIBRARY_PATH="+dir.absolutePath() );
		artsd->setEnvironment ( env );
		ecmd="\""+dir.absolutePath() +"\"/esd";
#endif
		if ( artsCmd.length() >0 )
			acmd=artsCmd;
		if ( esdCmd.length() >0 )
			ecmd=esdCmd;
		if ( useEsd )
			artsd->start ( ecmd+" -tcp -nobeeps -bind localhost -port "+
			               resumingSession.sndPort );
		else
			artsd->start ( acmd+" -u -N -p "+resumingSession.sndPort );
		try
		{
			sndTunnel=new sshProcess ( this,user,host,sshPort,
			                           QString::null,
			                           passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			slot_sndTunnelFailed ( false,message,0 );
		}
		if ( cardReady )
		{
			QStringList env=sndTunnel->environment();
			env+=sshEnv;
			sndTunnel->setEnvironment ( env );
		}

		connect ( sndTunnel,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
		          this,SLOT ( slot_sndTunnelFailed ( bool, QString,sshProcess* ) ) );

		try
		{
			sndTunnel->startTunnel ( "localhost",resumingSession.sndPort,
			                         resumingSession.sndPort,true );
		}
		catch ( QString message )
		{
			slot_sndTunnelFailed ( false,message,0 );
			return;
		}
	}
	else
		sndTunnel=0l;
}



x2goSession ONMainWindow::getSelectedSession()
{
	QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),S_ID ).data().toString();
	for ( int i=0;i<selectedSessions.size();++i )
	{
		if ( selectedSessions[i].sessionId==sessId )
			return selectedSessions[i];
	}
	return selectedSessions[0]; //warning !!!!! undefined session
}


void ONMainWindow::slot_tunnelOk()
{
	showExport=false;
	QString nxroot=QDir::homePath() +"/.x2go";
	QString dirpath=nxroot+"/S-"+resumingSession.sessionId;
	QDir d ( dirpath );
	if ( !d.exists() )
		if ( !d.mkpath ( dirpath ) )
		{
			QString message=tr ( "Unable to create Folder:" ) +dirpath;
			QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
			                        QMessageBox::NoButton );
			slot_showPassForm();
			if ( tunnel )
				delete tunnel;
			if ( sndTunnel )
				delete sndTunnel;
			if ( artsd )
				delete artsd;
			tunnel=sndTunnel=0l;
			artsd=0l;
			nxproxy=0l;
			return;
		}
	QFile file ( dirpath+"/options" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to write File:" ) +dirpath+"/options";
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		slot_showPassForm();
		return;
	}

	QTextStream out ( &file );
#ifndef WINDOWS

	out << "nx/nx,root="<<nxroot<<",connect=localhost,cookie="<<resumingSession.cookie<<",port="
	<<resumingSession.grPort<<",errors="<<dirpath<<"/sessions:"<<resumingSession.display;
#else

	QString cygwinRoot=nxroot;

	cygwinRoot=transform2cygwinPath ( cygwinRoot,true );


	QString cygwinDir=cygwinRoot+"/S-"+resumingSession.sessionId;
	out << "nx/nx,root="<<cygwinRoot<<",connect=localhost,cookie="<<resumingSession.cookie<<",port="
	<<resumingSession.grPort<<",errors="<<cygwinDir<<"/sessions:"<<resumingSession.display;
#endif

	file.close();
	xmodExecuted=false;
	nxproxy=new QProcess;
	QStringList env = QProcess::systemEnvironment();
	QString x2golibpath="/usr/lib/x2go";
#if defined ( WINDOWS ) || defined ( Q_OS_DARWIN )
	int dispInd=-1;
#endif
	for ( int l=0;l<env.size();++l )
	{
		if ( env[l].indexOf ( "X2GO_LIB" ) ==0 )
		{
			x2golibpath=env[l].split ( "=" ) [1];
		}
#if defined ( WINDOWS ) || defined ( Q_OS_DARWIN )
		if ( env[l].indexOf ( "DISPLAY" ) ==0 )
		{
			dispInd=l;
		}
#endif

	}
	env << "LD_LIBRARY_PATH="+x2golibpath;
	env << "NX_CLIENT="+QCoreApplication::applicationFilePath ();
#ifdef Q_OS_DARWIN
	//setting /usr/X11/bin to find xauth
	env.insert ( 0,"PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin" );
#endif

#if defined ( WINDOWS ) || defined ( Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
	{
		slot_proxyerror ( QProcess::FailedToStart );
		return;
	}
	if ( dispInd==-1 )
	{
		env <<"DISPLAY=localhost:"+disp;
	}
	else
	{
		env[dispInd]="DISPLAY=localhost:"+disp;
	}
#endif
	nxproxy->setEnvironment ( env );

	connect ( nxproxy,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	          SLOT ( slot_proxyerror ( QProcess::ProcessError ) ) );
	connect ( nxproxy,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_proxyFinished ( int,QProcess::ExitStatus ) ) );
	connect ( nxproxy,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_proxyStderr() ) );
	connect ( nxproxy,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_proxyStdout() ) );
#ifndef WINDOWS

	QString proxyCmd="nxproxy -S nx/nx,options="+dirpath+"/options:"+
	                 resumingSession.display;
#ifdef Q_OS_DARWIN
	//run nxproxy from bundle
	QDir dir ( QApplication::applicationDirPath() );
	dir.cdUp();
	dir.cd ( "exe" );
	proxyCmd="\""+dir.absolutePath() +"/\""+proxyCmd;
#endif //Q_OS_DARWIN
	nxproxy->start ( proxyCmd );
#else

	nxproxy->start ( "nxproxy -S nx/nx,options="+cygwinDir+"/options:"+
	                 resumingSession.display );
#endif

	showSessionStatus();
	QTimer::singleShot ( 30000,this,SLOT ( slot_restartNxProxy() ) );

}

void ONMainWindow::slot_tunnelFailed ( bool result,  QString output,sshProcess* )
{
	if ( result==false )
	{
		QString message=tr ( "Unable to create SSL Tunnel:\n" ) +output;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		if ( tunnel )
			delete tunnel;
		if ( sndTunnel )
			delete sndTunnel;
		if ( artsd )
			delete artsd;
		tunnel=sndTunnel=0l;
		artsd=0l;
		nxproxy=0l;
		slot_showPassForm();
	}
}

void ONMainWindow::slot_sndTunnelFailed ( bool result,  QString output,sshProcess* )
{
	if ( result==false )
	{
		QString message=tr ( "Unable to create SSL Tunnel:\n" ) +output;
		QMessageBox::warning ( 0l,tr ( "Warning" ),message,QMessageBox::Ok,
		                       QMessageBox::NoButton );
		if ( sndTunnel )
			delete sndTunnel;
		sndTunnel=0l;
	}
}



void ONMainWindow::slot_proxyerror ( QProcess::ProcessError )
{
	slot_proxyFinished ( -1,QProcess::CrashExit );
}


void ONMainWindow::slot_proxyFinished ( int,QProcess::ExitStatus )
{
	if ( tunnel )
		delete tunnel;
	if ( sndTunnel )
		delete sndTunnel;
	if ( artsd )
		delete artsd;
	delete nxproxy;
	tunnel=sndTunnel=0l;
	artsd=0l;
	nxproxy=0l;
	if(!usePGPCard)
	   check_cmd_status();

	if ( readExportsFrom!=QString::null )
	{
		exportTimer->stop();
		if ( extLogin )
		{
			currentKey=QString::null;
		}
	}

	if ( !restartResume )
	{
		pass->setText ( "" );
		QTimer::singleShot ( 2000,this,SLOT ( slot_showPassForm() ) );
	}
	else
	{
		restartResume=false;
		if ( sessionStatusDlg )
		{
			sessionStatusDlg->close();
			sessionStatusDlg=0l;
		}
		resumeSession ( resumingSession );
	}
}


void ONMainWindow::slot_proxyStderr()
{
	QString reserr ( nxproxy->readAllStandardError() );
	stInfo->insertPlainText ( reserr );
	stInfo->ensureCursorVisible();
	if ( stInfo->toPlainText().indexOf ( "Connecting to remote host 'localhost:"+
	                                     resumingSession.grPort ) !=-1 )
		setStatStatus ( tr ( "connecting" ) );

	if ( stInfo->toPlainText().indexOf ( "Connection to remote proxy 'localhost:"+
	                                     resumingSession.grPort+"' established" ) !=-1 )
	{
		if ( newSession )
			setStatStatus ( tr ( "starting" ) );
		else
			setStatStatus ( tr ( "resuming" ) );
	}

	if ( stInfo->toPlainText().indexOf ( "Established X server connection" ) !=-1 )
	{
		setStatStatus ( tr ( "running" ) );
		disconnect ( sbSusp,SIGNAL ( clicked() ),this, SLOT ( slot_testSessionStatus() ) );
		disconnect ( sbSusp,SIGNAL ( clicked() ),this, SLOT ( slotSuspendSessFromSt() ) );
		connect ( sbSusp,SIGNAL ( clicked() ),this, SLOT ( slotSuspendSessFromSt() ) );
		if ( !showExport )
		{
			showExport=true;
			connect ( sbExp,SIGNAL ( clicked() ),this, SLOT ( slot_exportDirectory() ) );
			sbExp->setEnabled ( true );
			if ( !useLdap )
				exportDefaultDirs();
			if ( readExportsFrom!=QString::null )
			{
				exportTimer->start ( 2000 );
			}
		}
		sbSusp->setText ( tr ( "Suspend" ) );
		if ( newSession )
		{
			runCommand();
			newSession=false;
		}
#ifdef 	Q_WS_HILDON
		else
		{
			if ( !xmodExecuted )
			{
				xmodExecuted=true;
				QTimer::singleShot ( 2000, this, SLOT ( slot_execXmodmap() ) );
			}
		}
#endif
	}
	if ( stInfo->toPlainText().indexOf ( tr ( "Connection timeout, aborting" ) ) !=-1 )
		setStatStatus ( tr ( "aborting" ) );

}


void ONMainWindow::slot_proxyStdout()
{
	QString resout ( nxproxy->readAllStandardOutput() );
}


void ONMainWindow::slot_showPassForm()
{
	if ( sessionStatusDlg )
	{
		sessionStatusDlg->close();
		sessionStatusDlg=0l;
	}
	setEnabled ( true );
	if ( passForm  &&  isPassShown )
	{
		passForm->show();
		passForm->setEnabled ( true );
	}
	isPassShown=true;
	pass->setFocus();
	pass->selectAll();
	u->setEnabled ( true );
}


void ONMainWindow::showSessionStatus()
{
	setEnabled ( true );
	showExport=false;
	sessionStatusDlg = new SVGFrame ( ":/svg/passform.svg",false,bgFrame );
	sessionStatusDlg->hide();
	if ( !miniMode )
		sessionStatusDlg->setFixedSize ( sessionStatusDlg->sizeHint() );
	else
		sessionStatusDlg->setFixedSize ( 310,200 );
	QFont fnt=sessionStatusDlg->font();
	if ( miniMode )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	sessionStatusDlg->setFont ( fnt );

	username->addWidget ( sessionStatusDlg );
	QPalette pal=sessionStatusDlg->palette();
	pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
	sessionStatusDlg->setPalette ( pal );

	slName=new QLabel ( sessionStatusDlg );
	slName->setText (
	    tr ( "<b>Session ID:<br>Server:<br>Username:<br>Display:<br>Creation Time:<br>Status:</b>" ) );
	if ( !miniMode )
		slName->move ( 30,30 );
	else
		slName->move ( 15,10 );
	slName->setFixedSize ( slName->sizeHint() );
	slName->hide();

	QString f="dd.MM.yy HH:mm:ss";
	QDateTime dt=QDateTime::fromString ( resumingSession.crTime,f );
	dt=dt.addYears ( 100 );

	slVal=new QLabel ( sessionStatusDlg );
	slVal->setText ( resumingSession.sessionId+"\n"+resumingSession.server+"\n"+
	                 login->text() +"\n"+resumingSession.display+"\n"+dt.toString() +"\n"+tr ( "suspended" ) );

	if ( !miniMode )
		slVal->move ( 40+slName->width(),30 );
	else
		slVal->move ( 25+slName->width(),10 );
	slVal->hide();
	sbSusp=new QPushButton ( tr ( "Abort" ),sessionStatusDlg );
	setWidgetStyle ( sbSusp );
	sbTerm=new QPushButton ( tr ( "Terminate" ),sessionStatusDlg );
	setWidgetStyle ( sbTerm );
	sbExp=new QPushButton ( tr ( "Share Folder..." ),sessionStatusDlg );
	setWidgetStyle ( sbExp );
	sbAdv=new QCheckBox ( tr ( "Show Details" ),sessionStatusDlg );
	setWidgetStyle ( sbAdv );
	sbAdv->setFixedSize ( sbAdv->sizeHint() );
#ifndef Q_WS_HILDON
	sbSusp->setFixedSize ( sbSusp->sizeHint() );
	sbTerm->setFixedSize ( sbTerm->sizeHint() );
	sbExp->setFixedSize ( sbExp->sizeHint() );
#else
	QSize sz=sbSusp->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbSusp->setFixedSize ( sz );
	sz=sbExp->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbExp->setFixedSize ( sz );
	sz=sbTerm->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbTerm->setFixedSize ( sz );
#endif
	sbAdv->hide();
	sbSusp->hide();
	sbTerm->hide();
	sbExp->hide();


	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

	sbAdv->setPalette ( pal );
	sbSusp->setPalette ( pal );
	sbTerm->setPalette ( pal );
	sbExp->setPalette ( pal );


	if ( !miniMode )
		sbExp->move ( ( sessionStatusDlg->width()- ( sbExp->width() +12+sbTerm->width() +sbSusp->width() +12 ) ) /2,
		              sessionStatusDlg->height()-sbExp->height()-12 );
	else
		sbExp->move ( ( sessionStatusDlg->width()- ( sbExp->width() +12+sbTerm->width() +sbSusp->width() +12 ) ) /2,
		              sessionStatusDlg->height()-sbExp->height()-8 );

	sbSusp->move ( sbExp->pos().x() +sbExp->width() +12,sbExp->pos().y() );
	sbTerm->move ( sbSusp->pos().x() +sbSusp->width() +12,sbSusp->pos().y() );

	if ( !miniMode )
		sbAdv->move ( 30,sbSusp->y()-8-sbSusp->height() );
	else
		sbAdv->move ( 15,sbSusp->y()-sbAdv->height() -4 );

	stInfo=new QTextEdit ( sessionStatusDlg );
	setWidgetStyle ( stInfo );
	setWidgetStyle ( stInfo->verticalScrollBar() );
	stInfo->setReadOnly ( true );
	stInfo->hide();
	stInfo->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
	stInfo->setPalette ( pal );

	sbExp->setEnabled ( false );


	connect ( sbSusp,SIGNAL ( clicked() ),this, SLOT ( slot_testSessionStatus() ) );
	connect ( sbTerm,SIGNAL ( clicked() ),this, SLOT ( slotTermSessFromSt() ) );
	connect ( sbAdv,SIGNAL ( clicked() ),this, SLOT ( slotShowAdvancedStat() ) );

	QTimer::singleShot ( 1,this,SLOT ( slot_showStatusWidgets() ) );
	sessionStatusDlg->show();
}


void ONMainWindow::slotShowAdvancedStat()
{
	if ( !miniMode )
	{
		if ( sbAdv->isChecked() )
		{
			sessionStatusDlg->setFixedSize ( sessionStatusDlg->width(),sessionStatusDlg->height() *2 );
		}
		else
		{
			sessionStatusDlg->setFixedSize ( sessionStatusDlg->sizeHint() );
			stInfo->hide();
		}
	}
	else
	{
		if ( sbAdv->isChecked() )
		{
			sessionStatusDlg->setFixedSize ( 310,300 );
		}
		else
		{
			stInfo->hide();
			sessionStatusDlg->setFixedSize ( 310,200 );
		}
	}


	if ( !miniMode )
		sbExp->move ( ( sessionStatusDlg->width()- ( sbExp->width() +12+sbTerm->width() +sbSusp->width() +12 ) ) /2,
		              sessionStatusDlg->height()-sbExp->height()-12 );
	else
		sbExp->move ( ( sessionStatusDlg->width()- ( sbExp->width() +12+sbTerm->width() +sbSusp->width() +12 ) ) /2,
		              sessionStatusDlg->height()-sbExp->height()-8 );

	sbSusp->move ( sbExp->pos().x() +sbExp->width() +12,sbExp->pos().y() );
	sbTerm->move ( sbSusp->pos().x() +sbSusp->width() +12,sbSusp->pos().y() );

	username->invalidate();

	if ( !miniMode )
		sbAdv->move ( 30,sbSusp->y()-8-sbSusp->height() );
	else
		sbAdv->move ( 15,sbSusp->y()-sbAdv->height() -3 );



	if ( sbAdv->isChecked() )
	{
		if ( !miniMode )
		{
			stInfo->setFixedSize ( sessionStatusDlg->width()-40,
			                       sbAdv->y()-slVal->y()-slVal->height()-24 );
			stInfo->move ( 20,slVal->y() +slVal->height() +12 );
		}
		else
		{
			stInfo->setFixedSize ( sessionStatusDlg->width()-30,
			                       sbAdv->y()-slVal->y()-slVal->height()-12 );
			stInfo->move ( 15,slVal->y() +slVal->height() +6 );
		}

		stInfo->show();
	}

#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif


	st.setValue ( "showStatus", ( QVariant ) sbAdv->isChecked() );
	st.sync();
}




void ONMainWindow::slot_resumeDoubleClick ( const QModelIndex& )
{
	slotResumeSess();
}


void ONMainWindow::suspendSession ( QString user,QString host,QString pass,QString key,QString sessId )
{
	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gosuspend-session "+sessId,
		                      pass,key,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retSuspSess ( false,message,0 );
	}
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_retSuspSess ( bool,  QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retSuspSess ( false,message,0 );
		return;
	}
}


void ONMainWindow::termSession ( QString user,QString host,QString pass,QString key, QString sessId )
{
	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2goterminate-session "+sessId,
		                      pass,key,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retTermSess ( false,message,0 );
	}
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_retTermSess ( bool,  QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retTermSess ( false,message,0 );
		return;
	}
}


void ONMainWindow::slot_showStatusWidgets()
{
	slName->show();
	slVal->show();
	sbAdv->show();
	sbSusp->show();
	sbTerm->show();
	sbExp->show();
#ifndef WINDOWS

	QSettings st ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif

	if ( st.value ( "showStatus", ( QVariant ) false ).toBool() )
	{
		sbAdv->setChecked ( true );
		slotShowAdvancedStat();
	}
}

void ONMainWindow::setStatStatus ( const QString& status )
{
	QStringList lst=slVal->text().split ( '\n' );
	lst.removeLast();
	lst.append ( status );
	slVal->setText ( lst.join ( "\n" ) );
	slVal->setFixedSize ( slVal->sizeHint() );
}


void ONMainWindow::slot_restartNxProxy()
{
	if ( !sessionStatusDlg )
		return;
	if ( stInfo->toPlainText().indexOf ( "Established X server connection" ) ==-1 )
	{
		stInfo->insertPlainText ( tr ( "Connection timeout, aborting" ) );
		nxproxy->terminate();
		restartResume=true;
	}
}


void ONMainWindow::slot_testSessionStatus()
{
	if ( !sessionStatusDlg )
		return;
	if ( stInfo->toPlainText().indexOf ( "Established X server connection" ) ==-1 )
	{
		stInfo->insertPlainText ( tr ( "Connection timeout, aborting" ) );
		nxproxy->terminate();
	}
}


x2goSession ONMainWindow::getNewSessionFromString ( const QString& string )
{
	QStringList lst=string.split ( '|' );
	x2goSession s;
	s.display=lst[0];
	s.cookie=lst[1];
	s.agentPid=lst[2];
	s.sessionId=lst[3];
	s.grPort=lst[4];
	s.sndPort=lst[5];
	return s;
}


void ONMainWindow::runCommand()
{
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QString passwd=pass->text();
	QString user=login->text();
	QString host=resumingSession.server;
	QString command;
	QString sessionType="D";
	if ( useLdap )
		command=sessionCmd;
	else
	{
		QString sid=lastSession->id();
		command=st.value ( sid+"/command", ( QVariant ) tr ( "KDE" ) ).toString();
		bool rootless=st.value ( sid+"/rootless", ( QVariant ) false ).toBool();
		if ( rootless )
			sessionType="R";
	}

	if ( command=="KDE" )
	{
		command="startkde";
	}
	else if ( command=="GNOME" )
	{
		command="gnome-session";
	}


	sshProcess *proc=0l;

	QString cmd;
	bool sound;

	if ( useLdap )
	{
		sound=startSound;
	}
	else
	{
#ifndef WINDOWS
		QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid=lastSession->id();
		sound=st.value ( sid+"/sound",
		                 ( QVariant ) defaultUseSound ).toBool();
	}

	command.replace ( " ","X2GO_SPACE_CHAR" );

	if ( !sound )
		cmd="setsid x2goruncommand "+resumingSession.display+" "+
		    resumingSession.agentPid + " " +resumingSession.sessionId+" "+
		    resumingSession.sndPort+ " "+ command+" nosnd "+
		    sessionType +">& /dev/null & exit";
	else if ( useEsd )
		cmd="setsid x2goruncommand "+resumingSession.display+" "+
		    resumingSession.agentPid + " " +resumingSession.sessionId+" "+
		    resumingSession.sndPort+ " "+ command+" esd "+
		    sessionType +">& /dev/null & exit";
	else
		cmd="setsid x2goruncommand "+resumingSession.display+" "+
		    resumingSession.agentPid + " " +resumingSession.sessionId+" "+
		    resumingSession.sndPort+ " "+ command+" arts "+
		    sessionType +">& /dev/null & exit";


	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retRunCommand ( false,message,0 );
	}
	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retRunCommand ( bool, QString,sshProcess* ) ) );

	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retRunCommand ( false,message,0 );
		return;
	}
#ifdef Q_WS_HILDON
	//wait 5 seconds and execute xkbcomp
	QTimer::singleShot ( 5000, this, SLOT ( slot_execXmodmap() ) );
#endif
}

void ONMainWindow::slot_retRunCommand ( bool result, QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n:\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
}


bool ONMainWindow::parseParam ( QString param )
{
	if ( param=="--help" )
	{
		showHelp();
		return false;
	}

	if ( param=="--help-pack" )
	{
		showHelpPack();
		return false;
	}

	if ( param=="--no-menu" )
	{
		drawMenu=false;
		return true;
	}

	if ( param=="--maximize" )
	{
		startMaximized=true;
		return true;
	}
	if ( param=="--esd" )
	{
		useEsd=true;
		return true;
	}
	if ( param=="--pgp-card" )
	{
		usePGPCard=true;
		return true;
	}
	if ( param=="--add-to-known-hosts" )
	{
		acceptRsa=true;
		return true;
	}

	QString setting,value;
	QStringList vals=param.split ( "=" );
	if ( vals.size() <2 )
	{
		printError ( param );
		return false;
	}
	setting=vals[0];
	vals.removeFirst();
	value=vals.join ( "=" );
	if ( setting=="--link" )
	{
		return link_par ( value );
	}
	if ( setting=="--sound" )
	{
		return sound_par ( value );
	}
	if ( setting=="--geometry" )
	{
		return geometry_par ( value );
	}
	if ( setting=="--pack" )
	{
		return pack_par ( value );
	}
	if ( setting=="--kbd-layout" )
	{
		defaultKbdType=value;
		return true;
	}
	if ( setting=="--kbd-type" )
	{
		defaultKbdType=value;
		return true;
	}
	if ( setting=="--set-kbd" )
	{
		return setKbd_par ( value );
	}
	if ( setting=="--ldap" )
	{
		return ldap_par ( value );
	}
	if ( setting=="--ldap1" )
	{
		return ldap1_par ( value );
	}
	if ( setting=="--ldap2" )
	{
		return ldap2_par ( value );
	}
	if ( setting=="--command" )
	{
		defaultCmd=value;
		return true;
	}
	if ( setting=="--read-exports-from" )
	{
		readExportsFrom=value;
		return true;
	}
	if ( setting=="--external-login" )
	{
		extLogin=true;
		readLoginsFrom=value;
		return true;
	}
	if ( setting=="--arts-cmd" )
	{
		artsCmd=value;
		return true;
	}
	if ( setting=="--ssh-port" )
	{
		defaultSshPort=value;
		return true;
	}
	if ( setting=="--client-ssh-port" )
	{
		clientSshPort=value;
		return true;
	}
	if ( setting=="--esd-cmd" )
	{
		esdCmd=value;
		return true;
	}
	printError ( param );
	return false;
}


bool ONMainWindow::link_par ( QString value )
{
	if ( value=="modem" )
		defaultLink=0;
	else if ( value=="isdn" )
		defaultLink=1;
	else if ( value=="adsl" )
		defaultLink=2;
	else if ( value=="wan" )
		defaultLink=3;
	else if ( value=="lan" )
		defaultLink=4;
	else
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--link\"" ).toLocal8Bit().data() );
		return false;
	}
	return true;

}

bool ONMainWindow::sound_par ( QString val )
{
	if ( val=="1" )
		defaultUseSound=true;
	else if ( val=="0" )
		defaultUseSound=false;
	else
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--sound\"" ).toLocal8Bit().data() );
		return false;
	}
	return true;
}

bool ONMainWindow::geometry_par ( QString val )
{
	if ( val=="fullscreen" )
		defaultFullscreen=true;
	else
	{
		QStringList res=val.split ( "x" );
		if ( res.size() !=2 )
		{
			qCritical ( "%s",tr ( "wrong value for argument\"--geometry\"" ).toLocal8Bit().data() );
			return false;
		}
		bool o1,o2;
		defaultWidth=res[0].toInt ( &o1 );
		defaultHeight=res[1].toInt ( &o2 );
		if ( ! ( defaultWidth >0 && defaultHeight >0 && o1 && o2 ) )
		{
			qCritical ( "%s",tr ( "wrong value for argument\"--geometry\"" ).toLocal8Bit().data() );
			return false;
		}
	}
	return true;
}

bool ONMainWindow::setKbd_par ( QString val )
{
	if ( val=="1" )
		defaultSetKbd=true;
	else if ( val=="0" )
		defaultSetKbd=false;
	else
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--set-kbd\"" ).toLocal8Bit().data() );
		return false;
	}
	return true;
}

bool ONMainWindow::ldap_par ( QString val )
{
	QString ldapstring=val;
	useLdap=true;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=3 )
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--ldap\"" ).toLocal8Bit().data() );
		return false;
	}
	ldapOnly=true;
	ldapServer=lst[0];
	ldapPort=lst[1].toInt();
	ldapDn=lst[2];


	return true;
}

bool ONMainWindow::ldap1_par ( QString val )
{
	QString ldapstring=val;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=2 )
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--ldap1\"" ).toLocal8Bit().data() );
		return false;
	}
	ldapServer1=lst[0];
	ldapPort1=lst[1].toInt();

	return true;
}

bool ONMainWindow::ldap2_par ( QString val )
{
	QString ldapstring=val;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=2 )
	{
		qCritical ( "%s",tr ( "wrong value for argument\"--ldap2\"" ).toLocal8Bit().data() );
		return false;
	}
	ldapServer2=lst[0];
	ldapPort2=lst[1].toInt();

	return true;
}


bool ONMainWindow::pack_par ( QString val )
{

	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return true;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );

			QStringList pctails=val.split ( "-" );
			QString pcq=pctails[pctails.size()-1];
			pctails.removeLast();

			if ( pctails.join ( "-" ) ==pc )
			{
				bool ok;
				int v=pcq.toInt ( &ok );
				if ( ok && v>=0 && v<=9 )
				{
					defaultPack=pc;
					defaultQuality=v;
					return true;
				}
				else
					break;
			}
		}
		else
		{
			if ( pc==val )
			{
				defaultPack=val;
				return true;
			}
		}
	}
	file.close();
	qCritical ( "%s",tr ( "wrong value for argument\"--pack\"" ).toLocal8Bit().data() );
	return false;
}


void ONMainWindow::printError ( QString param )
{
	qCritical ( "%s", ( tr ( "wrong parameter: " ) +param ).toLocal8Bit().data() );
}

void ONMainWindow::showHelp()
{
	QString helpMsg=
	    tr (
	        "Usage: x2goclient [Options]\n\
	        Options:\n\
	        --help                           Print this message\n\
	        --help-pack                      Print availabel pack methods\n\
	        --no-menu                        Hide menu bar\n\
	        --maximize                       Start maximized\n\
	        --pgp-card 			     Use openPGP Card authentification\n\
	        --add-to-known-hosts             Add RSA key fingerprint to .ssh/known_hosts \n\
	        if authenticity of server can't be established\n\
	        --ldap=<host:port:dn>            Start with LDAP Support. Example:\n\
	        --ldap=ldapserver:389:o=organization,c=de\n\
	        --ldap1=<host:port>              LDAP Failover Server #1 \n\
	        --ldap2=<host:port>              LDAP Failover Server #2 \n\
	        --ssh-port=<port>                connect to this port, default value 22\n\
	        --client-ssh-port=<port>         local ssh port (for fs export), default value 22\n\
	        --command=<cmd>                  Set default command, default value 'KDE'\n\
	        --sound=<0|1>                    Enable sound, default value '1'\n\
	        --esd                            Use ESD instead ARTS\n\
	        --geomerty=<W>x<H>|fullscreen    Set default geometry, default value '800x600'\n\
	        --link=<modem|isdn|adsl|wan|lan> Set default link type, default 'lan'\n\
	        --pack=<packmethod>              Set default pack method, default '16m-jpeg-9'\n\
	        --kbd-layout=<layout>            Set default keyboard layout, default 'us'\n\
	        --kbd-type=<typed>               Set default keyboard type, default 'pc105/us'\n\
	        --set-kbd=<0|1>                  Overwrite current keyboard settings, default '0'\n" );
	qCritical ( "%s",helpMsg.toLocal8Bit().data() );
	QMessageBox::information ( this,tr ( "Options" ),helpMsg );
}

void ONMainWindow::showHelpPack()
{
	qCritical ( "%s",tr ( "Available pack methodes:" ).toLocal8Bit().data() );
	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	QString msg;
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );
			pc+="-[0-9]";
		}
		msg+=pc+"\n";
		qCritical ( "%s",pc.toLocal8Bit().data() );
	}
	file.close();
#ifdef WINDOWS

	QMessageBox::information ( this,tr ( "Options" ),msg );
#endif

}

void ONMainWindow::slot_getServers ( bool result, QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	proc=0;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;

		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		pass->setFocus();
		pass->selectAll();
		return;
	}

	passForm->hide();
	setUsersEnabled ( false );
	uname->setEnabled ( false );
	u->setEnabled ( false );
	QStringList servers=output.trimmed().split ( '\n' );
	for ( int i=0;i<servers.size();++i )
	{
		QStringList lst=servers[i].simplified().split ( ' ' );
		if ( lst.size() >1 )
		{
			for ( int j=0;j<x2goServers.size();++j )
				if ( x2goServers[j].name==lst[0] )
				{
					x2goServers[j].sess=lst[1].toInt() *x2goServers[j].factor;
					x2goDebug<<x2goServers[j].name<<": sessions "<<
					lst[1].toInt() <<
					", multiplied "<<x2goServers[j].sess;
					break;
				}
		}
	}

	qSort ( x2goServers.begin(),x2goServers.end(),serv::lt );

	listedSessions.clear();
	retSessions=0;

	for ( int j=0;j<x2goServers.size();++j )
	{
		QString passwd;
		QString user=login->text();
		QString host=x2goServers[j].name;
		passwd=pass->text();

		sshProcess* lproc;
		try
		{
			lproc=new sshProcess ( this,user,host,sshPort,
			                       "export HOSTNAME && x2golistsessions",
			                       passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			slot_listAllSessions ( false,message,0 );
			continue;
		}
		if ( cardReady )
		{
			QStringList env=lproc->environment();
			env+=sshEnv;
			lproc->setEnvironment ( env );
		}

		connect ( lproc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
		          this,SLOT ( slot_listAllSessions ( bool, QString,sshProcess* ) ) );
		try
		{
			lproc->startNormal();
		}
		catch ( QString message )
		{
			slot_listAllSessions ( false,message,0 );
			continue;
		}
	}
}


void ONMainWindow::slot_listAllSessions ( bool result,QString output,sshProcess* proc )
{
	bool last=false;
	++retSessions;
	if ( retSessions == x2goServers.size() )
		last=true;
	if ( proc )
		delete proc;
	proc=0;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QString sv=output.split ( ":" ) [0];
		for ( int j=0;j<x2goServers.size();++j )
		{
			if ( x2goServers[j].name==sv )
			{
				x2goServers[j].connOk=false;
			}
		}
	}
	else
		listedSessions+=output.trimmed().split ( '\n',QString::SkipEmptyParts );
	if ( last )
	{
		if ( listedSessions.size() ==0||
		        ( listedSessions.size() ==1&&listedSessions[0].length() <5 ) )
			startNewSession();
		else if ( listedSessions.size() ==1 )
		{
			x2goSession s=getSessionFromString ( listedSessions[0] );
			QDesktopWidget wd;
			if ( s.status=="S" && isColorDepthOk ( wd.depth(),s.colorDepth ) )
				resumeSession ( s );
			else
				selectSession ( listedSessions );
		}
		else
			selectSession ( listedSessions );
	}
}

void ONMainWindow::slot_resize()
{
	if ( !startMaximized && !mwMax )
	{
		resize ( mwSize );
		move ( mwPos );
		show();
	}
	else
		showMaximized();
}

void ONMainWindow::slot_exportDirectory()
{
	QString path;
	if ( !useLdap )
	{
		ExportDialog dlg ( lastSession->id(),this );
		if ( dlg.exec() ==QDialog::Accepted )
			path=dlg.getExport();
	}
	else
		path= QFileDialog::getExistingDirectory (
		          this,QString::null,
		          QDir::homePath() );

#ifdef WINDOWS

	path=transform2cygwinPath ( path );
#endif

	if ( path!=QString::null )
		exportDirs ( path );
}


void ONMainWindow::exportDirs ( QString exports,bool removable )
{
	directory dr;
	dr.dirList=exports;
	dr.key=createRSAKey();
	QString passwd=pass->text();

	sshProcess* lproc;
	try
	{
		lproc=new sshProcess ( this,login->text(),resumingSession.server,sshPort,"",passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_copyKey ( false,message,0 );
		return;
	}
	if ( cardReady )
	{
		QStringList env=lproc->environment();
		env+=sshEnv;
		lproc->setEnvironment ( env );
	}

	connect ( lproc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_copyKey ( bool, QString,sshProcess* ) ) );
	try
	{
		QString dst=dr.key;
		dst.replace ( QDir::homePath() +"/.x2go/ssh/gen/","" );
		dst="/home/"+login->text() +"/.x2go/ssh/"+dst;
		dr.dstKey=dst;
		dr.isRemovable=removable;
		exportDir.append ( dr );
#ifndef WINDOWS

		lproc->start_cp ( dr.key,dst );
#else

		lproc->start_cp ( transform2cygwinPath ( dr.key ),dst );
#endif

	}
	catch ( QString message )
	{
		slot_copyKey ( false,message,0 );
	}
}


void ONMainWindow::exportDefaultDirs()
{
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QString exd=st.value ( lastSession->id() +"/export",
	                       ( QVariant ) QString::null ).toString();
	QStringList lst=exd.split ( ";",QString::SkipEmptyParts );
	QStringList dirs;
	for ( int i=0;i<lst.size();++i )
	{
#ifndef WINDOWS
		QStringList tails=lst[i].split ( ":",QString::SkipEmptyParts );
#else

		QStringList tails=lst[i].split ( "#",QString::SkipEmptyParts );
#endif

		if ( tails[1]=="1" )
#ifndef WINDOWS

			dirs+=tails[0];
#else

			dirs+=transform2cygwinPath ( tails[0] );
#endif

	}
	if ( dirs.size() <=0 )
		return;
	exportDirs ( dirs.join ( ":" ) );
}

QString ONMainWindow::createRSAKey()
{
	QDir dr;
	dr.mkpath ( QDir::homePath() +"/.x2go/ssh/gen" );
	QTemporaryFile fl ( QDir::homePath() +"/.x2go/ssh/gen/key" );
	fl.open();
	QString keyName=fl.fileName();
	fl.setAutoRemove ( false );
	fl.close();
	fl.remove();

	QStringList args;
#ifndef WINDOWS

	args<<"-t"<<"rsa"<<"-b"<<"1024"<<"-N"<<""<<"-f"<<keyName;
#else

	args<<"-t"<<"rsa"<<"-b"<<"1024"<<"-N"<<""<<"-f"<<transform2cygwinPath ( keyName );
#endif

	x2goDebug <<keyName<<endl;

	if ( QProcess::execute ( "ssh-keygen",args ) !=0 )
		return QString::null;

#ifdef WINDOWS
	QFile rsa ( ConfigDialog::getCygwinDir ( "/" ) +"\\etc\\ssh_host_rsa_key.pub" );
#endif
#ifdef Q_OS_DARWIN
	QFile rsa ( "/etc/ssh_host_rsa_key.pub" );
#else
	QFile rsa ( "/etc/ssh/ssh_host_rsa_key.pub" );
#endif

	if ( !rsa.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
#ifdef WINDOWS
		QMessageBox::critical ( 0l,tr ( "Error" ), tr ( "Can't read host rsa key:" ) +ConfigDialog::getCygwinDir ( "/" ) +"\\etc\\ssh_host_rsa_key.pub", QMessageBox::Ok,QMessageBox::NoButton );
#else

		QMessageBox::critical ( 0l,tr ( "Error" ), tr ( "Can't read host rsa key:" ) +"/etc/ssh/ssh_host_rsa_key.pub", QMessageBox::Ok,QMessageBox::NoButton );
#endif

		return QString::null;
	}

	QByteArray rsa_pub;

	if ( !rsa.atEnd() )
		rsa_pub = rsa.readLine();
	else
		return QString::null;

	QFile file ( keyName );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
		return keyName;
	QTextStream out ( &file );
	out<<"----BEGIN RSA IDENTITY----"<<rsa_pub;
	file.close();


	return keyName;
}

void ONMainWindow::slot_copyKey ( bool result,QString output,sshProcess* proc )
{
#ifdef WINDOWS
	QString key=transform2winPath ( proc->getSource() );
#else

	QString key=proc->getSource();
#endif

	if ( proc )
		delete proc;
	proc=0;
	QFile::remove
	( key );
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( key+".pub" );
		return;
	}

	QFile file ( key+".pub" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to read :\n" ) +key+".pub";
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( key+".pub" );
		return;
	}

	QByteArray line = file.readLine();
	file.close();

#ifndef WINDOWS

	QString authofname=QDir::homePath() +"/.ssh/authorized_keys" ;
#else

	QString authofname=cygwinHomePath() +"\\.ssh\\authorized_keys" ;
#endif

	QFile file1 ( authofname );

	if ( !file1.open ( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
	{
		QString message=tr ( "Unable to write :\n" ) + authofname;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( key+".pub" );
		return;

	}
	QTextStream out ( &file1 );
	out<<line;
	file1.close();
	directory* dir=getExpDir ( key );
	bool rem=dir->isRemovable;
	if ( !dir )
		return;

	QString passwd=pass->text();
	QString user=login->text();
	QString host=resumingSession.server;
	QString sessionId=resumingSession.sessionId;

	QStringList env=QProcess::systemEnvironment();
	QString cuser;
	for ( int i=0;i<env.size();++i )
	{
		QStringList ls=env[i].split ( "=" );
#ifndef WINDOWS

		if ( ls[0]=="USER" )
#else

		if ( ls[0]=="USERNAME" )
#endif

		{
			cuser=ls[1];
			break;
		}
	}
	proc=0l;
	QString cmd;
	QString dirs=dir->dirList;

	if ( clientSshPort!="22" )
	{
		dirs=dirs+"__SSH_PORT__"+clientSshPort;
	}
	if ( !rem )
		cmd="export HOSTNAME && x2gomountdirs dir "+sessionId+" "+cuser+
		    " "+dir->dstKey+" "+dirs;
	else
		cmd="export HOSTNAME && x2gomountdirs rem "+sessionId+" "+cuser+
		    " "+dir->dstKey+" "+dirs;

#ifdef WINDOWS

	cmd="chmod 600 "+dir->dstKey+"&&"+cmd;
#endif

	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
		dir->proc=proc;
	}
	catch ( QString message )
	{
		slot_retExportDir ( false,message,proc );
	}
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retExportDir ( bool, QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retExportDir ( false,message,proc );
		return;
	}


}

directory* ONMainWindow::getExpDir ( QString key )
{
	for ( int i=0;i<exportDir.size();++i )
	{
#ifndef   WINDOWS
		if ( exportDir[i].key==key )
#else

		if ( transform2winPath ( transform2cygwinPath ( exportDir[i].key ) ) ==key )
#endif

			return &exportDir[i];
	}
	return 0l;
}




void ONMainWindow::slot_retExportDir ( bool result,QString output,sshProcess* proc )
{

	QString key;
	for ( int i=0;i<exportDir.size();++i )
		if ( exportDir[i].proc==proc )
		{
			key=exportDir[i].key;
			exportDir.removeAt ( i );
			break;
		}

	if ( proc )
		delete proc;

	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	QFile file ( key+".pub" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to read :\n" ) +key+".pub";
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( key+".pub" );
		return;
	}

	QByteArray line = file.readLine();
	file.close();
#ifndef WINDOWS

	QString authofname=QDir::homePath() +"/.ssh/authorized_keys" ;
#else

	QString authofname=cygwinHomePath() +"\\.ssh\\authorized_keys" ;
#endif

	file.setFileName ( authofname );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to write :\n" ) +authofname;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( key+".pub" );
		return;
	}


	QTemporaryFile fl ( authofname );
	fl.open();
	QString tmpName=fl.fileName();
	fl.setAutoRemove ( false );
	QTextStream out ( &fl );

	while ( !file.atEnd() )
	{
		QByteArray newline = file.readLine();
		if ( newline!=line )
			out<<newline;
	}
	file.close();
	fl.close();

	QFile::remove
	( authofname );
	QFile::rename ( tmpName,authofname );

	QFile::remove
	( key+".pub" );
}



void ONMainWindow::exportsEdit ( SessionButton* bt )
{
	EditConnectionDialog dlg ( bt->id(),this,3 );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		bt->redraw();
		bool vis=bt->isVisible();
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
		bt->setVisible ( vis );
	}
}


void ONMainWindow::slotExtTimer()
{

	if ( QFile::permissions ( readLoginsFrom ) != ( QFile::ReadUser|QFile::WriteUser|QFile::ExeUser|
	        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
	{
		x2goDebug <<"Wrong permissions on "<<readLoginsFrom <<":"<<endl;
		x2goDebug << ( int ) ( QFile::permissions ( readLoginsFrom+"/." ) )
		<<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
		                        |QFile::ExeUser|QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) <<endl;
		if ( extLogin )
			extTimer->stop();
		return;
	}
	QString loginDir;
	QString logoutDir;
	QDir dir ( readLoginsFrom );
	QStringList list = dir.entryList ( QDir::Files );
	for ( int i=0;i<list.size();++i )
	{
		QFile file ( readLoginsFrom+"/"+list[i] );
		if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
			continue;
		if ( !file.atEnd() )
		{
			QByteArray line = file.readLine();
			QString ln ( line );
			QStringList args=ln.split ( "=",QString::SkipEmptyParts );
			if ( args.size() >1 )
			{
				if ( args[0]=="login" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						loginDir=args[1];
				}
				if ( args[0]=="logout" )
				{
					x2goDebug <<" I HAVE external logout"<<endl;
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						logoutDir=args[1];
				}
			}
		}
		file.close();
		file.remove();
	}
	if ( exportTimer->isActive() ) //running session
	{
		if ( logoutDir != QString::null )
		{
			x2goDebug <<"external logout"<<endl;
			externalLogout ( logoutDir );
		}
	}
	else
	{
		if ( loginDir != QString::null )
		{
			x2goDebug <<"external login"<<endl;
			externalLogin ( loginDir );
		}
	}
}


void ONMainWindow::slot_exportTimer()
{

	if ( QFile::permissions ( readExportsFrom ) != ( QFile::ReadUser|QFile::WriteUser|QFile::ExeUser|
	        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
	{
		x2goDebug <<"Wrong permissions on "<<readExportsFrom <<":"<<endl;
		x2goDebug << ( int ) ( QFile::permissions ( readExportsFrom+"/." ) )
		<<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
		                        |QFile::ExeUser|QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) <<endl;
		exportTimer->stop();
		return;
	}

	QDir dir ( readExportsFrom );
	QStringList list = dir.entryList ( QDir::Files );
	QString expList;
	QString unexpList;
	QString loginDir;
	QString logoutDir;
	for ( int i=0;i<list.size();++i )
	{
		QFile file ( readExportsFrom+"/"+list[i] );
		if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
			continue;
		if ( !file.atEnd() )
		{
			QByteArray line = file.readLine();
			QString ln ( line );
			QStringList args=ln.split ( "=",QString::SkipEmptyParts );
			if ( args.size() >1 )
			{
				if ( args[0]=="export" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						expList+=":"+args[1];
				}
				if ( args[0]=="unexport" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						unexpList+=":"+args[1];
				}
			}
		}
		file.close();
		file.remove();
	}
	QStringList args=expList.split ( ":",QString::SkipEmptyParts );
	expList=args.join ( ":" );
	if ( expList.size() >0 )
	{
		exportDirs ( expList,true );
	}
	args.clear();
	args=unexpList.split ( ":",QString::SkipEmptyParts );

	QString passwd=pass->text();
	QString user=login->text();
	QString host=resumingSession.server;
	QString sessionId=resumingSession.sessionId;

	for ( int i=0;i<args.size();++i )
	{
		sshProcess* sproc=new sshProcess ( this,user,host,sshPort,
		                                   "export HOSTNAME && x2goumount_session "+sessionId+" "+args[i],
		                                   passwd,currentKey,acceptRsa );
		if ( cardReady )
		{
			QStringList env=sproc->environment();
			env+=sshEnv;
			sproc->setEnvironment ( env );
		}

		sproc->startNormal();
	}
}

void ONMainWindow::slot_about_qt()
{
	QMessageBox::aboutQt ( this );
}

void ONMainWindow::slot_about()
{
	QMessageBox::about ( this,tr ( "About X2GO Client" ),tr ( "<b>X2Go Client V. " ) +VERSION+tr ( "</b><br> (C. 2006-2008 <b>obviously nice</b>: Oleksandr Shneyder, Heinz-Markus Graesing)<br><br>Client for use with the X2Go network based computing environment. This Client will be able to connect to X2Go server(s) and start, stop, resume and terminate (running) desktop sessions. X2Go Client stores different server connections and may automatically request authentification data from LDAP directories. Furthermore it can be used as fullscreen loginscreen (replacement for loginmanager like xdm). Please visit x2go.org for further information." ) );
}



void ONMainWindow::slot_rereadUsers()
{
	if ( !useLdap )
		return;
#ifdef USELDAP

	if ( ld )
	{
		delete ld;
		ld=0;
	}

	if ( ! initLdapSession ( false ) )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );

		slot_config();
		return;
	}


	list<string> attr;
	attr.push_back ( "uidNumber" );
	attr.push_back ( "uid" );


	list<LDAPBinEntry> result;
	try
	{
		ld->binSearch ( ldapDn.toStdString(),attr,"objectClass=posixAccount",result );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,QMessageBox::NoButton );
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}

	list<LDAPBinEntry>::iterator it=result.begin();
	list<LDAPBinEntry>::iterator end=result.end();

	for ( ;it!=end;++it )
	{
		user u;
		QString uin=LDAPSession::getBinAttrValues ( *it,"uidNumber" ).front().getData();
		u.uin=uin.toUInt();
		if ( u.uin<firstUid || u.uin>lastUid )
		{
			continue;
		}
		u.uid=LDAPSession::getBinAttrValues ( *it,"uid" ).front().getData();
		if ( !findInList ( u.uid ) )
		{
			reloadUsers();
			return;
		}
	}
#endif
}

void ONMainWindow::reloadUsers()
{
	int i;
	for ( i=0;i<names.size();++i )
		names[i]->close();
	for ( i=0;i<sessions.size();++i )
		sessions[i]->close();

	userList.clear();
	sessions.clear();


	loadSettings();
	if ( useLdap )
	{
		act_new->setEnabled ( false );
		act_edit->setEnabled ( false );
		u->setText ( tr ( "Login:" ) );
		QTimer::singleShot ( 1, this, SLOT ( readUsers() ) );
	}
	else
	{
		act_new->setEnabled ( true );
		act_edit->setEnabled ( true );
		u->setText ( tr ( "Session:" ) );
		QTimer::singleShot ( 1, this, SLOT ( slot_readSessions() ) );
	}
	slot_resize ( fr->size() );
}


bool ONMainWindow::findInList ( const QString& uid )
{
	for ( int i=0;i<userList.size();++i )
	{
		if ( userList[i].uid==uid )
			return true;
	}
	return false;
}

void ONMainWindow::setUsersEnabled ( bool enable )
{

	if ( useLdap )
	{
		QScrollBar* bar=users->verticalScrollBar();
		bar->setEnabled ( enable );
		int upos=bar->value();
		QDesktopWidget dw;
		int height=dw.screenGeometry ( fr ).height();
		QList<UserButton*>::iterator it;
		QList<UserButton*>::iterator endit=names.end();
		if ( !enable )
		{
			for ( it=names.begin();it!=endit;it++ )
			{
				QPoint pos= ( *it )->pos();
				if ( ( pos.y() >upos-height ) && ( pos.y() <upos+height ) )
					( *it )->setEnabled ( false );
				if ( pos.y() >upos+height )
					break;
			}
		}
		else
		{
			for ( it=names.begin();it!=endit;it++ )
			{
				if ( ! ( *it )->isEnabled() )
					( *it )->setEnabled ( true );
			}
		}
	}
	else
		users->setEnabled ( enable );
}


void ONMainWindow::externalLogin ( const QString& loginDir )
{
	QFile file ( loginDir+"/username" );
	QString user;

	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		user=in.readLine();
		break;
	}
	file.close();


	if ( passForm )
		slotClosePass();
	uname->setText ( user );
	slotUnameEntered();
	currentKey=loginDir+"/dsa.key";
	extStarted=true;
	slotPassEnter();
}


void ONMainWindow::externalLogout ( const QString& )
{
	if ( extStarted )
	{
		extStarted=false;
		currentKey=QString::null;
		if ( nxproxy )
			if ( nxproxy->state() ==QProcess::Running )
				nxproxy->terminate();
	}
}


void ONMainWindow::slot_startPGPAuth()
{
	scDaemon=new QProcess ( this );
	QStringList arguments;
	arguments<<"--server";
	connect ( scDaemon,SIGNAL ( readyReadStandardError() ),this,SLOT ( slot_scDaemonError() ) );
	connect ( scDaemon,SIGNAL ( readyReadStandardOutput() ),this,SLOT ( slot_scDaemonOut() ) );
	connect ( scDaemon,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_scDaemonFinished ( int, QProcess::ExitStatus ) ) );
	scDaemon->start ( "scdaemon",arguments );
	QTimer::singleShot ( 3000, this, SLOT ( slot_checkScDaemon() ) );
	isScDaemonOk=false;
}

void ONMainWindow::slot_checkScDaemon()
{
	if ( !isScDaemonOk )
	{
		scDaemon->kill();
	}
}

void ONMainWindow::slot_scDaemonError()
{
	QString stdOut ( scDaemon->readAllStandardError() );
	stdOut=stdOut.simplified();
	x2goDebug<<"SCDAEMON err:"<<stdOut<<endl;
	if ( stdOut.indexOf ( "updating status of" ) !=-1 )
	{
		isScDaemonOk=true;
		QStringList lst=stdOut.split ( " " );
		if ( lst[lst.count()-1]=="0x0002" || lst[lst.count()-1]=="0x0007" ) //USABLE or PRESENT
		{
			scDaemon->kill();
		}
	}
}

void ONMainWindow::slot_scDaemonOut()
{
	QString stdOut ( scDaemon->readAllStandardOutput() );
	stdOut=stdOut.simplified();
	x2goDebug<<"SCDAEMON out:"<<stdOut<<endl;
}

void ONMainWindow::slot_scDaemonFinished ( int , QProcess::ExitStatus )
{
	scDaemon=0l;
	if ( isScDaemonOk )
	{
		x2goDebug<<"scDaemon finished"<<endl;
		gpg=new QProcess ( this );
		QStringList arguments;
		arguments<<"--card-status";
		connect ( gpg,SIGNAL ( readyReadStandardError() ),this,SLOT ( slot_gpgError() ) );
		connect ( gpg,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
		          SLOT ( slot_gpgFinished ( int, QProcess::ExitStatus ) ) );
		gpg->start ( "gpg",arguments );
	}
	else
		slot_startPGPAuth();
}



void ONMainWindow::slot_gpgError()
{
	QString stdOut ( gpg->readAllStandardError() );
	stdOut=stdOut.simplified();
	x2goDebug<<"GPG err:"<<stdOut<<endl;
	if ( stdOut.indexOf ( "failed" ) !=-1 )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "No valid card found" ),
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		gpg->kill();
	}
}


void ONMainWindow::slot_gpgFinished ( int exitCode, QProcess::ExitStatus exitStatus )
{
	x2goDebug<<"gpg finished, exit code:"<<exitCode<<" exit status:"<<exitStatus<<endl;
	if ( exitStatus==0 )
	{
		QString stdOut ( gpg->readAllStandardOutput() );
		stdOut.chop ( 1 );
		x2goDebug<<"GPG out:"<<stdOut<<endl;
		QStringList lines=stdOut.split ( "\n" );
		QString login;
		QString appId;
		QString authKey;
		for ( int i=0;i<lines.count();++i )
		{
			if ( lines[i].indexOf ( "Application ID" ) !=-1 )
			{
				appId=lines[i].split ( ":" ) [1];
			}
			else if ( lines[i].indexOf ( "Login data" ) !=-1 )
			{
				login=lines[i].split ( ":" ) [1];
			}
			else if ( lines[i].indexOf ( "Authentication key" ) !=-1 )
			{
				authKey=lines[i].split ( ":" ) [1];
				break;
			}
		}
		appId=appId.simplified();
		login=login.simplified();
		authKey=authKey.simplified();
		x2goDebug<<"card data: "<<appId<<login<<authKey<<endl;
		if ( login=="[not set]" || authKey == "[none]" )
		{
			x2goDebug<<"Card not configured\n";
			QMessageBox::critical ( 0l,tr ( "Error" ),
			                        tr ( "This card is unknown by X2Go System" ),
			                        QMessageBox::Ok,
			                        QMessageBox::NoButton );
			QTimer::singleShot ( 1000, this, SLOT ( slot_startPGPAuth() ) );
		}
		else
			startGPGAgent ( login,appId );
	}
	else
		QTimer::singleShot ( 1000, this, SLOT ( slot_startPGPAuth() ) );
	gpg=0l;
}



void ONMainWindow::startGPGAgent ( const QString& login, const QString& appId )
{
	QString gpgPath=QDir::homePath() +"/.x2goclient/gnupg";
	QDir d;
	cardLogin=login;
	d.mkpath ( gpgPath );
	QFile file ( gpgPath+"/scd-event" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),tr ( "Unable to create file: " ) +gpgPath+"/scd-event"
		                        ,QMessageBox::Ok,QMessageBox::NoButton );
		exit ( -1 );
	}
	QTextStream out ( &file );
	out << "#!/bin/bash\n\nif [ \"$6\" != \"0x0002\" ] && [ \"$6\" != \"0x0007\" ]\n\
	then\n\
	kill -9 $_assuan_pipe_connect_pid\n\
	fi"<<endl;
	file.close();
	file.setPermissions ( gpgPath+"/scd-event",QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );

	gpgAgent=new QProcess ( this );
	QStringList arguments;
	arguments<<"--pinentry-program"<<"/usr/bin/pinentry-x2go"<<"--enable-ssh-support"<<"--daemon"
	<<"--no-detach";

	connect ( gpgAgent,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_gpgAgentFinished ( int, QProcess::ExitStatus ) ) );

	QStringList env=QProcess::systemEnvironment();
	env<<"GNUPGHOME="+gpgPath<<"CARDAPPID="+appId;
	gpgAgent->setEnvironment ( env );
	gpgAgent->start ( "gpg-agent",arguments );
}

void ONMainWindow::slot_gpgAgentFinished ( int , QProcess::ExitStatus )
{
	QString stdOut ( gpgAgent->readAllStandardOutput() );
	stdOut=stdOut.simplified();
	stdOut.replace ( " ","" );
	QStringList envLst=stdOut.split ( ";" );
	QString gpg_agent_info=envLst[0].split ( "=" ) [1];
	QString ssh_auth_sock=envLst[2].split ( "=" ) [1];
	agentPid=envLst[4].split ( "=" ) [1];
	x2goDebug<<gpg_agent_info<<ssh_auth_sock<<agentPid<<endl;
	x2goDebug<<"GPGAGENT out:"<<envLst[0]<<envLst[2]<<envLst[4]<<endl;

	agentCheckTimer->start ( 1000 );
	cardReady=true;

	sshEnv=QProcess::systemEnvironment();
	for ( int i=sshEnv.count()-1;i>=0;--i ) //clear gpg variables
	{
		if ( ( sshEnv[i].indexOf ( "GPG_AGENT_INFO" ) !=-1 ) || ( sshEnv[i].indexOf ( "SSH_AUTH_SOCK" ) !=-1 ) ||
		        ( sshEnv[i].indexOf ( "SSH_AGENT_PID" ) !=-1 ) )
		{
			sshEnv.removeAt ( i );
		}
	}
	sshEnv<<envLst[0]<<envLst[2]<<envLst[4];

	if ( !useLdap )
	{
		if ( passForm )
		{
			if ( passForm->isEnabled() )
			{
				if ( login->isEnabled() )
				{
					login->setText ( cardLogin );
					slotSessEnter();
					return;
				}
			}
		}
		QProcess sshadd ( this ); //using it to start scdaemon
		sshadd.setEnvironment ( sshEnv );
		QStringList arguments;
		arguments<<"-l";
		sshadd.start ( "ssh-add",arguments );
		sshadd.waitForFinished ( -1 );
		QString sshout ( sshadd.readAllStandardOutput() );
		sshout=sshout.simplified();
		x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
	}
	else
	{
		if ( selectSessionDlg || sessionStatusDlg )
		{
			QProcess sshadd ( this ); //using it to start scdaemon
			sshadd.setEnvironment ( sshEnv );
			QStringList arguments;
			arguments<<"-l";
			sshadd.start ( "ssh-add",arguments );
			sshadd.waitForFinished ( -1 );
			QString sshout ( sshadd.readAllStandardOutput() );
			sshout=sshout.simplified();
			x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
			return;
		}
		if ( passForm )
			slotClosePass();
		uname->setText ( cardLogin );
		slotUnameEntered();
		slotPassEnter();
	}
}


void ONMainWindow::slot_checkAgentProcess()
{
	if ( checkAgentProcess() )
		return;
	agentCheckTimer->stop();
	cardReady=false;
	if ( cardStarted )
	{
		cardStarted=false;
		if ( nxproxy )
			if ( nxproxy->state() ==QProcess::Running )
				nxproxy->terminate();
	}

	x2goDebug<<"gpg-agent finished\n";
	slot_startPGPAuth();
}


void ONMainWindow::cartReady()
{}


bool ONMainWindow::checkAgentProcess()
{
	QFile file ( "/proc/"+agentPid+"/cmdline" );
	if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString line ( file.readLine() );
		file.close();
		if ( line.indexOf ( "gpg-agent" ) !=-1 )
		{
			return true;
		}
	}
	return false;
}

#if defined ( WINDOWS ) || defined ( Q_OS_DARWIN )
QString ONMainWindow::getXDisplay()
{
	QTcpSocket tcpSocket ( this );
	uint dispNumber=0;
	QString xname,xdir,xopt;
#ifdef WINDOWS
	ConfigDialog::getXSettings ( &dispNumber, &xname, &xdir, &xopt );
#endif
#ifdef Q_OS_DARWIN
	dispNumber=0;
	xdir=ConfigDialog::getXDarwinDirectory();
	xname=xdir+"/Contents/MacOS/X11";
	xopt=" -rootless :0";
#endif
	tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );

	if ( tcpSocket.waitForConnected ( 3000 ) )
	{
		tcpSocket.close();
		return QString::number ( dispNumber );
	}
	if ( xname==QString::null )
	{
#if defined (WINDOWS) || defined (Q_OS_DARWIN)
		QMessageBox::critical ( this,tr ( "Can't connect to X-Server" ),
		                        tr ( "Can't connect to X-Server\nPlease check your settings" ) );
#endif
		slot_config();
		return QString::null;
	}
	QProcess* pr=new QProcess ( this );
	pr->setWorkingDirectory ( xdir );
	pr->start ( xname+" "+xopt,QIODevice::NotOpen );
	if ( pr->waitForStarted ( 3000 ) )
	{
#ifdef WINDOWS
		_sleep ( 1000 );
#endif
#ifdef Q_OS_DARWIN
		//FIXME: the call of unistd.h sleep() do not work on all
		// Mac OS X systems
		system ( "sleep 3" );
#endif
		tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );
		if ( tcpSocket.waitForConnected ( 1000 ) )
		{
			tcpSocket.close();
			return QString::number ( dispNumber );
		}
#if defined (WINDOWS) || defined (Q_OS_DARWIN)
		QMessageBox::critical ( this,tr ( "Can't connect to X-Server" ),
		                        tr ( "Can't connect to X-Server\nPlease check your settings" ) );
#endif
		slot_config();
		return QString::null;
	}
	QMessageBox::critical ( this,tr ( "Can't start X-Server" ),
	                        tr ( "Can't start X Server\nPlease check your settings" ) );
	slot_config();

	return QString::null;
}
#endif
#ifdef WINDOWS
QString ONMainWindow::transform2cygwinPath ( const QString& winPath, bool trunc )
{
	QStringList folders=winPath.split ( "/" );
	for ( int i=0;i<folders.count();++i )
	{
		if ( folders[i].length() >8 && folders[i].indexOf ( ' ' ) !=-1 && trunc ) //hack to transform win home
			//(4 ex /dokummente und einstellungen) in usable by nxproxy format
		{
			folders[i]=folders[i].left ( 6 ) +"~1";
		}
		if ( folders[i].indexOf ( ':' ) ==1 )
		{
			folders[i]="/cygdrive/"+folders[i][0];
		}
	}
	return folders.join ( "/" );
}

QString ONMainWindow::transform2winPath ( const QString& winPath )
{
	QStringList folders=winPath.split ( "/",QString::SkipEmptyParts );
	for ( int i=0;i<folders.count();++i )
	{
		if ( folders[i]=="cygdrive" )
		{
			folders[i]=folders[i+1];
			folders[i+1]=":";
			break;
		}
	}
	return folders.join ( "\\" ).replace ( "\\:",":" );
}

QString ONMainWindow::cygwinHomePath()
{
	QString path=ConfigDialog::getCygwinDir ( "/home" );
	if ( path!=QString::null )
		return path+"\\"+getenv ( "USERNAME" );
	path=ConfigDialog::getCygwinDir ( "/" );
	if ( path!=QString::null )
		return path+"\\home\\"+getenv ( "USERNAME" );
	return QDir::homePath();
}
#endif

bool ONMainWindow::isColorDepthOk ( int disp, int sess )
{
	if ( sess==0 )
		return true;
	if ( disp==sess )
		return true;
	if ( ( disp == 24 || disp == 32 ) && ( sess == 24 || sess == 32 ) )
		return true;
	return false;
}

void ONMainWindow::setWidgetStyle ( QWidget* widget )
{
#ifndef Q_OS_LINUX
	widget->setStyle ( widgetExtraStyle );
#endif
}

QString ONMainWindow::internAppName ( const QString& transAppName, bool* found )
{
	if ( found )
		*found=false;
	int ind=_transApplicationsNames.indexOf ( transAppName );
	if ( ind!=-1 )
	{
		if ( found )
			*found=true;
		return _internApplicationsNames[ind];
	}
	return transAppName;
}


QString ONMainWindow::transAppName ( const QString& internAppName, bool* found )
{
	if ( found )
		*found=false;
	int ind=_internApplicationsNames.indexOf ( internAppName );
	if ( ind!=-1 )
	{
		if ( found )
			*found=true;
		return _transApplicationsNames[ind];
	}
	return internAppName;
}

void ONMainWindow::addToAppNames ( QString intName, QString transName )
{
	_internApplicationsNames.append ( intName );
	_transApplicationsNames.append ( transName );
}


void ONMainWindow::slot_execXmodmap()
{
#ifdef Q_WS_HILDON
	QString passwd=pass->text();
	QString user=login->text();
	QString host=resumingSession.server;
	QString cmd;

	cmd="(xmodmap -pke ;"
	    "echo keycode 73= ;"
// 	    "echo clear shift ;"
// 	    "echo clear lock ;"
// 	    "echo clear control ;"
// 	    "echo clear mod1 ;"
// 	    "echo clear mod2 ;"
// 	    "echo clear mod3 ;"
// 	    "echo clear mod4 ;"
// 	    "echo clear mod5 ;"
//  	    "echo add shift = Shift_L ;"
	    "echo add control = Control_R "
//  	    "echo add mod5 = ISO_Level3_Shift"
	    ")| DISPLAY=:"
	    +resumingSession.display+" xmodmap - ";
	x2goDebug<<"cmd:"<<cmd;
	sshProcess* xmodProc;
	try
	{
		xmodProc=new sshProcess ( this,user,host,sshPort,
		                          cmd,
		                          passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		return;
	}

	if ( cardReady )
	{
		QStringList env=xmodProc->environment();
		env+=sshEnv;
		xmodProc->setEnvironment ( env );
	}
	xmodProc->setFwX ( true );
	xmodProc->startNormal();
#endif
}

void ONMainWindow::slot_sudoErr ( QString stderr, sshProcess* proc )
{
	/*	QMessageBox::critical ( 0l,tr ( "Error" ),tr("Check your sudo configuration"),
		                        QMessageBox::Ok,QMessageBox::NoButton );*/
	proc->setErrorString ( tr ( "<br>Sudo configuration error" ) );
	proc->kill();

}

void ONMainWindow::check_cmd_status()
{
	QString passwd;
	QString user=login->text();
	QString host=resumingSession.server;
	if ( currentKey==QString::null||currentKey=="" )
	{
		passwd=pass->text();
	}

	x2goDebug<<"check command message"<<endl;
	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gocmdexitmessage "+resumingSession.sessionId,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		return;
	}
	
	if ( cardReady )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_cmdMessage ( bool, QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		return;
	}

}

void ONMainWindow::slot_cmdMessage ( bool result,QString output,sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong Password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		pass->setFocus();
		pass->selectAll();
		return;
	}
	if(output.indexOf("X2GORUNCOMMAND ERR NOEXEC:")!=-1)
	{
		QString cmd=output;
		cmd.replace("X2GORUNCOMMAND ERR NOEXEC:","");
		QMessageBox::critical ( 0l,tr ( "Error" ),tr("Unable to execute: ")+cmd,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
		
	
}
