#if !defined(_X2GOCLIENT_CONFIG_H_)
#define _X2GOCLIENT_CONFIG_H_

#include <stdio.h>
#include <qconfig.h>
#include <qglobal.h>


// #define LOGFILE QDir::homePath()+"/x2goclient.log"

#if !defined Q_OS_WIN
#define USELDAP
#endif

#ifdef Q_OS_WIN
#undef USELDAP
#endif

#if defined Q_WS_HILDON
#undef USELDAP
#endif

#endif
