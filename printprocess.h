/**************************************************************************
*   Copyright (C) 2005-2014 by Oleksandr Shneyder                         *
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

#ifndef PRINTPROCESS_H
#define PRINTPROCESS_H

#include <QString>
#include <QObject>
#include <QProcess>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class PrintProcess: public QObject
{
		Q_OBJECT

	public:
		PrintProcess ( QString pdf, QString title,
		               QObject * parent=0l );
		~PrintProcess();
	private:
		QString pdfFile;
		QString pdfTitle;
		QString psFile;
		bool viewPdf;
		bool customPrintCmd;
		bool printStdIn;
		bool printPs;
		bool pdfOpen;
		QString pdfOpenCmd;
		QWidget* parentWidg;
		QString printCmd;
#ifdef Q_OS_WIN
		QString winX2goPrinter;
		QString winDefaultPrinter;
#endif
	private:
		bool loadSettings();
	private slots:
		void slot_processFinished ( int exitCode,
		                            QProcess::ExitStatus exitStatus );
		void slot_pdf2psError ( QProcess::ProcessError error ) ;
		void slot_error ( QProcess::ProcessError error );
		void openPdf();
		void print();
};

#endif
