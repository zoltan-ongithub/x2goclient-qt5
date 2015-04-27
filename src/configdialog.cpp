/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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

#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include "x2gosettings.h"
#include <QDir>
#include <QFileDialog>
#include "onmainwindow.h"
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include "configdialog.h"
#include "x2gologdebug.h"
#include "printwidget.h"
#include <QTabWidget>
#include <algorithm>
#include "x2goclientconfig.h"
#include "connectionwidget.h"
#include "settingswidget.h"
#include "mediawidget.h"
#include "x2goutils.h"


#if defined ( Q_OS_WIN) && defined (CFGCLIENT )
#include "xsettingswidget.h"
#endif


ConfigDialog::ConfigDialog ( QWidget * parent,  Qt::WFlags f )
    : QDialog ( parent,f )
{

    tabWidg=new QTabWidget ( this );
    QVBoxLayout* ml=new QVBoxLayout ( this );
    ml->addWidget ( tabWidg );

    QWidget *fr=new QWidget ( this );
    QVBoxLayout* frLay=new QVBoxLayout ( fr );
    tabWidg->addTab ( fr,tr ( "General" ) );

    embedMode= ( ( ONMainWindow* ) parent )->getEmbedMode();

    X2goSettings st ( "settings" );


#ifndef CFGPLUGIN

    gbTrayIcon=new QGroupBox(tr("Display icon in system tray"),fr);
    frLay->addWidget(gbTrayIcon);
    gbTrayIcon->setCheckable(true);
    QHBoxLayout* grmainLay=new QHBoxLayout(gbTrayIcon);
    QFrame* frTray=new QFrame(gbTrayIcon);
    grmainLay->setMargin(0);
    grmainLay->addWidget(frTray);
    cbMinToTray=new QCheckBox(tr("Hide to system tray when minimized"),frTray);
    cbNoClose=new QCheckBox(tr("Hide to system tray when closed"),frTray);
    cbMinimizeTray=new QCheckBox(tr("Hide to system tray after connection is established"),frTray);
    cbMaxmizeTray=new QCheckBox(tr("Restore from system tray after session is disconnected"),frTray);
    QVBoxLayout* trLay=new QVBoxLayout(frTray);
    trLay->addWidget(cbMinToTray);
    trLay->addWidget(cbNoClose);
    trLay->addWidget(cbMinimizeTray);
    trLay->addWidget(cbMaxmizeTray);
    gbTrayIcon->setChecked ( st.setting()->value ( "trayicon/enabled", false ).toBool() );
    cbMinimizeTray->setChecked ( st.setting()->value ( "trayicon/mincon", false ).toBool() );
    cbMaxmizeTray->setChecked ( st.setting()->value ( "trayicon/maxdiscon", false ).toBool() );
    cbNoClose->setChecked ( st.setting()->value ( "trayicon/noclose", false ).toBool() );
    cbMinToTray->setChecked ( st.setting()->value ( "trayicon/mintotray", false ).toBool() );
#endif

#ifdef USELDAP
    if ( !embedMode )
    {
        ONMainWindow* par= ( ONMainWindow* ) parent;

        gbLDAP=new QGroupBox ( tr ( "Use LDAP" ),fr );
        gbLDAP->setCheckable(true);
        QHBoxLayout* grmainLay=new QHBoxLayout(gbLDAP);
        QFrame* frLdap=new QFrame(gbLDAP);
        grmainLay->setMargin(0);
        grmainLay->addWidget(frLdap);

        ldapServer=new QLineEdit ( frLdap );
        port=new QSpinBox ( frLdap );
        ldapBase=new QLineEdit ( frLdap );
        port->setMaximum ( 1000000 );

        QHBoxLayout *grLay=new QHBoxLayout ( frLdap );

        QVBoxLayout *laiLay=new QVBoxLayout();
        QVBoxLayout *setLay=new QVBoxLayout();
        setLay->setSpacing ( 6 );
        laiLay->setSpacing ( 6 );

        grLay->setSpacing ( 20 );
        grLay->addLayout ( laiLay );
        grLay->addStretch();
        grLay->addLayout ( setLay );

        laiLay->addWidget ( new QLabel ( tr ( "Server URL:" ),frLdap) );
        laiLay->addWidget ( new QLabel ( tr ( "BaseDN:" ),frLdap ) );
        laiLay->addWidget ( new QLabel (
                                tr ( "Failover server 1 URL:" ),frLdap ) );
        laiLay->addWidget ( new QLabel (
                                tr ( "Failover server 2 URL:" ),frLdap ) );

        ldapServer1=new QLineEdit ( frLdap);
        port1=new QSpinBox ( frLdap );
        ldapServer2=new QLineEdit ( frLdap );
        port2=new QSpinBox ( frLdap );
        port1->setMaximum ( 1000000 );
        port2->setMaximum ( 1000000 );


        QHBoxLayout* aLay=new QHBoxLayout();
        aLay->setSpacing ( 3 );
        aLay->addWidget ( new QLabel ( "ldap//:",frLdap ) );
        aLay->addWidget ( ldapServer );
        aLay->addWidget ( new QLabel ( ":",frLdap ) );
        aLay->addWidget ( port );

        QHBoxLayout* aLay1=new QHBoxLayout();
        aLay1->setSpacing ( 3 );
        aLay1->addWidget ( new QLabel ( "ldap//:",frLdap ) );
        aLay1->addWidget ( ldapServer1 );
        aLay1->addWidget ( new QLabel ( ":",frLdap ) );
        aLay1->addWidget ( port1 );

        QHBoxLayout* aLay2=new QHBoxLayout();
        aLay2->setSpacing ( 3 );
        aLay2->addWidget ( new QLabel ( "ldap//:",frLdap ) );
        aLay2->addWidget ( ldapServer2 );
        aLay2->addWidget ( new QLabel ( ":",frLdap ) );
        aLay2->addWidget ( port2 );


        setLay->addLayout ( aLay );
        setLay->addWidget ( ldapBase );
        setLay->addLayout ( aLay1 );
        setLay->addLayout ( aLay2 );


        gbLDAP->setChecked ( st.setting()->value ( "LDAP/useldap",
                             ( QVariant ) par->retUseLdap()
                                                 ).toBool() );
        ldapServer->setText ( st.setting()->value (
                                  "LDAP/server",
                                  ( QVariant ) par->retLdapServer()
                              ).toString() );
        port->setValue ( st.setting()->value ( "LDAP/port",
                                               ( QVariant ) par->retLdapPort()
                                             ).toInt() );
        ldapServer1->setText ( st.setting()->value (
                                   "LDAP/server1",
                                   ( QVariant ) par->retLdapServer1()
                               ).toString() );
        port1->setValue ( st.setting()->value ( "LDAP/port1",
                                                ( QVariant ) par->retLdapPort1()
                                              ).toInt() );
        ldapServer2->setText ( st.setting()->value (
                                   "LDAP/server2",
                                   ( QVariant ) par->retLdapServer2()
                               ).toString() );
        port2->setValue ( st.setting()->value ( "LDAP/port2",
                                                ( QVariant ) par->retLdapPort2()
                                              ).toInt() );
        ldapBase->setText ( st.setting()->value ( "LDAP/basedn",
                            ( QVariant ) par->retLdapDn()
                                                ).toString() );
        frLdap->setEnabled ( gbLDAP->isChecked() );
        frLay->addWidget ( gbLDAP );
        connect ( gbLDAP,SIGNAL ( toggled ( bool ) ),frLdap,
                  SLOT ( setEnabled ( bool ) ) );

        connect ( gbLDAP,SIGNAL ( toggled ( bool ) ),this,
                  SLOT ( slot_checkOkStat() ) );
        connect ( ldapBase,SIGNAL ( textChanged ( const QString& ) ),
                  this,
                  SLOT ( slot_checkOkStat() ) );
        connect ( ldapServer,SIGNAL ( textChanged ( const QString& ) ),
                  this,
                  SLOT ( slot_checkOkStat() ) );
    }
#endif  //USELDAP

#ifdef Q_OS_DARWIN
    QGroupBox* xgb=new QGroupBox ( tr ( "X-Server settings" ),fr );
    QGridLayout *xLay=new QGridLayout ( xgb );

    leXexec=new QLineEdit ( xgb );
    leXexec->setReadOnly ( true );
    pbOpenExec=new QPushButton (
        QIcon ( ( ( ONMainWindow* ) parent )->iconsPath (
                    "/32x32/file-open.png" ) ),
        QString::null,xgb );
    xLay->addWidget ( new QLabel ( tr ( "X11 application:" ) ),0,0 );

    leCmdOpt=new QLineEdit ( xgb );
    leCmdOpt->setReadOnly ( true );

    QHBoxLayout* cmdLay=new QHBoxLayout();
    cmdLay->addWidget ( leXexec );
    cmdLay->addWidget ( pbOpenExec );


    xLay->addLayout ( cmdLay,0,1 );
    xLay->addWidget ( new QLabel ( tr ( "X11 version:" ) ),1,0 );
    xLay->addWidget ( leCmdOpt,1,1 );
    frLay->addWidget ( xgb );

    QString xver;
    QString path=getXDarwinDirectory();
    if ( path!="" )
    {
        leXexec->setText ( findXDarwin ( xver,path ) );
        leCmdOpt->setText ( xver );
    }
    else
        slot_findXDarwin();

    QPushButton* findButton=new QPushButton (
        tr ( "Find X11 application" ),xgb );
    xLay->addWidget ( findButton,2,1 );
    connect ( findButton,SIGNAL ( clicked() ),this,
              SLOT ( slot_findXDarwin() ) );
    connect ( pbOpenExec,SIGNAL ( clicked() ),this,
              SLOT ( slot_selectXDarwin() ) );

#endif //Q_OS_DARWIN

#ifndef Q_OS_WIN
    clientSshPort=new QSpinBox ( fr );
    clientSshPort->setMaximum ( 1000000 );
    clientSshPort->setValue ( st.setting()->value ( "clientport",
                              ( QVariant ) 22 ).toInt() );

    QHBoxLayout* sshLay=new QHBoxLayout();
    sshLay->addWidget (
        new QLabel ( tr (
                         "Clientside SSH port for file system export usage:"
                     ),fr ) );
    sshLay->addWidget ( clientSshPort );
    sshLay->addStretch();
    frLay->addLayout ( sshLay );
#endif
    if ( embedMode )
    {
        cbStartEmbed=new QCheckBox (
            tr (
                "Start session embedded inside website" ) ,fr );
        frLay->addWidget ( cbStartEmbed );
        advancedOptions=new QPushButton (
            tr ( "Advanced options" ) +" >>" ,
            this );
        connect ( advancedOptions,SIGNAL ( clicked() ),this,
                  SLOT ( slotAdvClicked() ) );
        advancedOptions->setVisible ( (
                                          ( ONMainWindow* ) parent )->
                                      getShowAdvOption() );
        advOptionsShown=false;
        conWidg=new ConnectionWidget ( QString::null,
                                       ( ONMainWindow* ) parent,this );
        setWidg=new SettingsWidget ( QString::null,
                                     ( ONMainWindow* ) parent,this );
        mediaWidget=new MediaWidget ( QString::null,
                                     ( ONMainWindow* ) parent,this );
        conWidg->hide();
        setWidg->hide();
        mediaWidget->hide();

        X2goSettings st ( "sessions" );
        cbStartEmbed->setChecked (
            st.setting()->value ( "embedded/startembed",
                                  true ).toBool() );
    }
    /* #ifdef Q_OS_WIN
    	else
    	{
    		tabWidg->removeTab ( 0 );
    	}
     #endif*/
    frLay->addStretch();
    defaults=new QPushButton ( tr ( "Defaults" ),this );
    ok=new QPushButton ( tr ( "&OK" ),this );
    QPushButton* cancel=new QPushButton ( tr ( "&Cancel" ),this );
    QHBoxLayout* bLay=new QHBoxLayout();

    connect ( this,SIGNAL ( accepted() ),this,SLOT ( slot_accepted() ) );
    connect ( ok,SIGNAL ( clicked() ),this,SLOT ( accept() ) );
    connect ( cancel,SIGNAL ( clicked() ),this,SLOT ( reject() ) );
    connect ( defaults,SIGNAL ( clicked() ),this,
              SLOT ( slotDefaults() ) );


    bLay->setSpacing ( 5 );
    if ( embedMode )
        bLay->addWidget ( advancedOptions );
    bLay->addStretch();
    bLay->addWidget ( ok );
    bLay->addWidget ( cancel );
    bLay->addWidget ( defaults );
    ml->addLayout ( bLay );

    setSizeGripEnabled ( true );
    setWindowIcon ( QIcon ( (
                                ( ONMainWindow* ) parent )->iconsPath (
                                "/32x32/edit_settings.png" ) ) );
    setWindowTitle ( tr ( "Settings" ) );

#ifdef Q_WS_HILDON
    QFont fnt=font();
    fnt.setPointSize ( 10 );
    setFont ( fnt );
    QSize sz=ok->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    ok->setFixedSize ( sz );
    sz=cancel->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    cancel->setFixedSize ( sz );
    clientSshPort->setFixedHeight (
        int ( clientSshPort->sizeHint().height() *1.5 ) );
    defaults->hide();
#endif

    pwid=new PrintWidget ( this );
    tabWidg->addTab ( pwid,tr ( "Printing" ) );

#if defined ( Q_OS_WIN) && defined (CFGCLIENT )
    xsetWidg=new XSettingsWidget(this);
    tabWidg->addTab(xsetWidg, tr("X-Server settings"));
#endif

}


ConfigDialog::~ConfigDialog()
{}


void ConfigDialog::slot_accepted()
{
    X2goSettings st ( "settings" );
#ifndef CFGPLUGIN
    st.setting()->setValue ( "trayicon/enabled", gbTrayIcon->isChecked() );
    st.setting()->setValue ( "trayicon/mintotray", cbMinToTray->isChecked() );
    st.setting()->setValue ( "trayicon/noclose", cbNoClose->isChecked() );
    st.setting()->setValue ( "trayicon/mincon", cbMinimizeTray->isChecked() );
    st.setting()->setValue ( "trayicon/maxdiscon", cbMaxmizeTray->isChecked() );
#endif
#ifdef USELDAP
    if ( !embedMode )
    {
        st.setting()->setValue ( "LDAP/useldap",
                                 ( QVariant ) gbLDAP->isChecked() );
        st.setting()->setValue ( "LDAP/port",
                                 ( QVariant ) port->value() );
        if ( ldapServer->text().length() )
            st.setting()->setValue ( "LDAP/server",
                                     ( QVariant ) ldapServer->text() );
        st.setting()->setValue ( "LDAP/port1",
                                 ( QVariant ) port1->value() );
        if ( ldapServer1->text().length() )
            st.setting()->setValue ( "LDAP/server1", ( QVariant )
                                     ldapServer1->text() );
        st.setting()->setValue ( "LDAP/port2",
                                 ( QVariant ) port2->value() );
        if ( ldapServer2->text().length() )
            st.setting()->setValue ( "LDAP/server2", ( QVariant )
                                     ldapServer2->text() );
        if ( ldapBase->text().length() )
            st.setting()->setValue ( "LDAP/basedn",
                                     ( QVariant ) ldapBase->text() );
    }
#endif //USELDAP
#ifdef Q_OS_DARWIN
    st.setting()->setValue ( "xdarwin/directory",
                             ( QVariant ) leXexec->text() );
#endif
#ifndef Q_OS_WIN
    st.setting()->setValue ( "clientport",
                             ( QVariant ) clientSshPort->value() );
#endif
    pwid->saveSettings();
    if ( embedMode )
    {
        X2goSettings st ( "sessions" );
        st.setting()->setValue ( "embedded/startembed",
                                 ( QVariant ) cbStartEmbed->isChecked() );
        st.setting()->sync();
        setWidg->saveSettings();
        mediaWidget->saveSettings();
        conWidg->saveSettings();
    }
#if defined ( Q_OS_WIN) && defined (CFGCLIENT )
    xsetWidg->saveSettings();
#endif
}


void ConfigDialog::slot_checkOkStat()
{
    ok->setEnabled ( ( !gbLDAP->isChecked() ) ||
                     ( ( ldapBase->text().length() &&
                         ldapServer->text().length() ) ) );
}


#ifdef Q_OS_WIN
QString ConfigDialog::getCygwinDir ( const QString& dir )
{
    QString cygdir=QString::null;
    QSettings lu_st ( "HKEY_CURRENT_USER\\Software"
                      "\\Cygnus Solutions\\Cygwin\\mounts v2\\"+
                      dir,QSettings::NativeFormat );
    cygdir=lu_st.value ( "native", ( QVariant ) QString::null ).toString();
    if ( cygdir!= QString::null )
        return cygdir;
    QSettings lm_st ( "HKEY_LOCAL_MACHINE\\SOFTWARE"
                      "\\Cygnus Solutions\\Cygwin\\mounts v2\\"+
                      dir,QSettings::NativeFormat );
    return lm_st.value ( "native", ( QVariant ) QString::null ).toString();
}
#endif

#ifdef Q_OS_DARWIN
QString ConfigDialog::retMaxXDarwinVersion ( QString v1, QString v2 )
{
    QStringList vl1=v1.split ( "." );
    QStringList vl2=v2.split ( "." );
    for ( int i=0; i<3; ++i )
    {
        if ( vl1.count() <i+1 )
            vl1<<"0";
        if ( vl2.count() <i+1 )
            vl2<<"0";
        if ( vl1[i].toInt() !=vl2[i].toInt() )
            return ( vl1[i].toInt() > vl2[i].toInt() ) ?v1:v2;
    }
    return v1;
}

QString ConfigDialog::findXDarwin ( QString& version, QString path )
{
    if (path.isEmpty ()) {
        QStringList locations;
        locations << "/Applications/Utilities/XQuartz.app"
                  << "/usr/X11/X11.app"
                  << "/Applications/MacPorts/X11.app";

        QStringList versions;

        QStringList::const_iterator const_it;
        for (const_it = locations.constBegin (); const_it != locations.constEnd (); ++const_it) {
            QFile info_plist (*const_it + "/Contents/Info.plist");

            if (info_plist.exists ()) {
                QSettings settings (info_plist.fileName (), QSettings::NativeFormat);
                versions << settings.value ("CFBundleShortVersionString", (QVariant) "0.0.0").toString ();
            }
            else {
                versions << QString ("0.0.0");
            }
        }

        QString max_ver = QString ();
        std::size_t max_idx = 0;
        if (locations.size () < 1) {
            version = QString ("0.0.0");
            return (QString ());
        }
        else if (locations.size () == 1) {
            max_ver = versions.at (0);
            max_idx = 0;
        }
        else {
            max_ver = versions.at (0);
            max_idx = 0;
            for (std::size_t i = 1; i < static_cast<std::size_t> (locations.size ()); ++i) {
                QString cur_ver = versions.at (i);
                if (retMaxXDarwinVersion (max_ver, cur_ver).compare (cur_ver) == 0) {
                    max_ver = cur_ver;
                    max_idx = i;
                }
            }
        }

        version = max_ver;

        if (max_ver.compare (QString ("0.0.0")) == 0) {
            return (QString ());
        }
        else {
            return (locations.at (max_idx));
        }
    }
    else {
        version = "0.0.0";
        if (QFile::exists (path + "/Contents/Info.plist")) {
            QSettings vst (path + "/Contents/Info.plist",
                           QSettings::NativeFormat);
            version=vst.value ("CFBundleShortVersionString",
                               (QVariant) "0.0.0").toString ();
        }
        else {
            path = QString ();
        }
        return path;
    }
}


void ConfigDialog::slot_findXDarwin ()
{
    QString version;
    QString path = findXDarwin (version);
    if (path.isEmpty ()) {
      show_XQuartz_not_found_error ();
    }
    else {
        QString minVer = "2.1.0";
        if (retMaxXDarwinVersion (minVer, version) == minVer) {
            printXDarwinVersionWarning (version);
        }

        leXexec->setText (path);
        leCmdOpt->setText (version);
    }
}

void ConfigDialog::printXDarwinVersionWarning (QString version)
{
    show_RichText_WarningMsgBox (tr ("Your XQuartz version is too old."),
                                 tr ("Your are using XQuartz (X Window System Server for OS X) version ")
                                 + version +
                                 tr (".\n\n"
                                     "This version causes problems with X applications in 24bit "
                                     "color mode.\n"
                                     "You should update your X11 environment.\n\n"

                                     "MacPorts users please follow the steps outlined on:\n"

                                     "<center><a href=\"https://guide.macports.org/chunked/using.common-tasks.html\">"
                                         "https://guide.macports.org/chunked/using.common-tasks.html"
                                     "</a></center>\n\n"

                                     "Users who have installed XQuartz via the installer package\n"
                                     "can find updated versions on:\n"

                                     "<center><a href=\"https://xquartz.macosforge.org/\">"
                                         "https://xquartz.macosforge.org/"
                                     "</a></center>"));
}


void ConfigDialog::slot_selectXDarwin ()
{
    QString newDir = QFileDialog::getOpenFileName (this, QString (), leXexec->text () + "/..");
    QString version;
    if (newDir.length () > 0) {
        findXDarwin (version, newDir);
        if (version.compare (QString ("0.0.0")) == 0) {
            show_RichText_WarningMsgBox (tr ("No valid XQuartz application selected."),
                                         tr ("You did not select a valid XQuartz application.\n"
                                             "Please try again.\n\n"

                                             "Some standard installation locations may be:\n"
                                             "<center><b>/Applications/Utilities/X11.app</b></center>\n"
                                             "<center><b>/Applications/Utilities/XQuartz.app</b></center>\n"
                                             "<center><b>/Applications/MacPorts/X11.app</b></center>"));
        }
        else {
            QString minVer = "2.1.0";
            if (retMaxXDarwinVersion (minVer, version) == minVer) {
                printXDarwinVersionWarning (version);
            }
            leXexec->setText (newDir);
            leCmdOpt->setText (version);
        }
    }
}

QString ConfigDialog::getXDarwinDirectory()
{
    X2goSettings st ("settings");
    return st.setting()->value ( "xdarwin/directory",
                                 ( QVariant ) "" ).toString() ;
}
#endif


void ConfigDialog::slotAdvClicked()
{
    if ( advOptionsShown )
    {
        advancedOptions->setText ( tr ( "Advanced options" ) +" >>" );
        conWidg->hide();
        setWidg->hide();
        mediaWidget->hide();
        conWidg->setParent ( this );
        setWidg->setParent ( this );
        mediaWidget->setParent (this);
        tabWidg->removeTab ( 4 );
        tabWidg->removeTab ( 3 );
        tabWidg->removeTab ( 2 );
    }
    else
    {
        tabWidg->addTab ( conWidg,tr ( "&Connection" ) );
        tabWidg->addTab ( setWidg, tr ( "&Input/Output" ) );
        tabWidg->addTab ( mediaWidget, tr ( "&Media" ) );
        advancedOptions->setText ( tr ( "Advanced options" ) +" <<" );
    }
    advOptionsShown=!advOptionsShown;
}


void ConfigDialog::slotDefaults()
{
    switch ( tabWidg->currentIndex() )
    {
    case 0:
    {
        if ( embedMode )
            cbStartEmbed->setChecked ( true );
        clientSshPort->setValue ( 22 );
#ifndef CFGPLUGIN
        gbTrayIcon->setChecked (false);
        cbMinimizeTray->setChecked (false);
        cbMaxmizeTray->setChecked ( false);
        cbNoClose->setChecked (false);
        cbMinToTray->setChecked (false);
#endif
    }
    break;
#if defined ( Q_OS_WIN) && defined (CFGCLIENT )
    case 2:
    {
        xsetWidg->setDefaults();
    }
    break;
#else
    case 1:
        break;
    case 2:
    {
        conWidg->setDefaults();
    }
    break;
    case 3:
    {
        setWidg->setDefaults();
    }
    break;
    case 4:
    {
        mediaWidget->setDefaults();
    }
    break;
#endif
    }

}
