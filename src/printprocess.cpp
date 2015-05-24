/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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
#include "printprocess.h"
#include "x2gologdebug.h"
#include <QFile>
#include "x2gosettings.h"
#include <QDir>
#include "printdialog.h"
#if (!defined Q_OS_WIN) && (!defined Q_WS_HILDON)
#include "cupsprint.h"
#else
#include "printwidget.h"
#endif
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include "x2gologdebug.h"
#ifdef Q_OS_WIN
#include "wapi.h"
#endif
#include <QTimer>
PrintProcess::PrintProcess ( QString pdf, QString title, QObject *parent ) :
		QObject ( parent )
{
	x2goDebug<<"Starting print process.";
	pdfFile=pdf;
	pdfTitle=title;
	parentWidg= ( QWidget* ) parent;
	if ( !loadSettings() )
	{
		QFile::remove ( pdfFile );
		return;
	}
	if ( viewPdf )
	{
		x2goDebug<<"Opening PDF file: "<<pdfFile<<"; title: "<<pdfTitle;
		QTimer::singleShot ( 100, this, SLOT ( openPdf() ) );
	}
	else
	{
		x2goDebug<<"Printing PDF file: "<<pdfFile<<"; title: "<<pdfTitle;
		QTimer::singleShot ( 100, this, SLOT ( print() ) );
	}
}


PrintProcess::~PrintProcess()
{
	x2goDebug<<"Closing print process.";
}


void PrintProcess::slot_processFinished ( int exitCode,
        QProcess::ExitStatus exitStatus )
{
	disconnect ( this,SIGNAL ( finished ( int , QProcess::ExitStatus ) ),
	             this,SLOT ( slot_processFinished (
	                             int,QProcess::ExitStatus ) ) );
	QFile::remove ( pdfFile );
	if ( exitCode==0 && exitStatus==QProcess::NormalExit )
	{
		if ( !printStdIn )
		{
			if ( !QProcess::startDetached (
			            printCmd+" \""+psFile+"\"" ) )
				slot_error ( QProcess::FailedToStart );
		}
		else
		{
			QProcess* proc=new QProcess;
			proc->setStandardInputFile ( psFile );
			connect ( proc,SIGNAL (
			              error ( QProcess::ProcessError ) ),
			          this,SLOT (
			              slot_error (
			                  QProcess::ProcessError ) ) );
			proc->start ( printCmd );
		}
	}
	else
		slot_pdf2psError ( QProcess::Crashed );
}


bool PrintProcess::loadSettings()
{
	X2goSettings st ( "printing" );

	if ( st.setting()->value ( "showdialog",true ).toBool() )
	{
		PrintDialog dlg;
		if ( dlg.exec() ==QDialog::Rejected )
			return false;
	}

	viewPdf=st.setting()->value ( "pdfview",false ).toBool();
	customPrintCmd=st.setting()->value ( "print/startcmd",false ).toBool();
	printCmd=st.setting()->value ( "print/command","lpr" ).toString();
	printStdIn= st.setting()->value ( "print/stdin",false ).toBool();
	printPs=st.setting()->value ( "print/ps",false ).toBool();

	pdfOpen= st.setting()->value ( "view/open",true ).toBool();

#ifndef Q_OS_WIN
	pdfOpenCmd=st.setting()->value ( "view/command","xpdf" ).toString();
#else
	winX2goPrinter=
	    st.setting()->value ( "print/defaultprinter",
	               wapiGetDefaultPrinter() ).toString();
#endif
#ifdef Q_WS_HILDON
	pdfOpenCmd="run-standalone.sh dbus-send --print-reply"
	           " --dest=com.nokia.osso_pdfviewer "
	           "/com/nokia/osso_pdfviewer "
	           "com.nokia.osso_pdfviewer.mime_open string:file://";
	viewPdf=true;
#endif
	return true;

}


void PrintProcess::openPdf()
{
	x2goDebug<<"Opening/saving PDF ...";
	if ( pdfOpen )
	{
#ifndef Q_OS_WIN
#ifndef Q_WS_HILDON
		QString cmd=pdfOpenCmd+" \""+pdfFile+"\"";
#else
		QString cmd=pdfOpenCmd+"\""+pdfFile+"\"";
#endif
		x2goDebug<<"Using PDF viewer command: "<<cmd;
		if ( ! QProcess::startDetached ( cmd ) )
			slot_error ( QProcess::FailedToStart );

#else
		wapiShellExecute ( "open",
		                   wapiShortFileName ( pdfFile ),
		                   QString::null,
		                   wapiShortFileName ( QDir::homePath() ) );
#endif
	}
	else
	{
		QString homePath=QDir::homePath();
#ifndef Q_OS_WIN
		homePath +="/"+pdfTitle+".pdf";
#else
		homePath +="\\x2goprint.pdf";
#endif
		QString fileName = QFileDialog::getSaveFileName ( 0,
		                   tr ( "Save File" ),					
		                   homePath,
		                   tr ( "PDF Document (*.pdf)" ) );
		if ( fileName.length() >0 )
			QFile::rename ( pdfFile,fileName );

	}
}

void PrintProcess::print()
{
#ifndef Q_WS_HILDON
	if ( !customPrintCmd )
	{
#ifndef Q_OS_WIN
		CUPSPrint prn;
		prn.setCurrentPrinter ( prn.getDefaultUserPrinter() );
		prn.print ( pdfFile, pdfTitle );
#else
		x2goDebug<<"Printing to "<<winX2goPrinter;
		wapiShellExecute ( "printto",
		                   wapiShortFileName ( pdfFile ),
		                   winX2goPrinter,
		                   wapiShortFileName ( QDir::homePath() ) );
#endif
	}
	else
#endif //Q_WS_HILDON
	{
		if ( !printPs )
		{
			if ( !printStdIn )
			{
				if ( !QProcess::startDetached (
				            printCmd+
				            " \""+pdfFile+"\"" ) )
					slot_error ( QProcess::FailedToStart );

			}
			else
			{
				QProcess* proc=new QProcess;
				proc->setStandardInputFile ( pdfFile );
				connect ( proc,SIGNAL (
				              error (
				                  QProcess::ProcessError ) ),
				          this,SLOT (
				              slot_error (
				                  QProcess::ProcessError ) ) );
				proc->start ( printCmd );
			}
		}
		else
		{
			QStringList args;
			psFile=pdfFile;
			psFile.replace ( "pdf","ps" );
			args<<pdfFile<<psFile;
			QProcess* proc=new QProcess;
			connect ( proc,SIGNAL (
			              finished ( int ,
			                         QProcess::ExitStatus ) ),
			          this,SLOT (
			              slot_processFinished (
			                  int,
			                  QProcess::ExitStatus ) ) );
			connect ( proc,SIGNAL (
			              error ( QProcess::ProcessError ) ),
			          this,SLOT (
			              slot_pdf2psError (
			                  QProcess::ProcessError ) ) );
#ifndef Q_OS_WIN
			proc->start ( "pdf2ps",args );
#else
			QString pdf2ps,ver;
			PrintWidget::gsInfo ( ver,pdf2ps );
			QString wdir=pdf2ps;
			wdir.replace ( "pdf2ps.bat","" );
			proc->setWorkingDirectory ( wdir );
			QStringList env=QProcess::systemEnvironment();
			env.replaceInStrings ( QRegExp ( "^PATH=(.*)",
			                                 Qt::CaseInsensitive ),
			                       "PATH=\\1;"+wdir );
			wdir.replace ( "\\lib\\","\\bin\\" );
			env.replaceInStrings ( QRegExp ( "^PATH=(.*)",
			                                 Qt::CaseInsensitive ),
			                       "PATH=\\1;"+wdir );
			proc->setEnvironment ( env );
			proc->start ( pdf2ps,args );
#endif
		}
	}
}

void PrintProcess::slot_error ( QProcess::ProcessError )
{
	QString message=tr ( "Failed to execute command:\n" );
	if ( viewPdf )
		message+=" "+pdfOpenCmd+ " " +pdfFile;
	else
	{
		message+=printCmd;
		if ( !printStdIn )
		{
			message+=" ";
			if ( printPs )
				message+=psFile;
			else
				message+=pdfFile;
		}
	}
	QMessageBox::critical ( 0l, tr ( "Printing error." ),
	                        message );

}

void PrintProcess::slot_pdf2psError ( QProcess::ProcessError )
{
	QMessageBox::critical ( 0l, tr ( "Printing error." ),
	                        tr ( "Failed to execute command:\n" ) +
	                        "pdf2ps "+pdfFile+" "+psFile );
}
