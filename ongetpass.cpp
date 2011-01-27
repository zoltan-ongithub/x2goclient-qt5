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
using namespace std;
int main ( int argc, char *argv[] )
{
	QApplication app ( argc,argv );
	QTranslator x2goclientTranslator;
	QString filename=QString ( ":/x2goclient_%1" ).arg ( QLocale::system().name() );
	filename=filename.toLower();
	if ( !x2goclientTranslator.load ( filename ) )
	{
		qDebug ( "Can't load translator (%s) !\n",filename.toLocal8Bit().data() );
	}
	else
		app.installTranslator ( &x2goclientTranslator );



	QTranslator qtTranslator;
	filename=QString ( ":/qt_%1" ).arg ( QLocale::system().name() );
	if ( !qtTranslator.load ( filename ) )
	{
		qDebug ( "Can't load translator (%s) !\n",filename.toLocal8Bit().data() );
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
	if ( args.count()>1 && args[1]=="--dialog" )
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
			                                QMessageBox::Yes,QMessageBox::No );
		return -1;
	}
	else
	{
		ONMainWindow* mw = new ONMainWindow;
		mw->show();
		return app.exec();
	}
}
