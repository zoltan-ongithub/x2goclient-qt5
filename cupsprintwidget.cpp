//
// C++ Implementation: cupsprintwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cupsprint.h"
#include "cupsprintwidget.h"
#ifndef Q_OS_WIN
#include "x2gologdebug.h"
#include "cupsprintersettingsdialog.h"

CUPSPrintWidget::CUPSPrintWidget ( QWidget* parent )
		: QWidget ( parent )
{
	m_cups=new CUPSPrint;
	ui.setupUi ( this );

	ui.cbPrinters->addItems ( m_cups->getPrinters() );
	int defind=ui.cbPrinters->findText ( m_cups->getDefaultUserPrinter() );
	if ( defind!=-1 )
	{
		ui.cbPrinters->setCurrentIndex ( defind );
		slot_printerSelected ( defind );
	}
	connect ( ui.cbPrinters,
	          SIGNAL ( currentIndexChanged ( int ) ),
	          this,SLOT ( slot_printerSelected ( int ) ) ) ;

	connect ( ui.pbProps,
	          SIGNAL ( clicked() ),
	          this, SLOT ( slot_printerSettings() ) );
}


CUPSPrintWidget::~CUPSPrintWidget()
{
	delete m_cups;
}


void CUPSPrintWidget::slot_printerSelected ( int index )
{
	if ( index == -1 )
		return;
	QString info;
	bool acceptJobs;
	QString location;
	QString model;
	CUPSPrint::printState state;
	QString stateReason;
	if ( !m_cups->getPrinterInfo ( ui.cbPrinters->currentText(),
	                               info,acceptJobs,
	                               location,model,state,stateReason ) )
		return;
	QString stText;
	switch ( state )
	{
		case CUPSPrint::IDLE:
			stText=tr ( "Idle" );
			break;
		case CUPSPrint::PRINTING:
			stText=tr ( "Printing" );
			break;
		case CUPSPrint::STOPPED:
			stText=tr ( "Stopped" );
			break;
		default:
			break;
	}
	if ( stateReason.length() >0 && stateReason != "none" )
		stText+= " ("+stateReason+")";
	ui.lState->setText ( stText );

	( acceptJobs ) ?ui.lJAccept->setText (
	    tr ( "Yes" ) ) :ui.lJAccept->setText ( tr ( "No" ) );

	ui.lType->setText ( info );
	ui.lLocation->setText ( location );
	ui.lComment->setText ( model );
}


void CUPSPrintWidget::slot_printerSettings()
{
	CUPSPrinterSettingsDialog dlg ( ui.cbPrinters->currentText(),
	                                m_cups,this );
	dlg.exec();
}

void CUPSPrintWidget::savePrinter()
{
	m_cups->setDefaultUserPrinter ( ui.cbPrinters->currentText() );
}
#endif
