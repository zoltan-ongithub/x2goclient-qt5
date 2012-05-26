# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: .
# Das Target ist eine Anwendung:  

#include (x2goclientconfig.pri)


CONFIG += $$(X2GO_CLIENT_TARGET)
CONFIG += $$(X2GO_LINUX_STATIC)
#CONFIG += console

FORMS += cupsprintsettingsdialog.ui \
           cupsprintwidget.ui \
           printdialog.ui \
           printercmddialog.ui \
           printwidget.ui \
           xsettingsui.ui \
           brokerpassdialog.ui \
           contest.ui \
           appdialog.ui

TRANSLATIONS += x2goclient_en.ts \
           x2goclient_de.ts \
           x2goclient_ru.ts \
           x2goclient_nb_no.ts \
           x2goclient_sv.ts \
           x2goclient_fr.ts \
           x2goclient_zh_tw.ts

HEADERS += configdialog.h \
           editconnectiondialog.h \
           exportdialog.h \
           imgframe.h \
           LDAPSession.h \
           onmainwindow.h \
           sessionbutton.h \
           sessionmanagedialog.h \
           sshmasterconnection.h \
           sshprocess.h \
           SVGFrame.h \
           userbutton.h \
           x2goclientconfig.h \
           x2gologdebug.h \
           printprocess.h \
           cupsprint.h \
           cupsprintwidget.h \
           cupsprintersettingsdialog.h \
           printwidget.h \
           printercmddialog.h \
           printdialog.h \
           wapi.h \
           sessionwidget.h \
           configwidget.h \
           connectionwidget.h \
           settingswidget.h \
           sharewidget.h \
           clicklineedit.h \
           httpbrokerclient.h \
           ongetpass.h \
           onmainwindow_privat.h \
           x2gosettings.h \
           brokerpassdlg.h \
           contest.h \
           xsettingswidget.h \
           appdialog.h

SOURCES += sharewidget.cpp \
           settingswidget.cpp\
           configwidget.cpp \
           sessionwidget.cpp \
           connectionwidget.cpp \
           configdialog.cpp \
           editconnectiondialog.cpp \
           exportdialog.cpp \
           imgframe.cpp \
           LDAPSession.cpp \
           onmainwindow.cpp \
           sessionbutton.cpp \
           sessionmanagedialog.cpp \
           sshmasterconnection.cpp \
           sshprocess.cpp \
           SVGFrame.cpp \
           userbutton.cpp \
           x2gologdebug.cpp \
           printprocess.cpp \
           cupsprint.cpp \
           cupsprintwidget.cpp \
           cupsprintersettingsdialog.cpp \
           printwidget.cpp \
           printercmddialog.cpp \
           printdialog.cpp \
           wapi.cpp \
           clicklineedit.cpp \
           httpbrokerclient.cpp \
           ongetpass.cpp \
           x2gosettings.cpp \
           brokerpassdlg.cpp \
           contest.cpp \
           xsettingswidget.cpp \
           appdialog.cpp

LIBS += -lssh

plugin {
	TARGET = x2goplugin
}
else{
	RC_FILE = x2goclient.rc
	SOURCES += x2goclient.cpp
	TARGET = x2goclient
	DEFINES += CFGCLIENT
	message(if you want to build x2goplugin you should export X2GO_CLIENT_TARGET=plugin)
}

!isEmpty(TRANSLATIONS) {
  isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
  }

  isEmpty(TS_DIR):TS_DIR = .

  TSQM.name = lrelease ${QMAKE_FILE_IN}
  TSQM.input = TRANSLATIONS
  TSQM.output = $$TS_DIR/${QMAKE_FILE_BASE}.qm
  TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
  TSQM.CONFIG = no_link
  QMAKE_EXTRA_COMPILERS += TSQM
  PRE_TARGETDEPS += compiler_TSQM_make_all
}
else:message(No translation files in project)

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES += resources.rcc

linux-g++ {
	message(building $$TARGET with ldap and cups)
	LIBS += -lldap -lcups -lX11 -lXpm
}
linux-g++-64 {
	message(building $$TARGET with ldap and cups)
	LIBS += -lldap -lcups -lX11 -lXpm
}
x2go_linux_static {
	message (linking all libs statically)
	LIBS -= -lssh
	LIBS += -lssh_static -lssl -lXpm
	QMAKE_LFLAGS = -Bstatic $$QMAKE_LFLAGS
}

macx {
	message(building $$TARGET with ldap and cups)
	LIBS += -framework LDAP -lcups -lcrypto -lssl -lz
	CONFIG += x86 x86_64
}
win32-* {
	message(building $$TARGET for windows without ldap and cups)
	LIBS += -lwinspool -lws2_32
	CONFIG += static release
}
QT += svg network
ICON =icons/x2go-mac.icns
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

plugin{

	DEFINES += CFGPLUGIN
	linux-g++ {
		include(qtbrowserplugin-2.4_1-opensource/src/qtbrowserplugin.pri)
	}
	linux-g++-64 {
		include(qtbrowserplugin-2.4_1-opensource/src/qtbrowserplugin.pri)
	}
	win32-* {
		DEFINES += QT_NODLL
		CONFIG += qaxserver
		include(qtbrowserplugin-2.4_1-opensource/src/qtbrowserplugin.pri)
		}
	RC_FILE = x2goplugin.rc

}
