#if !defined(_X2GOCLIENT_CONFIG_H_)
#define _X2GOCLIENT_CONFIG_H_

#include <stdio.h>
#include <qconfig.h>

#ifdef __MINGW_H //wir support only mingw to build x2goclient for windows
#define WINDOWS
#endif     

//#define LOGFILE QDir::homePath()+"/x2goclient.log"

#if !defined WINDOWS
#define USELDAP
#endif


#if defined Q_WS_HILDON
#undef USELDAP
#endif

#endif
