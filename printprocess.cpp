//
// C++ Implementation: printprocess
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2goclientconfig.h"
#include "printprocess.h"
#include "x2gologdebug.h"
#include <QFile>
#include <QSettings>
#include <QDir>
#include "printdialog.h"
#ifndef WINDOWS
  #include "cupsprint.h"
#else
#include "printwidget.h"
#endif
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
PrintProcess::PrintProcess ( QString pdf, QString title, QObject *parent ) :
		QObject ( parent )
{
	pdfFile=pdf;
	pdfTitle=title;
	if ( !loadSettings() )
	{
		QFile::remove ( pdfFile );
		return;
	}
	if ( viewPdf )
		openPdf();
	else
		print();
}


PrintProcess::~PrintProcess()
{
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
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/printing",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "printing" );
#endif

	if ( st.value ( "showdialog",true ).toBool() )
	{
		PrintDialog dlg;
		if ( dlg.exec() ==QDialog::QDialog::Rejected )
			return false;
	}

	viewPdf=st.value ( "pdfview",false ).toBool();
	customPrintCmd=st.value ( "print/startcmd",false ).toBool();
	printCmd=st.value ( "print/command","lpr" ).toString();
	printStdIn= st.value ( "print/stdin",false ).toBool();
	printPs=st.value ( "print/ps",false ).toBool();

	pdfOpen= st.value ( "view/open",true ).toBool();

#ifndef WINDOWS
	pdfOpenCmd=st.value ( "view/command","xpdf" ).toString();
#endif
	return true;

}


void PrintProcess::openPdf()
{
	if ( pdfOpen )
	{
#ifndef WINDOWS
		if ( ! QProcess::startDetached ( pdfOpenCmd+" \""+pdfFile+
		                                 "\"" ) )
#else
		pdfOpenCmd=PrintWidget::getPdfCmd();
		pdfOpenCmd.replace ( "%1",pdfFile );

		if ( ! QProcess::startDetached ( pdfOpenCmd) )
#endif
			slot_error ( QProcess::FailedToStart );
	}
	else
	{
		QString fileName = QFileDialog::getSaveFileName ( 0l,
		                   tr ( "Save File" ),
		                   QDir::homePath() +"/"+pdfTitle+".pdf",
		                   tr ( "PDF Document (*.pdf)" ) );
		if ( fileName.length() >0 )
			QFile::rename ( pdfFile,fileName );

	}
}

void PrintProcess::print()
{
#ifndef WINDOWS
	if ( !customPrintCmd )
	{
		CUPSPrint prn;
		prn.setCurrentPrinter ( prn.getDefaultUserPrinter() );
		prn.print ( pdfFile, pdfTitle );
	}
	else
#endif //WINDOWS
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
#ifndef WINDOWS			
			proc->start ( "pdf2ps",args );
#else
			QString pdf2ps,ver;
			PrintWidget::gsInfo(ver,pdf2ps);
			QString wdir=pdf2ps;
			wdir.replace("pdf2ps.bat","");
			proc->setWorkingDirectory(wdir);
			QStringList env=QProcess::systemEnvironment();
			env.replaceInStrings(QRegExp("^PATH=(.*)", 
					     Qt::CaseInsensitive), "PATH=\\1;"+wdir);
			wdir.replace("\\lib\\","\\bin\\");
			env.replaceInStrings(QRegExp("^PATH=(.*)", 
					     Qt::CaseInsensitive), "PATH=\\1;"+wdir);
			proc->setEnvironment(env);
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
	QMessageBox::critical ( 0l, tr ( "Printing error" ),
	                        message );

}

void PrintProcess::slot_pdf2psError ( QProcess::ProcessError )
{
	QMessageBox::critical ( 0l, tr ( "Printing error" ),
	                        tr ( "Failed to execute command:\n" ) +
	                        "pdf2ps "+pdfFile+" "+psFile );
}

