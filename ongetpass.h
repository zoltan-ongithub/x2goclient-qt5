//
// C++ Header File
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ONGETPASS_H
#define ONGETPASS_H
#include <QString>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef Q_OS_WIN
__declspec(dllexport)	
#endif	
int x2goMain ( int argc, char *argv[] );

void askpass ( const QString& param, const QString& accept,
               const QString& cookie, const QString& socketName );


#ifdef __cplusplus
}
#endif


#endif
