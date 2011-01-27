//
// C++ Implementation: sshprocess
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
#include "sshprocess.h"
#include <QDir>
#include "x2gologdebug.h"
#include <QMessageBox>
#include <QTemporaryFile>
#include <QApplication>

sshProcess::sshProcess ( QObject* parent,const QString& user, const QString& host,const QString& pt,
                         const QString& cmd,const QString& pass, const QString& key, bool acc )
		: QProcess ( parent )
{
	sudoErr=false;
	QString root=QDir::homePath() +"/.x2go";
	isTunnel=false;
	isCopy=false;
	fwX=false;
	sshPort=pt;
	askpass=root+"/ssh";
	QDir dr ( askpass );
	if ( !dr.exists() )
		if ( !dr.mkpath ( askpass ) )
		{
			QString message=tr ( "Unable to create: " );
			message+=askpass;
			throw message;
		}
	askpass+="/askpass";
	QTemporaryFile fl ( askpass );
	fl.open();
	askpass=fl.fileName();
	fl.setAutoRemove ( false );
	fl.close();
	this->user=user;
	this->host=host;
	command=cmd;
	this->pass=pass;
	this->key=key;
	autoAccept=acc;
	QStringList env = QProcess::systemEnvironment();
	for ( int i=env.count()-1;i>=0;--i ) //clear gpg variables
	{
		if ( ( env[i].indexOf ( "GPG_AGENT_INFO" ) !=-1 ) || ( env[i].indexOf ( "SSH_AUTH_SOCK" ) !=-1 ) ||
		        ( env[i].indexOf ( "SSH_AGENT_PID" ) !=-1 ) )
		{
			env.removeAt ( i );
		}
	}
#ifndef WINDOWS
	env.insert ( 0, "SSH_ASKPASS="+askpass );
#else

	env.insert ( 0, "SSH_ASKPASS=winaskpass" );
	env.insert ( 0, "SSH_PASSFILE="+askpass );
	env.insert ( 0, "DISPLAY=localhost:0" );   //don't care if real display is not 0
	//we need it only to start winaskpass
	//which is not X application
#endif

	setEnvironment ( env );
}


sshProcess::~sshProcess()
{
	QFile::remove
	( askpass );
	QFile::remove
	( askpass+".log" );
	if ( state() ==QProcess::Running )
		kill();
}


void sshProcess::slot_error ( QProcess::ProcessError )
{}


void sshProcess::slot_finished ( int exitCode, QProcess::ExitStatus status )
{
	hidePass();
// 	QString resout ( readAllStandardOutput() );
	x2goDebug<<outputString<<endl;
	x2goDebug<<"exitCode: "<<exitCode<<" status:"<<status<<endl;
	if ( ( exitCode!=0&&exitCode!=1 ) || status !=0 )
	{
		QString resp=getResponce();
		if ( errorString.indexOf ( "Host key verification failed" ) !=-1 )
		{
			int res;
			if ( !autoAccept )
				res=QMessageBox::warning ( 0l,errorString,resp,tr ( "Yes" ),tr ( "No" ) );
			else
				res=0;
			if ( res==0 )
			{
				if ( isTunnel )
					startTunnel ( tunnelHost,localPort,remotePort,reverse,true );
				else if ( isCopy )
					start_cp ( source,destination,true );
				else
					startNormal ( true );
			}
			else
				emit sshFinished ( false,host+":\n"+errorString,this );
		}
		else
			emit sshFinished ( false,host+":\n"+errorString,this );
	}
	else
		emit sshFinished ( true,outputString,this );
}


void sshProcess::slot_stderr()
{
	QString reserr ( readAllStandardError() );
	errorString+=reserr;
// 	x2goDebug<<reserr<<endl;
	if ( reserr.indexOf ( "Permission denied (publickey,keyboard-interactive)" ) !=-1 )
	{
		needPass=true;
		kill();
	}
	if ( reserr.indexOf ( "Authentication succeeded" ) !=-1 )
	{
		hidePass();
		emit sshTunnelOk();
	}
	if (( reserr.indexOf ( "Password:" ) !=-1 )||(reserr.indexOf ( "[sudo] password for" ) !=-1))
	{
		sudoErr=true;
		emit sudoConfigError ( errorString, this );
	}
}


void sshProcess::slot_stdout()
{
	if ( isTunnel )
	{
		QString resout ( readAllStandardOutput() );
	}
	else
	{
		QString reserr ( readAllStandardOutput() );
		outputString+=reserr;
// 		x2goDebug<<reserr<<endl;

	}
}

void sshProcess::startNormal ( bool accept )
{
	x2goDebug<<"normal"<<endl;
	x2goDebug<<command<<endl;
	x2goDebug<<"host:"<<host<<endl;
	errorString="";
	needPass=false;
	disconnect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	             SLOT ( slot_error ( QProcess::ProcessError ) ) );
	disconnect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	             SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	disconnect ( this,SIGNAL ( readyReadStandardError() ),this,
	             SLOT ( slot_stderr() ) );
	disconnect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	             SLOT ( slot_stdout() ) );

	connect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	          SLOT ( slot_error ( QProcess::ProcessError ) ) );
	connect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	connect ( this,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_stderr() ) );
	connect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_stdout() ) );

	QString cmX;
	if ( fwX )
	{
		cmX=" -X ";
	}
	if ( key!=QString::null && key!="" )
	{
		printKey ( accept );
#ifndef  WINDOWS

		start ( setsid() +" ssh "+cmX+"-i "+key+" -p "+sshPort+" "+user+"@"+host+" \""+command+"\"" );
#else
#endif

		start ( "ssh "+cmX+"-i "+key+" -p "+sshPort+" "+user+"@"+host+" \""+command+"\"" );
	}
	else
	{
		printPass ( accept );
#ifndef  WINDOWS

		start ( setsid() +" ssh" +cmX+" -p "+sshPort+" "+user+"@"+host+" \""+command+"\"" );
#else

		start ( "ssh "+cmX+"-p "+sshPort+" "+user+"@"+host+" \""+command+"\"" );
#endif

	}
}


void sshProcess::printPass ( bool accept )
{
	QFile fl ( askpass );
	if ( !fl.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to write: " ) +askpass;
		throw message;
	}
	fl.setPermissions ( QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );
	QTextStream out ( &fl );
#ifndef WINDOWS

	out<<"#!/usr/bin/perl\n\
	$param=shift;\n\
	open (F, \">"<<askpass<<".log\");\
	print F $param;\
	close (F);\
	if($param =~ m/RSA key/)\
{";
	if ( accept )
		out<<"print \"yes\\n\";";
	else
		out<<"print \"no\\n\";";
	out<<"}\
	print \""<<pass<<"\\n\";";
#else

	out<<accept<<" "<<pass;
#endif

	fl.close();
}

void sshProcess::printKey ( bool accept )
{
	QFile fl ( askpass );
	if ( !fl.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to write: " ) +askpass;
		throw message;
	}
	fl.setPermissions ( QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );
	QTextStream out ( &fl );
#ifndef WINDOWS

	out<<"#!/usr/bin/perl\n\
	$param=shift;\n\
	open (F, \">"<<askpass<<".log\");\
	print F $param;\
	close (F);\
	if($param =~ m/RSA key/)\
{";
	if ( accept )
		out<<"print \"yes\\n\";}";
	else
		out<<"print \"no\\n\";}";
#else

	out<<accept<<" null";
#endif

	fl.close();
}

QString sshProcess::getResponce()
{
	QFile fl ( askpass+".log" );
	if ( !fl.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return QString::null;
	}
	QTextStream in ( &fl );
	return in.readAll();
}

void sshProcess::hidePass()
{
	QFile fl ( askpass );
	if ( !fl.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		return;
	}
	fl.setPermissions ( QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );
	QTextStream out ( &fl );
	for ( int i=0;i<1024;++i )
		out<<"X";
	fl.close();
}

void sshProcess::startTunnel ( QString h,QString lp,QString rp,bool rev,bool accept )
{
	x2goDebug<<"tunnel"<<endl;
	isTunnel=true;
	errorString="";
	needPass=false;

	tunnelHost=h;
	localPort=lp;
	remotePort=rp;
	reverse=rev;

	disconnect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	             SLOT ( slot_error ( QProcess::ProcessError ) ) );
	disconnect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	             SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	disconnect ( this,SIGNAL ( readyReadStandardError() ),this,
	             SLOT ( slot_stderr() ) );
	disconnect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	             SLOT ( slot_stdout() ) );

	connect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	          SLOT ( slot_error ( QProcess::ProcessError ) ) );
	connect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	connect ( this,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_stderr() ) );
	connect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_stdout() ) );

	QString params=" -N ";
	if ( reverse )
		params+=" -R ";
	else
		params+=" -L ";
	params+=localPort+":"+tunnelHost+":"+remotePort;
	if ( key!=QString::null && key!="" )
	{
		printKey ( accept );
#ifndef  WINDOWS

		start ( setsid() +" ssh -v -i "+key+" -p "+sshPort+" "+user+"@"+host+params );
#else

		start ( "ssh -v -i "+key+" -p "+sshPort+" "+user+"@"+host+params );
#endif

	}
	else
	{
		printPass ( accept );
#ifndef  WINDOWS

		start ( setsid() +" ssh -v "+user+"@"+host+params+" -p "+sshPort );
#else

		start ( "ssh -v "+user+"@"+host+params+" -p "+sshPort );
#endif

	}
}

void sshProcess::start_cp ( QString src, QString dst, bool accept )
{
	x2goDebug<<"copy"<<endl;
	isCopy=true;
	errorString="";
	needPass=false;
	source=src;
	destination=dst;
	disconnect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	             SLOT ( slot_error ( QProcess::ProcessError ) ) );
	disconnect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	             SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	disconnect ( this,SIGNAL ( readyReadStandardError() ),this,
	             SLOT ( slot_stderr() ) );
	disconnect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	             SLOT ( slot_stdout() ) );

	connect ( this,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	          SLOT ( slot_error ( QProcess::ProcessError ) ) );
	connect ( this,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_finished ( int,QProcess::ExitStatus ) ) );
	connect ( this,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_stderr() ) );
	connect ( this,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_stdout() ) );

	if ( key!=QString::null && key!="" )
	{
		printKey ( accept );
#ifndef  WINDOWS

		start ( setsid() +" scp -i \""+key+"\" -P "+sshPort+" "+" \""+src+"\" "+user+"@"+host+":"+dst );
#else

		start ( "scp -i \""+key+"\" -P "+sshPort+" "+" \""+src+"\" "+user+"@"+host+":"+dst );

#endif

	}
	else
	{
		printPass ( accept );
#ifndef  WINDOWS

		start ( setsid() +" scp -P "+sshPort+" \""+src+"\" "+user+"@"+host+":"+dst );
#else

		start ( "scp -P "+sshPort+" \""+src+"\" "+user+"@"+host+":"+dst );
#endif
	}
}


QString sshProcess::setsid()
{
#ifdef Q_OS_DARWIN
	//run setsid from bundle
	QDir dir ( QApplication::applicationDirPath() );
	dir.cdUp();
	dir.cd ( "exe" );
	return "\""+dir.absolutePath() +"/setsid\"";
#else
#ifdef Q_WS_HILDON
	return "/usr/lib/x2go/setsid";
#endif
	return "setsid";
#endif
}

void sshProcess::setErrorString ( const QString& str )
{
	if ( sudoErr )
		errorString=str;
}
