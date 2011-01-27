//
// C++ Interface: x2gosettings
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef X2GOSETTINGS_H
#define X2GOSETTINGS_H

#include <QSettings>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class X2goSettings
{
	public:
		X2goSettings ( QString group );
		~X2goSettings();
		QSettings* setting()
		{
			return set;
		}
	private:
		QSettings* set;

};

#endif
