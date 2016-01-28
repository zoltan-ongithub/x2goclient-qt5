/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
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

#ifndef CUPSPRINTERSETTINGSDIALOG_H
#define CUPSPRINTERSETTINGSDIALOG_H
#include "x2goclientconfig.h"
#ifndef Q_OS_WIN
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
#endif
