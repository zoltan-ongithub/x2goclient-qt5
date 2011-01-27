//
// C++ Implementation: configdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2goclientconfig.h"
#include "configdialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include "onmainwindow.h"
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include "x2gologdebug.h"

ConfigDialog::ConfigDialog ( QWidget * parent, Qt::WFlags f )
		: QDialog ( parent,f )
{

	QVBoxLayout* ml=new QVBoxLayout ( this );
	QFrame *fr=new QFrame ( this );
	QVBoxLayout* frLay=new QVBoxLayout ( fr );
	ml->addWidget ( fr );
	ONMainWindow* par= ( ONMainWindow* ) parent;


#ifndef WINDOWS

	QSettings st ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif

#ifdef USELDAP
	useldap=new QCheckBox ( tr ( "Use LDAP" ),fr );

	frLay->addWidget ( useldap );

	QGroupBox* gb=new QGroupBox ( tr ( "LDAP Settings" ),fr );

	ldapServer=new QLineEdit ( gb );
	port=new QSpinBox ( gb );
	ldapBase=new QLineEdit ( gb );
	port->setMaximum ( 1000000 );

	QHBoxLayout *grLay=new QHBoxLayout ( gb );

	QVBoxLayout *laiLay=new QVBoxLayout();
	QVBoxLayout *setLay=new QVBoxLayout();
	setLay->setSpacing ( 6 );
	laiLay->setSpacing ( 6 );

	grLay->setSpacing ( 20 );
	grLay->addLayout ( laiLay );
	grLay->addStretch();
	grLay->addLayout ( setLay );

	laiLay->addWidget ( new QLabel ( tr ( "Server URL:" ),gb ) );
	laiLay->addWidget ( new QLabel ( tr ( "BaseDN:" ),gb ) );
	laiLay->addWidget ( new QLabel ( tr ( "Failover Server 1 URL:" ),gb ) );
	laiLay->addWidget ( new QLabel ( tr ( "Failover Server 2 URL:" ),gb ) );

	ldapServer1=new QLineEdit ( gb );
	port1=new QSpinBox ( gb );
	ldapServer2=new QLineEdit ( gb );
	port2=new QSpinBox ( gb );
	port1->setMaximum ( 1000000 );
	port2->setMaximum ( 1000000 );


	QHBoxLayout* aLay=new QHBoxLayout();
	aLay->setSpacing ( 3 );
	aLay->addWidget ( new QLabel ( "ldap//:",gb ) );
	aLay->addWidget ( ldapServer );
	aLay->addWidget ( new QLabel ( ":",gb ) );
	aLay->addWidget ( port );

	QHBoxLayout* aLay1=new QHBoxLayout();
	aLay1->setSpacing ( 3 );
	aLay1->addWidget ( new QLabel ( "ldap//:",gb ) );
	aLay1->addWidget ( ldapServer1 );
	aLay1->addWidget ( new QLabel ( ":",gb ) );
	aLay1->addWidget ( port1 );

	QHBoxLayout* aLay2=new QHBoxLayout();
	aLay2->setSpacing ( 3 );
	aLay2->addWidget ( new QLabel ( "ldap//:",gb ) );
	aLay2->addWidget ( ldapServer2 );
	aLay2->addWidget ( new QLabel ( ":",gb ) );
	aLay2->addWidget ( port2 );


	setLay->addLayout ( aLay );
	setLay->addWidget ( ldapBase );
	setLay->addLayout ( aLay1 );
	setLay->addLayout ( aLay2 );


	useldap->setChecked ( st.value ( "LDAP/useldap", ( QVariant ) par->retUseLdap() ).toBool() );
	ldapServer->setText ( st.value ( "LDAP/server", ( QVariant ) par->retLdapServer() ).toString() );
	port->setValue ( st.value ( "LDAP/port", ( QVariant ) par->retLdapPort() ).toInt() );
	ldapServer1->setText ( st.value ( "LDAP/server1", ( QVariant ) par->retLdapServer1() ).toString() );
	port1->setValue ( st.value ( "LDAP/port1", ( QVariant ) par->retLdapPort1() ).toInt() );
	ldapServer2->setText ( st.value ( "LDAP/server2", ( QVariant ) par->retLdapServer2() ).toString() );
	port2->setValue ( st.value ( "LDAP/port2", ( QVariant ) par->retLdapPort2() ).toInt() );
	ldapBase->setText ( st.value ( "LDAP/basedn", ( QVariant ) par->retLdapDn() ).toString() );
	gb->setEnabled ( useldap->isChecked() );
	frLay->addWidget ( gb );
	connect ( useldap,SIGNAL ( toggled ( bool ) ),gb,SLOT ( setEnabled ( bool ) ) );

	connect ( useldap,SIGNAL ( toggled ( bool ) ),this,SLOT ( slot_checkOkStat() ) );
	connect ( ldapBase,SIGNAL ( textChanged ( const QString& ) ),this,SLOT ( slot_checkOkStat() ) );
	connect ( ldapServer,SIGNAL ( textChanged ( const QString& ) ),this,
	          SLOT ( slot_checkOkStat() ) );
#endif  //USELDAP

#ifdef Q_OS_DARWIN
	QGroupBox* xgb=new QGroupBox ( tr ( "X-Server Settings" ),fr );
	QGridLayout *xLay=new QGridLayout ( xgb );

	leXexec=new QLineEdit ( xgb );
	leXexec->setReadOnly ( true );
	pbOpenExec=new QPushButton ( QIcon ( par->iconsPath ( "/32x32/file-open.png" ) ),
	                             QString::null,xgb );
	xLay->addWidget ( new QLabel ( tr ( "X11 Application:" ) ),0,0 );

	leCmdOpt=new QLineEdit ( xgb );
	leCmdOpt->setReadOnly ( true );

	QHBoxLayout* cmdLay=new QHBoxLayout();
	cmdLay->addWidget ( leXexec );
	cmdLay->addWidget ( pbOpenExec );


	xLay->addLayout ( cmdLay,0,1 );
	xLay->addWidget ( new QLabel ( tr ( "X11 Version:" ) ),1,0 );
	xLay->addWidget ( leCmdOpt,1,1 );
	frLay->addWidget ( xgb );

	QString xver;
	QString path=getXDarwinDirectory();
	if ( path!="" )
	{
		leXexec->setText ( findXDarwin ( xver,path ) );
		leCmdOpt->setText ( xver );
	}
	else
		slot_findXDarwin();

	QPushButton* findButton=new QPushButton ( tr ( "Find X11 Application" ),xgb );
	xLay->addWidget ( findButton,2,1 );
	connect ( findButton,SIGNAL ( clicked() ),this,SLOT ( slot_findXDarwin() ) );
	connect ( pbOpenExec,SIGNAL ( clicked() ),this,SLOT ( slot_selectXDarwin() ) );

#endif //Q_OS_DARWIN


#ifdef WINDOWS
	QGroupBox* xgb=new QGroupBox ( tr ( "X-Server Settings" ),fr );
	QGridLayout *xLay=new QGridLayout ( xgb );

	bgRadio=new QButtonGroup ( xgb );
	rbX[0]=new QRadioButton ( tr ( "Xming" ),xgb );
	rbX[1]=new QRadioButton ( tr ( "XWin (Cygwin)" ),xgb );
	rbX[2]=new QRadioButton ( tr ( "Custom X-Server" ),xgb );
	for ( uint i=0;i<3;++i )
		bgRadio->addButton ( rbX[i],i );

	leXexec=new QLineEdit ( xgb );
	pbOpenExec=new QPushButton ( QIcon ( par->iconsPath ( "/32x32/file-open.png" ) ),
	                             QString::null,xgb );
	leCmdOpt=new QLineEdit ( xgb );
	sbDisp=new QSpinBox ( xgb );
	sbDisp->setMinimum ( 0 );
	sbDisp->setMaximum ( 100 );
	leXexecDir=new QLineEdit ( xgb );
	QPushButton* pbOpenExecDir=new QPushButton ( QIcon ( par->iconsPath ( "/32x32/file-open.png" ) ),
	        QString::null,xgb );

	QPushButton* pbDefault=new QPushButton ( tr ( "Reset to defaults" ),xgb );
	xLay->addWidget ( rbX[0],0,0 );
	xLay->addWidget ( rbX[1],1,0 );
	xLay->addWidget ( rbX[2],2,0 );
	xLay->addWidget ( new QLabel ( tr ( "Command:" ) ),3,0 );
	xLay->addWidget ( new QLabel ( tr ( "Arguments:" ) ),4,0 );
	xLay->addWidget ( leCmdOpt,4,1 );
	xLay->addWidget ( new QLabel ( tr ( "Display:" ) ),5,0 );
	xLay->addWidget ( new QLabel ( tr ( "Working directory:" ) ),6,0 );
	xLay->addWidget ( pbDefault,7,0 );

	QHBoxLayout* cmdLay=new QHBoxLayout();
	cmdLay->addWidget ( leXexec );
	cmdLay->addWidget ( pbOpenExec );

	QHBoxLayout* cmdDirLay=new QHBoxLayout();
	cmdDirLay->addWidget ( leXexecDir );
	cmdDirLay->addWidget ( pbOpenExecDir );

	xLay->addLayout ( cmdLay,3,1 );
	xLay->addLayout ( cmdDirLay,6,1 );
	QHBoxLayout* dispLay=new QHBoxLayout();
	dispLay->addWidget ( sbDisp );
	dispLay->addStretch();
	xLay->addLayout ( dispLay,5,1 );
	frLay->addWidget ( xgb );

	QSettings xst ( "Obviously Nice","x2goclient" );
	xst.beginGroup ( "settings" );
	leXexec->setText ( xst.value ( "X/command", ( QVariant ) QString::null ).toString() );
	sbDisp->setValue ( xst.value ( "X/display", ( QVariant ) 0 ).toInt() );
	leXexecDir->setText ( xst.value ( "X/execdir", ( QVariant ) QString::null ).toString() );
	leCmdOpt->setText ( xst.value ( "X/options", ( QVariant ) QString::null ).toString() );
	rbX[2]->setChecked ( true );
	if ( leXexec->text().length() <=0 )
		slotDefaultXSettings();
	connect ( pbDefault,SIGNAL ( clicked() ),this,SLOT ( slotDefaultXSettings() ) );
	connect ( pbOpenExecDir,SIGNAL ( clicked() ),this,SLOT ( slotGetExecDir() ) );
	connect ( pbOpenExec,SIGNAL ( clicked() ),this,SLOT ( slotGetExec() ) );
	connect ( bgRadio,SIGNAL ( buttonClicked ( int ) ),this,SLOT ( slotXSelected ( int ) ) );
	connect ( sbDisp,SIGNAL ( valueChanged ( const QString& ) ),this,SLOT ( slotDispChanged ( const QString& ) ) );

	bool found;
	QString xname,xdir,xopt;
	getXming ( &found, &xname,&xdir, &xopt );
	rbX[0]->setEnabled ( found );
	if ( leXexec->text() =="XMING" )
	{
		if ( !found )
		{
			leXexec->setText ( QString::null );
			leXexecDir->setText ( QString::null );
			leCmdOpt->setText ( QString::null );
			slotDefaultXSettings();
		}
		else
		{
			leXexec->setText ( xname );
			rbX[0]->setChecked ( true );
			leXexec->setReadOnly ( true );
			pbOpenExec->setEnabled ( false );
		}
	}
	getCygwin ( &found, &xname,&xdir, &xopt );
	rbX[1]->setEnabled ( found );
	if ( leXexec->text() =="CYGWIN" )
	{
		if ( !found )
		{
			leXexec->setText ( QString::null );
			leXexecDir->setText ( QString::null );
			leCmdOpt->setText ( QString::null );
			slotDefaultXSettings();
		}
		else
		{
			leXexec->setText ( xname );
			rbX[1]->setChecked ( true );
			leXexec->setReadOnly ( true );
			pbOpenExec->setEnabled ( false );
		}
	}


#endif //WINDOWS
	clientSshPort=new QSpinBox ( fr );
	clientSshPort->setMaximum ( 1000000 );
	clientSshPort->setValue ( st.value ( "clientport", ( QVariant ) 22 ).toInt() );

	QHBoxLayout* sshLay=new QHBoxLayout();
	sshLay->addWidget ( new QLabel ( tr ( "Clientside SSH Port For File System Export Usage:" ),fr ) );
	sshLay->addWidget ( clientSshPort );
	sshLay->addStretch();
	frLay->addLayout ( sshLay );

	frLay->addStretch();
	ok=new QPushButton ( tr ( "&OK" ),this );
	QPushButton* cancel=new QPushButton ( tr ( "&Cancel" ),this );
	QHBoxLayout* bLay=new QHBoxLayout();


	connect ( this,SIGNAL ( accepted() ),this,SLOT ( slot_accepted() ) );
	connect ( ok,SIGNAL ( clicked() ),this,SLOT ( accept() ) );
	connect ( cancel,SIGNAL ( clicked() ),this,SLOT ( reject() ) );


	bLay->setSpacing ( 5 );
	bLay->addStretch();
	bLay->addWidget ( ok );
	bLay->addWidget ( cancel );
	ml->addLayout ( bLay );

	fr->setFrameStyle ( QFrame::StyledPanel | QFrame::Raised );
	fr->setLineWidth ( 2 );

	setSizeGripEnabled ( true );
	setWindowIcon ( QIcon ( ( ( ONMainWindow* ) parent )->iconsPath ( "/32x32/edit_settings.png" ) ) );
	setWindowTitle ( tr ( "Settings" ) );

#ifdef Q_WS_HILDON
	QFont fnt=font();
	fnt.setPointSize ( 10 );
	setFont ( fnt );
	QSize sz=ok->sizeHint();
	sz.setWidth((int)(sz.width()/1.5));
	sz.setHeight((int)(sz.height()/1.5));
	ok->setFixedSize ( sz );
	sz=cancel->sizeHint();
	sz.setWidth((int)(sz.width()));
	sz.setHeight((int)(sz.height()/1.5));
	cancel->setFixedSize ( sz );
        clientSshPort->setFixedHeight(int(clientSshPort->sizeHint().height()*1.5));
#endif

}


ConfigDialog::~ConfigDialog()
{}


void ConfigDialog::slot_accepted()
{
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif

#ifdef USELDAP
	st.setValue ( "LDAP/useldap", ( QVariant ) useldap->isChecked() );
	st.setValue ( "LDAP/port", ( QVariant ) port->value() );
	if ( ldapServer->text().length() )
		st.setValue ( "LDAP/server", ( QVariant ) ldapServer->text() );
	st.setValue ( "LDAP/port1", ( QVariant ) port1->value() );
	if ( ldapServer1->text().length() )
		st.setValue ( "LDAP/server1", ( QVariant ) ldapServer1->text() );
	st.setValue ( "LDAP/port2", ( QVariant ) port2->value() );
	if ( ldapServer2->text().length() )
		st.setValue ( "LDAP/server2", ( QVariant ) ldapServer2->text() );
	if ( ldapBase->text().length() )
		st.setValue ( "LDAP/basedn", ( QVariant ) ldapBase->text() );
#endif //USELDAP
#ifdef WINDOWS

	QSettings xst ( "Obviously Nice","x2goclient" );
	xst.beginGroup ( "settings" );
	switch ( bgRadio->checkedId() )
	{
		case XMING:
			xst.setValue ( "X/command", ( QVariant ) "XMING" );
			break;
		case CYGWIN:
			xst.setValue ( "X/command", ( QVariant ) "CYGWIN" );
			break;
		case CUSTOM:
			xst.setValue ( "X/command", ( QVariant ) leXexec->text() );
			break;
	}
	xst.setValue ( "X/display", ( QVariant ) sbDisp->value() );
	xst.setValue ( "X/execdir", ( QVariant ) leXexecDir->text() );
	xst.setValue ( "X/options", ( QVariant ) leCmdOpt->text() );
#endif
#ifdef Q_OS_DARWIN
	st.setValue ( "xdarwin/directory", ( QVariant ) leXexec->text() );
#endif
	st.setValue ( "clientport", ( QVariant ) clientSshPort->value() );

}


void ConfigDialog::slot_checkOkStat()
{
	ok->setEnabled ( ( !useldap->isChecked() ) || ( ( ldapBase->text().length() &&
	                 ldapServer->text().length() ) ) );
}


#ifdef WINDOWS
void ConfigDialog::getXming ( bool* found, QString* execName,QString* execDir, QString* options )
{
	*found=false;
	QString xming;
	if ( getenv ( "ProgramFiles" ) !=0 )
	{
		xming=getenv ( "ProgramFiles" );
		xming+="\\Xming\\Xming.exe";
		if ( QFile::exists ( xming ) )
		{
			*found=true;
			*execName=xming;
			*options="-multiwindow -clipboard";
			*execDir=getenv ( "ProgramFiles" );
			*execDir+="\\Xming";
			return;
		}
	}
	QSettings st ( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Xming_is1",QSettings::NativeFormat );
	xming=st.value ( "Inno Setup: App Path", ( QVariant ) QString::null ).toString();
	if ( xming.length() >0 && QFile::exists ( xming+"\\Xming.exe" ) )
	{
		*found=true;
		*execDir=xming;
		*options="-multiwindow -clipboard";
		*execName=xming+"\\Xming.exe";
	}
}

void ConfigDialog::getCygwin ( bool* found, QString* execName,QString* execDir, QString* options )
{
	*found=false;
	QString cygbin;
	cygbin=getCygwinDir ( "/" );
	QString cygWin= ( cygbin.split ( "/" ) [0] );
	cygWin+="\\usr\\X11R6\\bin\\XWin.exe";
	cygbin.replace ( "/","\\" );
	if ( cygbin.length() >0 && QFile::exists ( cygWin ) )
	{
		*found=true;
		*execDir=cygbin+"\\bin";
		*options="-multiwindow -clipboard";
		*execName=cygWin;
	}
}

void ConfigDialog::slotDefaultXSettings()
{
	bool found;
	QString execName;
	QString execDir;
	QString options;

	getXming ( &found, &execName,&execDir, &options );
	sbDisp->setValue ( 0 );
	rbX[0]->setEnabled ( found );
	if ( found )
	{
		rbX[0]->setChecked ( true );
		leXexec->setText ( execName );
		leXexecDir->setText ( execDir );
		leCmdOpt->setText ( options+" :0" );
	}
	getCygwin ( &found, &execName,&execDir, &options );
	rbX[1]->setEnabled ( found );
	if ( found && !rbX[0]->isEnabled() )
	{
		rbX[1]->setChecked ( true );
		leXexec->setText ( execName );
		leXexecDir->setText ( execDir );
		leCmdOpt->setText ( options+" :0" );
	}
	if ( !rbX[1]->isEnabled() &&!rbX[0]->isEnabled() )
		rbX[2]->setChecked ( true );
}

void ConfigDialog::slotGetExecDir()
{
	QString newDir=QFileDialog::getExistingDirectory ( this,QString(),leXexecDir->text() );
	if ( newDir.length() >0 )
		leXexecDir->setText ( newDir );
}

void ConfigDialog::slotGetExec()
{
	bool xmingFound;
	QString execXmingName;
	QString execXmingDir;
	QString xmingOptions;

	bool  cygFound;
	QString execCygName;
	QString execCygDir;
	QString cygOptions;

	getXming ( &xmingFound, &execXmingName,&execXmingDir, &xmingOptions );
	getCygwin ( &cygFound, &execCygName,&execCygDir, &cygOptions );


	xmingFound=cygFound=false;
	if ( !xmingFound && !cygFound )
	{
		QString newFile=
		    QFileDialog::getOpenFileName ( this,QString(),leXexecDir->text(),tr ( "Applications (*.exe);;All Files (*.*)" ) );
		if ( newFile.length() >0 )
			leXexec->setText ( newFile );
		return;
	}
}

void ConfigDialog::slotXSelected ( int id )
{

	bool found;
	QString execName=leXexec->text();
	QString execDir=leXexecDir->text();
	QString options=leCmdOpt->text();
	switch ( id )
	{
		case XMING:
		{
			leXexec->setReadOnly ( true );
			pbOpenExec->setEnabled ( false );
			getXming ( &found, &execName,&execDir, &options );
			if ( !found )
			{
				slotDefaultXSettings();
				return;
			}
			options+=" :"+QString::number ( sbDisp->value() );
		}
		break;

		case CYGWIN:
		{
			leXexec->setReadOnly ( true );
			pbOpenExec->setEnabled ( false );
			getCygwin ( &found, &execName,&execDir, &options );
			if ( !found )
			{
				slotDefaultXSettings();
				return;
			}
			options+=" :"+QString::number ( sbDisp->value() );
		}
		break;
		case CUSTOM:
		{
			leXexec->setReadOnly ( false );
			pbOpenExec->setEnabled ( true );
		}
		break;
	}
	leXexec->setText ( execName );
	leXexecDir->setText ( execDir );
	leCmdOpt->setText ( options );
}

void ConfigDialog::slotDispChanged ( const QString& val )
{
	if ( bgRadio->checkedId() ==CUSTOM )
		return;
	QString opt=leCmdOpt->text();
	QStringList args=opt.split ( ":" );
	leCmdOpt->setText ( args[0].trimmed() +" :"+val );
}

void ConfigDialog::getXSettings ( uint *display, QString* execName,QString* execDir, QString* options )
{
	QSettings xst ( "Obviously Nice","x2goclient" );
	xst.beginGroup ( "settings" );
	QString xname=xst.value ( "X/command", ( QVariant ) QString::null ).toString();
	int disp=xst.value ( "X/display", ( QVariant ) 0 ).toInt();
	QString xdir=xst.value ( "X/execdir", ( QVariant ) QString::null ).toString();
	QString opt=xst.value ( "X/options", ( QVariant ) QString::null ).toString();
	*display=disp;
	if ( xname.length() >0 && xname!="XMING" && xname !="CYGWIN" )
	{
		*execName=xname;
		*execDir=xdir;
		*options=opt;
		return;
	}

	bool found;
	QString xopt;
	if ( xname.length() <=0 )
	{
		opt=QString::null;
		*display=0;
	}
	if ( xname.length() <=0||xname=="XMING" )
	{
		getXming ( &found, &xname,&xdir, &xopt );
		if ( found )
		{
			if ( opt==QString::null )
			{
				opt=xopt;
				*display=0;
			}
			*execName=xname;
			*execDir=xdir;
			*options=opt;
			return;
		}
		opt=QString::null;
	}
	getCygwin ( &found, &xname,&xdir, &xopt );
	if ( found )
	{
		if ( opt==QString::null )
		{
			opt=xopt;
			*display=0;
		}
		*execName=xname;
		*execDir=xdir;
		*options=opt;
		return;
	}
	getXming ( &found, &xname,&xdir, &xopt );
	if ( found )
	{
		*execName=xname;
		*execDir=xdir;
		*options=xopt;
	}
}

QString ConfigDialog::getCygwinDir ( const QString& dir )
{
	QString cygdir=QString::null;
	QSettings lu_st ( "HKEY_CURRENT_USER\\Software\\Cygnus Solutions\\Cygwin\\mounts v2\\"+dir,QSettings::NativeFormat );
	cygdir=lu_st.value ( "native", ( QVariant ) QString::null ).toString();
	if ( cygdir!= QString::null )
		return cygdir;
	QSettings lm_st ( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\"+dir,QSettings::NativeFormat );
	return lm_st.value ( "native", ( QVariant ) QString::null ).toString();
}
#endif

#ifdef Q_OS_DARWIN
QString ConfigDialog::retMaxXDarwinVersion ( QString v1, QString v2 )
{
	QStringList vl1=v1.split ( "." );
	QStringList vl2=v2.split ( "." );
	for ( int i=0;i<3;++i )
	{
		if ( vl1.count() <i+1 )
			vl1<<"0";
		if ( vl2.count() <i+1 )
			vl2<<"0";
		if ( vl1[i].toInt() !=vl2[i].toInt() )
			return ( vl1[i].toInt() > vl2[i].toInt() ) ?v1:v2;
	}
	return v1;
}

QString ConfigDialog::findXDarwin ( QString& version, QString path )
{
	if ( path=="" )
	{
		QString dir1="/Applications/Utilities/X11.app";
		QString ver1="0.0.0";
		if ( QFile::exists ( dir1+"/Contents/Info.plist" ) )
		{
			QSettings vst ( dir1+"/Contents/Info.plist",QSettings::NativeFormat );
			ver1=vst.value ( "CFBundleShortVersionString", ( QVariant ) "0.0.0" ).toString();
		}
		QString dir2="/usr/X11/X11.app";
		QString ver2="0.0.0";;
		if ( QFile::exists ( dir2+"/Contents/Info.plist" ) )
		{
			QSettings vst ( dir2+"/Contents/Info.plist",QSettings::NativeFormat );
			ver2=vst.value ( "CFBundleShortVersionString", ( QVariant ) "0.0.0" ).toString();
		}
		if ( retMaxXDarwinVersion ( ver1,ver2 ) ==ver1 )
		{
			version=ver1;
			return dir1;
		}
		else
		{
			version=ver2;
			return dir2;
		}
	}
	version="0.0.0";
	if ( QFile::exists ( path+"/Contents/Info.plist" ) )
	{
		QSettings vst ( path+"/Contents/Info.plist",QSettings::NativeFormat );
		version=vst.value ( "CFBundleShortVersionString", ( QVariant ) "0.0.0" ).toString();
	}
	return path;
}


void ConfigDialog::slot_findXDarwin()
{
	QString version;
	QString path=findXDarwin ( version );
	if ( path=="" )
	{
		QMessageBox::warning ( this,tr ( "Warning" ), tr ( "x2goclient could not find any suitable X11 Application. Please install Apple X11 or select the path to the application" ) );
	}
	QString minVer="2.1.0";
	if ( retMaxXDarwinVersion ( minVer,version ) ==minVer )
	{
		printXDarwinVersionWarning ( version );
	}
	leXexec->setText ( path );
	leCmdOpt->setText ( version );

}

void ConfigDialog::printXDarwinVersionWarning ( QString version )
{
	QMessageBox::warning ( this,tr ( "Warning" ),
	                       tr ( "Your are using X11 (Apple X-Window Server) version " )
	                       +version+
	                       tr ( ". This version causes problems with X-application in 24bit color mode. You should update your X11 environment (http://trac.macosforge.org/projects/xquartz)." ) );
}


void ConfigDialog::slot_selectXDarwin()
{
	QString newDir=QFileDialog::getOpenFileName ( this,QString(),leXexec->text() +"/.." );
	QString version;
	if ( newDir.length() >0 )
	{
		findXDarwin ( version,newDir );
		if ( version=="0.0.0" )
		{
			QMessageBox::warning ( this, tr ( "Warning" ),tr ( "No suitable X11 application found in selected path" ) );
			return;
		}
		QString minVer="2.1.0";
		if ( retMaxXDarwinVersion ( minVer,version ) ==minVer )
		{
			printXDarwinVersionWarning ( version );
		}
		leXexec->setText ( newDir );
		leCmdOpt->setText ( version );
	}
}
QString ConfigDialog::getXDarwinDirectory()
{
	QSettings st ( QDir::homePath() +"/.x2goclient/settings",QSettings::NativeFormat );
	return st.value ( "xdarwin/directory", ( QVariant ) "" ).toString() ;
}
#endif


