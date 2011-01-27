//
// C++ Implementation: printwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "printwidget.h"
#if (!defined Q_OS_WIN) && (!defined Q_WS_HILDON)
#include "cupsprintwidget.h"
#endif
#include "printercmddialog.h"
#include "x2gosettings.h"
#include "x2gologdebug.h"
#include <QDir>
#ifdef Q_OS_WIN
#include "wapi.h"
#endif
PrintWidget::PrintWidget ( QWidget* parent )
		: QWidget ( parent )
{
	ui.setupUi ( this );
	ui.rbPrint->setChecked ( true );
	ui.gbView->setVisible ( false );
	QVBoxLayout* lay= ( QVBoxLayout* ) ui.gbPrint->layout();
#if (!defined Q_OS_WIN) && (!defined Q_WS_HILDON)
	ui.cbWinPrinter->hide();
	ui.lWinPrinter->hide();
	ui.lWinInfo->hide();
	pwid=new CUPSPrintWidget ( ui.gbPrint );
	lay->insertWidget ( 0,pwid );
	connect ( ui.cbPrintCmd,SIGNAL ( toggled ( bool ) ),pwid,
	          SLOT ( setDisabled ( bool ) ) );
#else
#ifdef Q_OS_WIN
	connect ( ui.cbPrintCmd,SIGNAL ( toggled ( bool ) ),ui.cbWinPrinter,
	          SLOT ( setDisabled ( bool ) ) );
	connect ( ui.cbPrintCmd,SIGNAL ( toggled ( bool ) ),ui.lWinPrinter,
	          SLOT ( setDisabled ( bool ) ) );
	connect ( ui.cbPrintCmd,SIGNAL ( toggled ( bool ) ),ui.lWinInfo,
	          SLOT ( setDisabled ( bool ) ) );
	printers=wapiGetLocalPrinters();
	defaultPrinter=wapiGetDefaultPrinter();
	ui.cbWinPrinter->insertItems ( 0,printers );
	int index=printers.indexOf ( defaultPrinter );
	if ( index!=-1 )
		ui.cbWinPrinter->setCurrentIndex ( index );
	QLabel *rtfm=new QLabel (
	    tr (
	        "Please configure your client side printing settings.<br><br>"
	        "If you want to print the created file, you'll need "
	        "an external application. Typically you can use "
	        "<a href=\"http://pages.cs.wisc.edu/~ghost/doc/GPL/index.htm\">"
	        "ghostprint</a> and "
	        "<a href=\"http://pages.cs.wisc.edu/~ghost/gsview/\">"
	        "ghostview</a><br>You can find  further information "
	        "<a href=\"http://www.x2go.org/index.php?id=49\">here</a>." ),
	    ui.gbPrint );
	rtfm->setWordWrap ( true );
	rtfm->setOpenExternalLinks ( true );
	lay->insertWidget ( 6,rtfm );
#endif
#endif
	connect ( ui.pbPrintCmd,SIGNAL ( clicked() ),this,
	          SLOT ( slot_editPrintCmd() ) );

	QButtonGroup* bg=new QButtonGroup();
	bg->addButton ( ui.rbPrint );
	bg->addButton ( ui.rbView );
	loadSettings();
	connect ( ui.cbShowDialog,SIGNAL ( toggled ( bool ) ),
	          this, SIGNAL ( dialogShowEnabled ( bool ) ) );
#if (defined Q_OS_WIN)
	ui.label->hide();
	ui.leOpenCmd->hide();
#endif
#ifdef Q_WS_HILDON
	ui.rbView->setChecked ( true );
	ui.rbPrint->hide();
	ui.rbView->hide();
	ui.label->hide();
	ui.leOpenCmd->hide();
#endif
}


PrintWidget::~PrintWidget()
{
}


void PrintWidget::slot_editPrintCmd()
{
	QString printCmd=ui.lePrintCmd->text();
	PrinterCmdDialog dlg ( &printCmd,&printStdIn,&printPs, this );
	dlg.exec();
	ui.lePrintCmd->setText ( printCmd );
}

void PrintWidget::loadSettings()
{
	X2goSettings st ( "printing" );
	bool pdfView=st.setting()->value ( "pdfview",false ).toBool();
	QString prcmd=
	    st.setting()->value ( "print/command","" ).toString();
#ifdef Q_OS_WIN
	defaultPrinter=
	    st.setting()->value ( "print/defaultprinter",defaultPrinter ).toString();
	
	int index=printers.indexOf ( defaultPrinter );
	if ( index!=-1 )
		ui.cbWinPrinter->setCurrentIndex ( index );

	QString ver,gspath,gsvpath;
	bool isGsInstalled=gsInfo ( ver,gspath );
	bool isGsViewInstalled=gsViewInfo ( ver,gsvpath );
	if ( prcmd=="" && ! ( isGsInstalled && isGsViewInstalled ) )
	{
// 		x2goDebug<<"fallback to view"<<endl;
// 		pdfView=true;
	}
	else if ( prcmd=="" )
	{
		prcmd=gsvpath+" -query -color";
	}
#endif
	ui.cbShowDialog->setChecked (
	    st.setting()->value ( "showdialog",true ).toBool() );

	if ( pdfView )
		ui.rbView->setChecked ( true );
	else
		ui.rbPrint->setChecked ( true );

	ui.cbPrintCmd->setChecked ( st.setting()->value ( "print/startcmd",
	                                       false ).toBool() );
#ifndef Q_OS_WIN
	if ( prcmd=="" )
		prcmd="lpr";
#endif
	ui.lePrintCmd->setText ( prcmd );

	printStdIn= st.setting()->value ( "print/stdin",false ).toBool();
	printPs=st.setting()->value ( "print/ps",false ).toBool();

#ifdef Q_OS_WIN
	printPs=printPs&&isGsInstalled;
#endif

	if ( ( st.setting()->value ( "view/open",true ).toBool() ) )
		ui.rbOpen->setChecked ( true );
	else
		ui.rbSave->setChecked ( true );
	ui.leOpenCmd->setText (
	    st.setting()->value ( "view/command","xpdf" ).toString() );
}

void PrintWidget::saveSettings()
{
	X2goSettings st ( "printing" );

	st.setting()->setValue ( "showdialog",
	              QVariant ( ui.cbShowDialog->isChecked () ) );
	st.setting()->setValue ( "pdfview",
	              QVariant ( ui.rbView->isChecked () ) );
	st.setting()->setValue ( "print/startcmd",
	              QVariant ( ui.cbPrintCmd->isChecked ( ) ) );
	st.setting()->setValue ( "print/command",
	              QVariant ( ui.lePrintCmd->text () ) );
	st.setting()->setValue ( "print/stdin",
	              QVariant ( printStdIn ) );
	st.setting()->setValue ( "print/ps",
	              QVariant ( printPs ) );

	st.setting()->setValue ( "view/open",
	              QVariant ( ui.rbOpen->isChecked () ) );
	st.setting()->setValue ( "view/command",
	              QVariant ( ui.leOpenCmd->text () ) );
#ifdef Q_OS_WIN
	st.setting()->setValue ( "print/defaultprinter",
	              QVariant ( ui.cbWinPrinter->currentText()) );	
#endif
#if (!defined Q_OS_WIN) && (!defined Q_WS_HILDON)
	pwid->savePrinter();
#endif
}


#ifdef Q_OS_WIN
bool PrintWidget::gsInfo ( QString& version, QString& pdf2ps )
{
	QSettings st ( "HKEY_LOCAL_MACHINE\\"
	               "SOFTWARE\\GPL Ghostscript",
	               QSettings::NativeFormat );
	version="0.0";
	QStringList keys=st.allKeys();
	for ( int i=0;i<keys.size();++i )
	{
		if ( keys[i].indexOf ( "GS_LIB" ) !=-1 )
		{
			QString v=keys[i].split ( "/" ) [0];
			QString libs=st.value ( keys[i],"" ).toString();
			libs=libs.split ( ";" ) [0];
			if ( QFile::exists ( libs+"\\pdf2ps.bat" ) &&
			        v.toFloat() >version.toFloat() )
			{
				version=v;
				pdf2ps=libs+"\\pdf2ps.bat";
			}

		}
	}
	if ( version.toFloat() >0.0 )
	{
		return true;
	}
	return false;
}

bool PrintWidget::gsViewInfo ( QString& version, QString& gsprint )
{
	QSettings st ( "HKEY_LOCAL_MACHINE\\"
	               "SOFTWARE\\Ghostgum\\GSview",
	               QSettings::NativeFormat );
	version="0.0";
	QStringList keys=st.allKeys();
	for ( int i=0;i<keys.size();++i )
	{
		QString v=keys[i];
		QString libs=st.value ( keys[i],"" ).toString();
		if ( QFile::exists ( libs+"\\gsview\\gsprint.exe" ) &&
		        v.toFloat() >version.toFloat() )
		{
			version=v;
			gsprint=libs+"\\gsview\\gsprint.exe";
		}


	}
	if ( version.toFloat() >0.0 )
	{
		return true;
	}
	return false;
}
#endif //Q_OS_WIN
