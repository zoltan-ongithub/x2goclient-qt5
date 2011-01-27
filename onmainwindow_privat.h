/***************************************************************************
 *   Copyright (C) 2005-2011 by Oleksandr Shneyder   *
 *   oleksandr.shneyder@obviously-nice.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef ONMAINWINDOWPRIVAT_H
#define ONMAINWINDOWPRIVAT_H


#include <QTextStream>
#include <QTranslator>

#include "version.h"
#include "x2goclientconfig.h"
#include "onmainwindow.h"
#include "userbutton.h"
#include "exportdialog.h"
#include "printprocess.h"
#include <QDesktopServices>
#include <QApplication>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>

#include "httpbrokerclient.h"
#include <QTimer>
#include <QComboBox>
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QTextEdit>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut>
#include "x2gosettings.h"
#include <QStatusBar>
#include <QInputDialog>
#include <QDir>
#include <QTreeView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QCheckBox>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QHttp>
#include <QUrl>
#include <QtNetwork/QTcpSocket>
#include <QPlastiqueStyle>
#include "sshprocess.h"
#include "imgframe.h"
#include <QToolTip>
#include "clicklineedit.h"
#include <QThread>


#include "sshmasterconnection.h"

#if !defined Q_OS_WIN
#include <sys/mount.h>
#ifdef Q_OS_LINUX
#include <linux/fs.h>
#endif // Q_OS_LINUX
#endif // !defined Q_OS_WIN

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QCoreApplication>

#include <QDesktopWidget>

#define ldap_SUCCESS 0
#define ldap_INITERROR 1
#define ldap_OPTERROR 2
#define ldap_BINDERROR 3
#define ldap_SEARCHERROR 4
#define ldap_NOBASE 5



//LDAP attributes
#define SESSIONID "sn"
#define USERNAME  "cn"
#define CLIENT    "registeredAddress"
#define SERVER    "postalAddress"
#define RES       "title"
#define DISPLAY   "street"
#define STATUS    "st"
#define STARTTIME "telephoneNumber"
#define CREATTIME "telexNumber"
#define SUSPTIME  "internationaliSDNNumber"

#define SESSIONCMD "o"
#define FIRSTUID "ou"
#define LASTUID "l"

#define SNDSUPPORT "sn"
#define NETSOUNDSYSTEM "o"
#define SNDSUPPORT "sn"
#define SNDPORT   "ou"
#define STARTSNDSERVER "title"



#include <QDateTime>

#include "SVGFrame.h"
#include "configdialog.h"
#include "editconnectiondialog.h"
#include "sessionbutton.h"
#include "sessionmanagedialog.h"
#include "x2gologdebug.h"
#include <QMouseEvent>

#ifdef Q_OS_WIN
#include "wapi.h"
#endif

#ifdef Q_OS_LINUX
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef CFGPLUGIN
#ifdef Q_OS_LINUX
#include <dlfcn.h>
#include <QX11EmbedContainer>
#endif
#endif


#endif //ONMAINWINDOWPRIVAT_H
