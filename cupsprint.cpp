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

#include "cupsprint.h"
#ifndef Q_OS_WIN
#include "x2gologdebug.h"
#include "x2gosettings.h"
#include <QDir>
CUPSPrint::CUPSPrint()
{
	ppd=0l;
	num_dests= cupsGetDests ( &dests );
}


CUPSPrint::~CUPSPrint()
{
	cupsFreeDests ( num_dests, dests );
	if ( ppd )
		ppdClose ( ppd );
	ppd=0l;
}


QStringList CUPSPrint::getPrinters()
{
	QStringList printers;

	for ( int i=0;i<num_dests;++i )
		printers<<dests[i].name;
	return printers;
}

QString CUPSPrint::getDefaultUserPrinter()
{
	X2goSettings st ( "printing" );

	QString defPrint=st.setting()->value (
	                     "CUPS/defaultprinter","" ). toString();
	if ( defPrint.length() >0 )
	{
		cups_dest_t *dest = cupsGetDest ( defPrint.toAscii(),
		                                  0l, num_dests, dests );
		if ( dest )
			return defPrint;
	}

	cups_dest_t *dest = cupsGetDest ( 0l, 0l, num_dests, dests );
	if ( dest )
		defPrint=dest->name;

	return defPrint;
}

void CUPSPrint::setDefaultUserPrinter ( QString printer )
{
	X2goSettings st ( "printing" );
	st.setting()->setValue ( "CUPS/defaultprinter", QVariant ( printer ) );
}

bool CUPSPrint::getPrinterInfo ( const QString& printerName, QString& info,
                                 bool& acceptJobs, QString& location,
                                 QString& model, printState& state,
                                 QString& stateReason )
{
	cups_dest_t *dest = cupsGetDest ( printerName.toAscii(), 0l,
	                                  num_dests,
	                                  dests );
	if ( !dest )
		return false;
	acceptJobs=qstrcmp ( cupsGetOption ( "printer-is-accepting-jobs",
	                                     dest->num_options,
	                                     dest->options ),"0" );

	info=QString::fromLocal8Bit (
	         cupsGetOption ( "printer-info",
	                         dest->num_options, dest->options ) );

	location=QString::fromLocal8Bit (
	             cupsGetOption ( "printer-location",
	                             dest->num_options, dest->options ) );
	model=QString::fromLocal8Bit (
	          cupsGetOption ( "printer-make-and-model",
	                          dest->num_options, dest->options ) );
	QString st=cupsGetOption ( "printer-state",
	                           dest->num_options, dest->options );
	state=NDEF;
	if ( st=="3" )
		state=IDLE;
	if ( st=="4" )
		state=PRINTING;
	if ( st=="5" )
		state=STOPPED;
	stateReason=QString::fromLocal8Bit (
	                cupsGetOption ( "printer-state-reasons",
	                                dest->num_options, dest->options ) );
	return true;
}

bool CUPSPrint::setCurrentPrinter ( QString prn )
{
	currentPrinter=prn;
	QString fl=cupsGetPPD ( prn.toAscii() );
	if ( fl.length() <=0 )
		return false;

	if ( ppd )
		ppdClose ( ppd );
	ppd=0l;
	ppd=ppdOpenFile ( fl.toAscii() );
	unlink ( fl.toAscii() );
	if ( ppd==0l )
		return false;
	ppdMarkDefaults ( ppd );
	loadUserOptions();
	if ( ppdConflicts ( ppd ) !=0 )
	{
		x2goDebug<<"There are conflicting options in user settings,\n"
		"loading defaults"<<endl;
		setDefaults();
	}
	return true;
}

bool CUPSPrint::getOptionValue ( const QString& option,
                                 QString& value, QString& valueText )
{
	if ( !ppd )
		return false;
	ppd_choice_t* choice=ppdFindMarkedChoice ( ppd,option.toAscii() );
	if ( !choice )
	{
		ppd_option_t* opt=ppdFindOption ( ppd,option.toAscii() );
		if ( !opt )
			return false;
		choice=ppdFindChoice ( opt,opt->defchoice );
		if ( !choice )
			return false;
	}
	value=QString::fromLocal8Bit ( choice->choice );
	valueText=QString::fromLocal8Bit ( choice->text );
// 	x2goDebug<<"getValue:"<<value<<endl;
	return true;
}

int CUPSPrint::getOptionValues ( const QString& option,
                                 QStringList& values,
                                 QStringList& descriptions )
{
	values.clear();
	descriptions.clear();
	if ( !ppd )
		return -1;
	int cur_val=-1;
	values.clear();
	descriptions.clear();
	ppd_option_t* opt=ppdFindOption ( ppd,option.toAscii() );
	if ( !opt )
		return -1;
	for ( int k=0;k<opt->num_choices;++k )
	{
		ppd_choice_t* choice=& ( opt->choices[k] );
		if ( choice->marked )
		{
			cur_val=values.size();
		}
		//if no choice is marked, return default
		if ( !qstrcmp ( choice->choice,opt->defchoice ) && cur_val==-1 )
		{
			cur_val=values.size();
		}
		values<<QString::fromLocal8Bit ( choice->choice );
		descriptions<<QString::fromLocal8Bit ( choice->text );
	}
	return cur_val;

}


int CUPSPrint::getOptionGroups ( QStringList& names, QStringList& texts )
{
	names.clear();
	texts.clear();
	if ( !ppd )
		return -1;

	for ( int i=0;i<ppd->num_groups;++i )
	{
		ppd_group_t* group=& ( ppd->groups[i] );
		names<<QString::fromLocal8Bit ( group->name );
		texts<<QString::fromLocal8Bit ( group->text );
	}
	return names.size();
}

int CUPSPrint::getOptionsList ( const QString& groupName, QStringList& names,
                                QStringList& texts )
{
	names.clear();
	texts.clear();
	if ( !ppd )
		return -1;

	for ( int i=0;i<ppd->num_groups;++i )
	{
		ppd_group_t* group=& ( ppd->groups[i] );
		if ( groupName.length() >0 && groupName !=
		        QString::fromLocal8Bit ( group->name ) )
			continue;
		for ( int j=0;j<group->num_options;++j )
		{
			ppd_option_t* option=& ( group->options[j] );
			names<<QString::fromLocal8Bit ( option->keyword );
			texts<<QString::fromLocal8Bit ( option->text );
		}
	}
	return names.size();
}


bool CUPSPrint::setValue ( const QString& option, const QString& value,
                           QString& conflict_opt, QString& conflict_val )
{
	if ( !ppd )
		return false;
	int conflictsBefore= ppdConflicts ( ppd );
	QString valueBefore, textBefore;
	if ( !getOptionValue ( option,valueBefore,textBefore ) )
		return false;
	ppdMarkOption ( ppd,option.toAscii(),value.toAscii() );

	if ( conflictsBefore==ppdConflicts ( ppd ) )
	{
		return true;
	}

	//find conflicting option
	for ( int i=0;i<ppd->num_consts;++i )
	{
		QString confOpt,confVal;
		if ( option==ppd->consts[i].option1 &&
		        value==ppd->consts[i].choice1 )
		{
			confOpt=ppd->consts[i].option2;
			confVal=ppd->consts[i].choice2;
		}
		else if ( option==ppd->consts[i].option2 &&
		          value==ppd->consts[i].choice2 )
		{
			confOpt=ppd->consts[i].option1;
			confVal=ppd->consts[i].choice1;
		}
		else
			continue;
		QString selectedValue, selectedText;
		if ( getOptionValue ( confOpt,selectedValue,selectedText ) )
		{
			if ( selectedValue==confVal )
			{
				//conflicting option/choice found
				conflict_val=confVal;
				conflict_opt=confOpt;
				break;
			}
		}
	}


	//set previous value
	ppdMarkOption ( ppd,option.toAscii(),valueBefore.toAscii() );
	return false;
}


bool CUPSPrint::getOptionText ( const QString& option, QString& text )
{
	if ( !ppd )
		return false;
	ppd_option_t* opt=ppdFindOption ( ppd,option .toAscii() );
	if ( !opt )
		return false;
	text=QString::fromLocal8Bit ( opt->text );
	return true;
}

void CUPSPrint::setDefaults()
{
	//don't use ppdMarkDefaults here
	//ppdMarkDefaults do not unmark
	//already marked choices
	if ( !ppd )
		return;
	for ( int i=0;i<ppd->num_groups;++i )
	{
		ppd_group_t* group=& ( ppd->groups[i] );
		for ( int j=0;j<group->num_options;++j )
		{
			ppd_option_t* option=& ( group->options[j] );
			ppdMarkOption ( ppd,option->keyword,option->defchoice );

		}
	}
}



void CUPSPrint::saveOptions()
{
	if ( !ppd )
		return;
	X2goSettings st( "printing" );

	QStringList options;
	for ( int i=0;i<ppd->num_groups;++i )
	{
		ppd_group_t* group=& ( ppd->groups[i] );
		for ( int j=0;j<group->num_options;++j )
		{
			ppd_option_t* option=& ( group->options[j] );
			QString val,valtext;
			if ( !getOptionValue ( option->keyword,val,valtext ) )
				continue; //something is wrong here
			if ( val!=option->defchoice )
			{
				QString opt=option->keyword;
				opt+="="+val;
				options<<opt;
			}
		}
	}
	st.setting()->setValue ( "CUPS/options/"+currentPrinter,
	              QVariant ( options ) );
}


void CUPSPrint::loadUserOptions()
{
	X2goSettings st ( "printing" );
	QStringList options=st.setting()->value (
	                        "CUPS/options/"+currentPrinter ).toStringList();
	for ( int i=0;i<options.size();++i )
	{
		QStringList opt=options[i].split ( "=" );
		ppdMarkOption ( ppd,opt[0].toAscii(),opt[1].toAscii() );
	}
}


void CUPSPrint::print ( const QString& file, QString title )
{
	if ( !ppd )
		return;

	int num_options = 0;
	cups_option_t *options = NULL;


	for ( int i=0;i<ppd->num_groups;++i )
	{
		ppd_group_t* group=& ( ppd->groups[i] );
		for ( int j=0;j<group->num_options;++j )
		{
			ppd_option_t* option=& ( group->options[j] );
			QString val,valtext;
			if ( !getOptionValue ( option->keyword,val,valtext ) )
				continue; //something is wrong here
			if ( val!=option->defchoice )
			{
				num_options = cupsAddOption ( option->keyword,
				                              val.toAscii(),
				                              num_options,
				                              &options );
			}
		}
	}
	cupsPrintFile ( currentPrinter.toAscii(),file.toAscii(),
	                title.toAscii(), num_options,options );
	cupsFreeOptions ( num_options, options );
}
#endif
