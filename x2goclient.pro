# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: .
# Das Target ist eine Anwendung:  

include (x2goclientconfig.pri)

TRANSLATIONS += x2goclient_de.ts 
TRANSLATIONS += x2goclient_ru.ts 
TRANSLATIONS += x2goclient_fr.ts 
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
 embedwidget.h \
 wapi.h \
 sessionwidget.h \
 configwidget.h \
 connectionwidget.h \
 settingswidget.h \
 sharewidget.h \
 clicklineedit.h \
 httpbrokerclient.h \
 ongetpass.h \
 x2gosettings.h

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
 embedwidget.cpp \
 wapi.cpp \
 clicklineedit.cpp \
 httpbrokerclient.cpp \
 ongetpass.cpp \
 x2gosettings.cpp

client{
SOURCES += x2goclient.cpp
TARGET = x2goclient
DEFINES += CFGCLIENT
}
else {
TARGET = x2goplugin
RC_FILE = x2goclient.rc
}

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES += resources.rcc

linux-g++ {
    message(building $$TARGET with ldap and cups)
    LIBS += -lldap -lcups
}
macx {
    message(building $$TARGET with ldap and cups)
    LIBS += -lldap -lcups
}
win32-* {
    message(building $$TARGET for windows without ldap and cups)
    LIBS += -lwinspool
    CONFIG += static
}
LIBS += -lssh
QT += svg network
ICON =icons/x2go-mac.icns
QMAKE_MAC_SDK =/Developer/SDKs/MacOSX10.6.sdk
FORMS += cupsprintsettingsdialog.ui \
cupsprintwidget.ui \
printwidget.ui \
printercmddialog.ui \
printdialog.ui


plugin{

DEFINES += CFGPLUGIN
   linux-g++ {
      include(../../../qtbrowserplugin-2.4_1-opensource/src/qtbrowserplugin.pri)
   }
   win32-* {
         DEFINES += QT_NODLL
         CONFIG += qaxserver
         include(../qtbrowserplugin-2.4_1-opensource/src/qtbrowserplugin.pri)
   }
RC_FILE = x2goplugin.rc
}
