//
// C++ Interface: cupsprint
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CUPSPRINT_H
#define CUPSPRINT_H
#include <QStringList>
#include <cups/cups.h>
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
		ppd_file_t* ppd;
		QString currentPrinter;
	private:
		void loadUserOptions();
};

#endif
