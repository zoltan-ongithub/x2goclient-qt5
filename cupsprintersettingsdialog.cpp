/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
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

#include "cupsprintersettingsdialog.h"
#include "cupsprint.h"
#ifndef Q_OS_WIN
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>

CUPSPrinterSettingsDialog::CUPSPrinterSettingsDialog (
    QString prnName,
    CUPSPrint*  cupsObject,
    QWidget * parent,
    Qt::WFlags flags
) :QDialog ( parent, flags )
{
	m_cups=cupsObject;
	printer=prnName;
	ui.setupUi ( this );
	setWindowTitle ( prnName );
	QList<int> sz;
	sz<<250<<100;
	ui.splitter->setSizes ( sz );
	if ( !m_cups->setCurrentPrinter ( printer ) )
	{
		//message here
		close();
	}
	setGeneralTab();
	setPPDTab();
	connect ( ( QObject* ) ( ui.buttonBox->button (
	                             QDialogButtonBox::RestoreDefaults ) ),
	          SIGNAL ( clicked() ),this,SLOT ( slot_restoreDefaults() ) );
	connect ( ( QObject* ) ( ui.buttonBox->button (
	                             QDialogButtonBox::Save ) ),
	          SIGNAL ( clicked() ),this,SLOT ( slot_saveOptions() ) );
	connect ( ( QObject* ) ( ui.buttonBox->button (
	                             QDialogButtonBox::Cancel ) ),
	          SIGNAL ( clicked() ),this,SLOT ( reject() ) );
	connect ( ( QObject* ) ( ui.buttonBox->button (
	                             QDialogButtonBox::Ok ) ),
	          SIGNAL ( clicked() ),this,SLOT ( slot_ok() ) );
}



CUPSPrinterSettingsDialog::~CUPSPrinterSettingsDialog()
{
}



void CUPSPrinterSettingsDialog::setCbBox ( QComboBox* cb,
        QString optionKeyword )
{
	QStringList values;
	QStringList descriptions;
	int cur_val=m_cups->getOptionValues ( optionKeyword,
	                                      values,descriptions );
	if ( cur_val==-1 )
		cb->setEnabled ( false );
	else
	{
		cb->addItems ( descriptions );
		cb->setCurrentIndex ( cur_val );
	}
}

void CUPSPrinterSettingsDialog::slot_optionSelected ( QTreeWidgetItem * current,
        QTreeWidgetItem * )
{
	ui.optionsTree->clear();
	if ( current )
		if ( current->childCount() ==0 )
		{
			ui.gbOptions->setTitle ( current->text ( 0 ) );
			QStringList valueNames, valueTexts;
			int selectedValue=m_cups->
			                  getOptionValues (
			                      current->text ( 2 ),
			                      valueNames,valueTexts );
			for ( int i=0;i<valueNames.size();++i )
			{
				QTreeWidgetItem* ritem=new QTreeWidgetItem (
				    ui.optionsTree,
				    QTreeWidgetItem::Type ) ;
				ritem->setText ( 0,valueTexts[i] );
				ritem->setText ( 1,valueNames[i] );
				if ( i==selectedValue )
					ui.optionsTree->setCurrentItem (
					    ritem );
			}
			return;
		}
	ui.gbOptions->setTitle ( tr ( "No option selected" ) );
}

void CUPSPrinterSettingsDialog::slot_valueSelected ( QTreeWidgetItem * current,
        QTreeWidgetItem * )
{
	if ( !current )
		return;
	QTreeWidgetItem* optionItem=ui.ppdTree->currentItem();
	QString option=optionItem->text ( 2 );
	QString newVal=current->text ( 1 );
	QString prevVal,prevText;
	m_cups->getOptionValue ( option,prevVal,prevText );
	if ( prevVal==newVal )
		return;

	//we need change current item
	//do it outside this function
	setNewValue ( option,newVal ) ;
	QTimer::singleShot ( 1, this, SLOT ( slot_reloadValues() ) );

	m_cups->getOptionValue ( option,prevVal,prevText );
	optionItem->setText ( 1,prevText );
	optionItem->setText ( 3,prevVal );
	setGeneralTab();
}


void CUPSPrinterSettingsDialog::setGeneralTab()
{
	disconnect ( ui.cbPageSize,SIGNAL ( currentIndexChanged ( int ) ),
	             this,SLOT ( slot_changePSize ( int ) ) );
	disconnect ( ui.cbMediaType,SIGNAL ( currentIndexChanged ( int ) ),
	             this,SLOT ( slot_changePType ( int ) ) );
	disconnect ( ui.cbInputSlot,SIGNAL ( currentIndexChanged ( int ) ),
	             this,SLOT ( slot_changeISlot ( int ) ) );
	disconnect ( ui.rbNone,SIGNAL ( clicked ( ) ),
	             this,SLOT ( slot_changeDuplex() ) );
	disconnect ( ui.rbShort,SIGNAL ( clicked ( ) ),
	             this,SLOT ( slot_changeDuplex() ) );
	disconnect ( ui.rbLong,SIGNAL ( clicked ( ) ),
	             this,SLOT ( slot_changeDuplex() ) );
	ui.cbPageSize->clear();
	ui.cbMediaType->clear();
	ui.cbInputSlot->clear();
	setCbBox ( ui.cbPageSize,"PageSize" );
	setCbBox ( ui.cbMediaType,"MediaType" );
	setCbBox ( ui.cbInputSlot,"InputSlot" );
	QString valueName, valueText;
	ui.rbNone->setChecked ( true );
	if ( m_cups->getOptionValue ( "Duplex",valueName,valueText ) )
	{
		if ( valueName=="DuplexTumble" )
			ui.rbShort->setChecked ( true );
		if ( valueName=="DuplexNoTumble" )
			ui.rbLong->setChecked ( true );

	}
	else
		ui.gbDuplex->setEnabled ( false );
	connect ( ui.cbPageSize,SIGNAL ( currentIndexChanged ( int ) ),
	          this,SLOT ( slot_changePSize ( int ) ) );
	connect ( ui.cbMediaType,SIGNAL ( currentIndexChanged ( int ) ),
	          this,SLOT ( slot_changePType ( int ) ) );
	connect ( ui.cbInputSlot,SIGNAL ( currentIndexChanged ( int ) ),
	          this,SLOT ( slot_changeISlot ( int ) ) );
	connect ( ui.rbNone,SIGNAL ( clicked ( ) ),
	          this,SLOT ( slot_changeDuplex() ) );
	connect ( ui.rbShort,SIGNAL ( clicked ( ) ),
	          this,SLOT ( slot_changeDuplex() ) );
	connect ( ui.rbLong,SIGNAL ( clicked ( ) ),
	          this,SLOT ( slot_changeDuplex() ) );

}


void CUPSPrinterSettingsDialog::setPPDTab()
{
	disconnect ( ui.ppdTree,
	             SIGNAL ( currentItemChanged ( QTreeWidgetItem*,
	                                           QTreeWidgetItem* ) ),
	             this,
	             SLOT ( slot_optionSelected ( QTreeWidgetItem*,
	                                          QTreeWidgetItem* ) ) );

	disconnect ( ui.optionsTree,
	             SIGNAL ( currentItemChanged ( QTreeWidgetItem*,
	                                           QTreeWidgetItem* ) ),
	             this,
	             SLOT ( slot_valueSelected ( QTreeWidgetItem*,
	                                         QTreeWidgetItem* ) ) );

	QString info;
	bool acceptJobs;
	QString location;
	QString model;
	CUPSPrint::printState state;
	QString stateReason;
	QString valueName,valueText;
	!m_cups->getPrinterInfo ( printer,
	                          info,acceptJobs,
	                          location,model,state,stateReason ) ;


	ui.ppdTree->clear();
	QTreeWidgetItem* ritem=new QTreeWidgetItem ( ( QTreeWidgetItem* ) 0,
	        QTreeWidgetItem::Type ) ;

	ritem->setText ( 0,model );
	ui.ppdTree->addTopLevelItem ( ritem );
	QStringList grName, grText;
	m_cups->getOptionGroups ( grName,grText );
	for ( int i=0;i<grName.size();++i )
	{
		QTreeWidgetItem* gritem=new QTreeWidgetItem ( ritem,
		        QTreeWidgetItem::Type ) ;
		gritem->setText ( 0,grText[i] );
		gritem->setText ( 2,grName[i] );
		QStringList optName, optText;
		m_cups->getOptionsList ( grName[i],optName,optText );
		for ( int j=0;j<optName.size();++j )
		{
			QTreeWidgetItem* optitem=new QTreeWidgetItem ( gritem,
			        QTreeWidgetItem::Type ) ;
			optitem->setText ( 0,optText[j] );
			optitem->setText ( 2,optName[j] );
			m_cups->getOptionValue ( optName[j],valueName,
			                         valueText );
			optitem->setText ( 1,valueText );
			optitem->setText ( 3,valueName );
		}

	}
	ui.ppdTree->expandAll();
	ui.ppdTree->header()->resizeSections ( QHeaderView::ResizeToContents );
	slot_optionSelected ( ritem,0l );
	connect ( ui.ppdTree,
	          SIGNAL ( currentItemChanged ( QTreeWidgetItem*,
	                                        QTreeWidgetItem* ) ),
	          this,
	          SLOT ( slot_optionSelected ( QTreeWidgetItem*,
	                                       QTreeWidgetItem* ) ) );

	connect ( ui.optionsTree,
	          SIGNAL ( currentItemChanged ( QTreeWidgetItem*,
	                                        QTreeWidgetItem* ) ),
	          this,
	          SLOT ( slot_valueSelected ( QTreeWidgetItem*,
	                                      QTreeWidgetItem* ) ) );

}


bool CUPSPrinterSettingsDialog::setNewValue ( const QString& option,
        const QString& value )
{
	QString confVal,confOpt;
	bool res=m_cups->setValue ( option,value,confOpt,confVal );
	if ( !res )
	{
		QString textMessage=
		    tr ( "This value is in conflict with other option" );
		QString txt;
		m_cups->getOptionText ( confOpt,txt );
		QString val,valt;
		m_cups->getOptionValue ( confOpt,val,valt );
		if ( confOpt.length() >0 &&confVal.length() >0 )
		{
			textMessage+="\n("+txt+" : "+valt+")";

		}
		QMessageBox::critical ( this,tr ( "Options conflict" ),
		                        textMessage );
	}
	return res;
}


void CUPSPrinterSettingsDialog::slot_reloadValues()
{
	if ( ui.ppdTree->currentItem() )
		slot_optionSelected ( ui.ppdTree->currentItem(),0l );
	QTreeWidgetItemIterator it ( ui.ppdTree );
	while ( *it )
	{
		if ( ( *it )->childCount() ==0 )
		{
			QString opt= ( *it )->text ( 2 );
			QString nval,ntext;
			m_cups->getOptionValue ( opt,nval,ntext );
			if ( ( *it )->text ( 3 ) != nval )
				( *it )->setText ( 1,ntext );
			( *it )->setText ( 3,nval );
		}
		++it;
	}
}


void CUPSPrinterSettingsDialog::slot_changePSize ( int ind )
{
	changeFromCbBox ( "PageSize",ind );

}

void CUPSPrinterSettingsDialog::slot_changePType ( int ind )
{
	changeFromCbBox ( "MediaType",ind );
}

void CUPSPrinterSettingsDialog::slot_changeISlot ( int ind )
{
	changeFromCbBox ( "InputSlot",ind );
}

void CUPSPrinterSettingsDialog::changeFromCbBox ( const QString& opt, int id )
{
	QStringList vals,texts;
	m_cups->getOptionValues ( opt,vals,texts );
	if ( vals.size() <id )
		return;
	changeGeneralOption ( opt,vals[id] );
}

void CUPSPrinterSettingsDialog::slot_changeDuplex()
{
	if ( ui.rbShort->isChecked() )
	{
		changeGeneralOption ( "Duplex","DuplexTumble" );
	}
	if ( ui.rbLong->isChecked() )
	{
		changeGeneralOption ( "Duplex","DuplexNoTumble" );
	}
	if ( ui.rbNone->isChecked() )
	{
		changeGeneralOption ( "Duplex","None" );
	}
}

void CUPSPrinterSettingsDialog::changeGeneralOption ( const QString&  option,
        const QString& val )
{
	if ( !setNewValue ( option,val ) )
		QTimer::singleShot ( 1, this, SLOT ( setGeneralTab() ) );
	slot_reloadValues();
}

void CUPSPrinterSettingsDialog::slot_restoreDefaults()
{
	m_cups->setDefaults();
	setGeneralTab();
	slot_reloadValues();
}

void CUPSPrinterSettingsDialog::slot_saveOptions()
{
	m_cups->saveOptions();
}

void CUPSPrinterSettingsDialog::slot_ok()
{
	m_cups->saveOptions();
	accept();
}

#endif
