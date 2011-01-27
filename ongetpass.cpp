/***************************************************************************
 *   Copyright (C) 2005 by Oleksandr Shneyder   *
 *   oleksandr.shneyder@obviously-nice.de   *
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "x2goclientconfig.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <onmainwindow.h>
#include <QPlastiqueStyle>
#include <QMessageBox>
#include <iostream>
#include <QFile>
#include <QLocalSocket>
#include "x2gologdebug.h"
using namespace std;
void askpass ( const QString& param, const QString& accept,
               const QString& cookie, const QString& socketName )
{
	QFile fl ( socketName+".log" );
	if ( !fl.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QString message="Unable to write: " +socketName+".log";
		cout<<message.toAscii().data() <<endl;
		exit ( -1 );
	}
	fl.setPermissions (
	    QFile::ReadOwner|QFile::WriteOwner );
	QTextStream out ( &fl );
	out<<param;
	fl.close();
	if ( param.indexOf ( "RSA key" ) !=-1 )
	{
		cout<<accept.toAscii().data() <<endl;
		return;
	}
	if ( cookie=="X2GO_RSA_DSA_KEY_USED" )
		return;
	QLocalSocket sock;
	sock.connectToServer ( socketName );
	if ( !sock.waitForConnected ( 3000 ) )
	{
		QString message="Unable to connect: " +socketName;
		cout<<message.toAscii().data() <<endl;
		cerr<<message.toAscii().data() <<endl;
		exit ( -1 );
	}
	sock.write ( cookie.toAscii().data(),cookie.toAscii().length() );
	if ( !sock.waitForReadyRead() )
	{
		cout<<"Cannot read password from x2goclient"<<endl;
		cerr<<"Cannot read password from x2goclient"<<endl;
		exit ( -1 );
	}
	char buffer[256];
	int read=sock.read ( buffer,255 );
	if ( read<=0 )
	{
		cout<<"Cannot read password from x2goclient"<<endl;
		cerr<<"Cannot read password from x2goclient"<<endl;
		exit ( -1 );
	}
	buffer[read]=0;
	cout<<buffer<<endl;
}

int main ( int argc, char *argv[] )
{
	QString envaccept,envcoockie,envsocket;
	QStringList env = QProcess::systemEnvironment();
	for ( int i=env.count()-1;i>=0;--i )
	{
		if ( env[i].indexOf ( "X2GO_PCOOKIE" ) !=-1 )
		{
			envcoockie=env[i].split ( "=" ) [1];
		}
		if ( env[i].indexOf ( "X2GO_PACCEPT" ) !=-1 )
		{
			envaccept=env[i].split ( "=" ) [1];
		}
		if ( env[i].indexOf ( "X2GO_PSOCKET" ) !=-1 )
		{
			envsocket=env[i].split ( "=" ) [1];
		}
	}
	if ( envaccept.length() >0 && envcoockie.length() >0 )
	{
		askpass ( argv[1],envaccept,envcoockie,envsocket );
		return 0;
	}

	QApplication app ( argc,argv );
	QTranslator x2goclientTranslator;
	QString filename=QString ( ":/x2goclient_%1" ).arg (
	                     QLocale::system().name() );
	filename=filename.toLower();
	if ( !x2goclientTranslator.load ( filename ) )
	{
		qDebug ( "Can't load translator (%s) !\n",
		         filename.toLocal8Bit().data() );
	}
	else
		app.installTranslator ( &x2goclientTranslator );



	QTranslator qtTranslator;
	filename=QString ( ":/qt_%1" ).arg ( QLocale::system().name() );
	if ( !qtTranslator.load ( filename ) )
	{
		qDebug ( "Can't load translator (%s) !\n",
		         filename.toLocal8Bit().data() );
	}
	else
		app.installTranslator ( &qtTranslator );


#ifndef Q_WS_HILDON
#ifdef Q_OS_LINUX
	app.setStyle ( new QPlastiqueStyle() );
#endif
#endif
	
	QStringList args;
	if ( argc > 1 )
		args=app.arguments();
	if ( args.count() >1 && args[1]=="--dialog" )
	{
		QString type=args[2];
		QString caption=args[4];
		caption=caption.replace ( "NX","X2Go" );
		QString text=args[6];
		if ( type=="error" || type=="panic" )
			return QMessageBox::critical ( 0, caption,text );
		if ( type=="ok" )
			return QMessageBox::information ( 0, caption,text );
		if ( type=="yesno" )
			return  QMessageBox::question ( 0, caption,text,
			                                QMessageBox::Yes,
			                                QMessageBox::No );
		return -1;
	}
	else
	{
		ONMainWindow* mw = new ONMainWindow;
		mw->show();
		return app.exec();
	}
}
