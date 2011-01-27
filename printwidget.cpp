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
#if (!defined WINDOWS) && (!defined Q_WS_HILDON)
#include "cupsprintwidget.h"
#endif
#include "printercmddialog.h"
#include <QSettings>
#include "x2gologdebug.h"
#include <QDir>
PrintWidget::PrintWidget ( QWidget* parent )
		: QWidget ( parent )
{
	ui.setupUi ( this );
	ui.rbPrint->setChecked ( true );
	ui.gbView->setVisible ( false );
	QVBoxLayout* lay= ( QVBoxLayout* ) ui.gbPrint->layout();
#if (!defined WINDOWS) && (!defined Q_WS_HILDON)
	pwid=new CUPSPrintWidget ( ui.gbPrint );
	lay->insertWidget ( 0,pwid );
	connect ( ui.cbPrintCmd,SIGNAL ( toggled ( bool ) ),pwid,
	          SLOT ( setDisabled ( bool ) ) );
#else
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
	lay->insertWidget ( 3,rtfm );
#endif
	connect ( ui.pbPrintCmd,SIGNAL ( clicked() ),this,
	          SLOT ( slot_editPrintCmd() ) );

	QButtonGroup* bg=new QButtonGroup();
	bg->addButton ( ui.rbPrint );
	bg->addButton ( ui.rbView );
	loadSettings();
	connect ( ui.cbShowDialog,SIGNAL ( toggled ( bool ) ),
	          this, SIGNAL ( dialogShowEnabled ( bool ) ) );
#if (defined WINDOWS)
	ui.line->hide();
	ui.cbPrintCmd->setChecked ( true );
	ui.cbPrintCmd->setEnabled ( false );
	ui.label->hide();
	ui.leOpenCmd->hide();
#endif
#ifdef Q_WS_HILDON
	ui.rbView->setChecked(true);
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
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/printing",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "printing" );
#endif
	bool pdfView=st.value ( "pdfview",false ).toBool();
	QString prcmd=
	    st.value ( "print/command","" ).toString();
#ifdef WINDOWS
	QString ver,gspath,gsvpath;
	bool isGsInstalled=gsInfo ( ver,gspath );
	bool isGsViewInstalled=gsViewInfo ( ver,gsvpath );
	if ( prcmd=="" && ! ( isGsInstalled && isGsViewInstalled ) )
	{
		x2goDebug<<"fallback to view"<<endl;
		pdfView=true;
	}
	else if ( prcmd=="" )
	{
		prcmd=gsvpath+" -query -color";
	}
#endif
	ui.cbShowDialog->setChecked (
	    st.value ( "showdialog",true ).toBool() );

	if ( pdfView )
		ui.rbView->setChecked ( true );
	else
		ui.rbPrint->setChecked ( true );

#ifndef WINDOWS
	ui.cbPrintCmd->setChecked ( st.value ( "print/startcmd",
	                                       false ).toBool() );
	if ( prcmd=="" )
		prcmd="lpr";
#endif
	ui.lePrintCmd->setText ( prcmd );

	printStdIn= st.value ( "print/stdin",false ).toBool();
	printPs=st.value ( "print/ps",false ).toBool();

#ifdef WINDOWS
	printPs=printPs&&isGsInstalled;
#endif

	if ( ( st.value ( "view/open",true ).toBool() )
#ifdef WINDOWS
	        && ( getPdfCmd().length() >0 )
#endif
	   )
		ui.rbOpen->setChecked ( true );
	else
		ui.rbSave->setChecked ( true );
	ui.leOpenCmd->setText (
	    st.value ( "view/command","xpdf" ).toString() );
}

void PrintWidget::saveSettings()
{
#ifndef WINDOWS
	QSettings st ( QDir::homePath() +"/.x2goclient/printing",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "printing" );
#endif

	st.setValue ( "showdialog",
	              QVariant ( ui.cbShowDialog->isChecked () ) );
	st.setValue ( "pdfview",
	              QVariant ( ui.rbView->isChecked () ) );
	st.setValue ( "print/startcmd",
	              QVariant ( ui.cbPrintCmd->isChecked ( ) ) );
	st.setValue ( "print/command",
	              QVariant ( ui.lePrintCmd->text () ) );
	st.setValue ( "print/stdin",
	              QVariant ( printStdIn ) );
	st.setValue ( "print/ps",
	              QVariant ( printPs ) );

	st.setValue ( "view/open",
	              QVariant ( ui.rbOpen->isChecked () ) );
	st.setValue ( "view/command",
	              QVariant ( ui.leOpenCmd->text () ) );
#if (!defined WINDOWS) && (!defined Q_WS_HILDON)
	pwid->savePrinter();
#endif
}


#ifdef WINDOWS
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

QString PrintWidget::getAppCmd ( const QString& app )
{
	QSettings st ( "HKEY_CLASSES_ROOT\\Applications\\"+
	               app+"\\shell\\open\\command",
	               QSettings::NativeFormat );
	QString cmd=st.value ( ".","" ).toString();
	if ( cmd.length() >0 )
		return  cmd;
	else
	{
	QSettings st ( "HKEY_CLASSES_ROOT\\Applications\\"+
	               app+"\\shell\\Read\\command",
	               QSettings::NativeFormat );
	return st.value ( ".","" ).toString();
	}

}

QString PrintWidget::getPdfCmd()
{
	QSettings st ( "HKEY_CURRENT_USER\\"
	               "Software\\Microsoft\\"
	               "Windows\\CurrentVersion\\"
	               "Explorer\\FileExts\\.pdf",
	               QSettings::NativeFormat );
	QString defApp=st.value ( "Application",
	                          "" ).toString();
	QString appA=st.value ( "OpenWithList/a",
	                        "" ).toString();
	QString appB=st.value ( "OpenWithList/b",
	                        "" ).toString();
	QString appC=st.value ( "OpenWithList/c",
	                        "" ).toString();
	QString cmd;
	cmd=getAppCmd ( defApp );
	if ( cmd.length() >0 )
		return cmd;
	cmd=getAppCmd ( appA );
	if ( cmd.length() >0 )
		return cmd;
	cmd=getAppCmd ( appB );
	if ( cmd.length() >0 )
		return cmd;
	return getAppCmd ( appC );
}
#endif //WINDOWS
