//
// C++ Interface: cupsprintersettingsdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CUPSPRINTERSETTINGSDIALOG_H
#define CUPSPRINTERSETTINGSDIALOG_H

#include <QDialog>
#include "ui_cupsprintsettingsdialog.h"

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class CUPSPrint;
class CUPSPrinterSettingsDialog : public QDialog
{
		Q_OBJECT
	public:
		enum tabs{GENERAL,PPD};
		CUPSPrinterSettingsDialog ( QString prnName,
		                            CUPSPrint*  cupsObject,
		                            QWidget * parent=0l,
		                            Qt::WFlags flags =0 );
		~CUPSPrinterSettingsDialog();
	private:
		CUPSPrint* m_cups;
		Ui::CupsPrinterSettingsDialog ui;
		QString printer;
	private:
		void setCbBox ( QComboBox* cb, QString optionKeyword );
		void setPPDTab();
		bool setNewValue ( const QString& option,
		                   const QString& value );
		void changeFromCbBox ( const QString& opt, int id );
		void changeGeneralOption ( const QString&  option,
		                           const QString& val );
	private slots:
		void slot_optionSelected ( QTreeWidgetItem * current, 
					   QTreeWidgetItem * previous );
		void slot_valueSelected ( QTreeWidgetItem * current, 
					  QTreeWidgetItem * previous );
		void slot_reloadValues();
		void slot_changePSize ( int ind );
		void slot_changePType ( int ind );
		void slot_changeISlot ( int ind );
		void slot_changeDuplex();
		void setGeneralTab();
		void slot_restoreDefaults();
		void slot_saveOptions();
		void slot_ok();

};

#endif
