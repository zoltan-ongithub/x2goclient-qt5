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

#ifndef CUPSPRINT_H
#define CUPSPRINT_H
#include "x2goclientconfig.h"
#ifndef Q_OS_WIN
#include <QStringList>
#include <cups/cups.h>
#include <cups/ppd.h>
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class CUPSPrint
{
	public:
		enum printState {NDEF,IDLE,PRINTING,STOPPED};

		CUPSPrint();
		~CUPSPrint();
		void setDefaultUserPrinter ( QString printer );
		QString getDefaultUserPrinter();
		QStringList getPrinters();
		bool getPrinterInfo ( const QString& printerName,
		                      QString& info, bool& acceptJobs,
		                      QString& location, QString& model,
		                      printState& state, QString& stateReason );
		bool setCurrentPrinter ( QString );
		bool getOptionValue ( const QString& option,
		                      QString& value, QString& valueText );
		int getOptionValues ( const QString& option,
		                      QStringList& values,
		                      QStringList& descriptions );
		int getOptionGroups ( QStringList& names, QStringList& texts );
		int getOptionsList ( const QString& group, QStringList& names,
		                     QStringList& texts );
		bool setValue ( const QString& option, const QString& value,
		                QString& conflict_opt, QString& conflict_val );
		bool getOptionText ( const QString& option, QString& text );
		void setDefaults();
		void saveOptions();
		void print ( const QString& file, QString title="" );

	private:
		cups_dest_t *dests;
		int num_dests;
		ppd_file_t *ppd;
		QString currentPrinter;
	private:
		void loadUserOptions();
};

#endif
#endif
