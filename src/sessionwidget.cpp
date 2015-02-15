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

#include "sessionwidget.h"
#include "x2goutils.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QIcon>
#include <QComboBox>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QInputDialog>
#include <QCheckBox>
#include "onmainwindow.h"
#include "x2gosettings.h"
#include <QMessageBox>
#include <QButtonGroup>
#include <QRadioButton>
#include "folderexplorer.h"
#include "sessionexplorer.h"

SessionWidget::SessionWidget ( bool newSession, QString id, ONMainWindow * mw,
                               QWidget * parent, Qt::WindowFlags f )
    : ConfigWidget ( id,mw,parent,f )
{
    QVBoxLayout* sessLay=new QVBoxLayout ( this );
#ifdef Q_WS_HILDON
    sessLay->setMargin ( 2 );
#endif
    this->parent=mw;
    this->newSession=newSession;

    sessName=new QLineEdit ( this );
    icon=new QPushButton ( QString::null,this );
    if ( !miniMode )
    {
        icon->setIconSize ( QSize ( 100,100 ) );
        icon->setFixedSize ( 100,100 );
    }
    else
    {
        icon->setIconSize ( QSize ( 48,48 ) );
        icon->setFixedSize ( 48,48 );
    }
    icon->setFlat ( true );

    QHBoxLayout* slay=new QHBoxLayout();
    slay->addWidget ( new QLabel ( tr ( "Session name:" ),this ) );
    slay->addWidget ( sessName );

    QHBoxLayout* ilay=new QHBoxLayout();
    ilay->addWidget ( icon );
    ilay->addWidget ( new QLabel ( tr ( "<< change icon" ),this ) );

    lPath=new QLabel(this);
    lPath->setFrameStyle(QFrame::StyledPanel);
    QPushButton* pathButton=new QPushButton("...",this);
    QHBoxLayout* pathLay=new QHBoxLayout();
    pathLay->addWidget(new QLabel(tr("Path:"), this),0);
    pathLay->addWidget(lPath,1);
    pathLay->addWidget(pathButton,0);

#ifndef Q_WS_HILDON
    QGroupBox *sgb=new QGroupBox ( tr ( "&Server" ),this );
#else
    QFrame* sgb=this;
#endif
    server=new QLineEdit ( sgb );
    uname=new QLineEdit ( sgb );
    sshPort=new QSpinBox ( sgb );
    sshPort->setValue ( mainWindow->getDefaultSshPort().toInt() );
    sshPort->setMinimum ( 1 );
    sshPort->setMaximum ( 999999999 );
#ifdef Q_OS_LINUX
    rdpPort=new QSpinBox ( sgb );
    rdpPort->setValue ( mainWindow->getDefaultSshPort().toInt() );
    rdpPort->setMinimum ( 1 );
    rdpPort->setMaximum ( 999999999 );
#endif
    key=new QLineEdit ( sgb );

#ifndef Q_WS_HILDON
    openKey=new QPushButton (
        QIcon ( mainWindow->iconsPath (
                    "/32x32/file-open.png" ) ),
        QString::null,sgb );
    QVBoxLayout *sgbLay = new QVBoxLayout ( sgb );
#else
    QPushButton* openKey=new QPushButton (
        QIcon ( mainWindow->iconsPath ( "/16x16/file-open.png" ) ),
        QString::null,sgb );
    QVBoxLayout *sgbLay = new QVBoxLayout ();
#endif
    QHBoxLayout *suLay =new QHBoxLayout();
    QVBoxLayout *slLay =new QVBoxLayout();
    QVBoxLayout *elLay =new QVBoxLayout();
    slLay->addWidget ( new QLabel ( tr ( "Host:" ),sgb ) );
    slLay->addWidget ( new QLabel ( tr ( "Login:" ),sgb ) );
    lPort=new QLabel ( tr ( "SSH port:" ),sgb );
    slLay->addWidget ( lPort );
    elLay->addWidget ( server );
    elLay->addWidget ( uname );
    elLay->addWidget ( sshPort );
#ifdef Q_OS_LINUX
    elLay->addWidget ( rdpPort );
#endif
    suLay->addLayout ( slLay );
    suLay->addLayout ( elLay );
#ifdef Q_WS_HILDON
    sshPort->setFixedHeight ( int ( sshPort->sizeHint().height() *1.5 ) );
#endif

    QHBoxLayout *keyLay =new QHBoxLayout();
    lKey=new QLabel ( tr ( "Use RSA/DSA key for ssh connection:" ),sgb );
    keyLay->addWidget (lKey );
    keyLay->addWidget ( key );
    keyLay->addWidget ( openKey );

    sgbLay->addLayout ( suLay );
    sgbLay->addLayout ( keyLay );
    cbAutoLogin=new QCheckBox(tr("Try auto login (ssh-agent or default ssh key)"),sgb);
    cbKrbLogin=new QCheckBox(tr("Kerberos 5 (GSSAPI) authentication"),sgb);
    cbKrbDelegation=new QCheckBox(tr("Delegation of GSSAPI credentials to the server"),sgb);
    sgbLay->addWidget(cbAutoLogin);
    sgbLay->addWidget(cbKrbLogin);
    sgbLay->addWidget(cbKrbDelegation);
    cbProxy=new QCheckBox(tr("Use Proxy server for SSH connection"),sgb);
    proxyBox=new QGroupBox(tr("Proxy server"),sgb);
    sgbLay->addWidget(cbProxy);
    sgbLay->addWidget(proxyBox);
    QGridLayout* proxyLaout=new QGridLayout(proxyBox);
    QButtonGroup* proxyType=new QButtonGroup(proxyBox);
    proxyType->setExclusive(true);
    rbSshProxy=new QRadioButton(tr("SSH"),proxyBox);
    rbHttpProxy=new QRadioButton(tr("HTTP"),proxyBox);
    proxyType->addButton(rbSshProxy);
    proxyType->addButton(rbHttpProxy);
    proxyHost=new QLineEdit(proxyBox);
    proxyPort=new QSpinBox(proxyBox);
    proxyPort->setMinimum ( 1 );
    proxyPort->setMaximum ( 999999999 );

    cbProxySameUser=new QCheckBox(tr("Same login as on X2Go Server"), proxyBox);
    proxyLogin=new QLineEdit(proxyBox);
    cbProxySamePass=new QCheckBox(tr("Same password as on X2Go Server"), proxyBox);

    proxyKeyLabel=new QLabel( tr ( "RSA/DSA key:" ), proxyBox);
    proxyKey=new QLineEdit(proxyBox);
    pbOpenProxyKey=new QPushButton (
        QIcon ( mainWindow->iconsPath ( "/16x16/file-open.png" ) ),
        QString::null,proxyBox );
    cbProxyAutologin=new QCheckBox(tr("ssh-agent or default ssh key"),proxyBox);
    cbProxyKrbLogin=new QCheckBox(tr("Kerberos 5 (GSSAPI) authentication"),proxyBox);


    proxyLaout->addWidget(new QLabel(tr("Type:"),proxyBox),0,0,1,2);
    proxyLaout->addWidget(rbSshProxy,1,0,1,2);
    proxyLaout->addWidget(rbHttpProxy,2,0,1,2);
    proxyLaout->addWidget(new QLabel(tr("Host:"),proxyBox),3,0);
    proxyLaout->addWidget(new QLabel(tr("Port:"),proxyBox),4,0);
    proxyLaout->addWidget(proxyHost,3,1);
    proxyLaout->addWidget(proxyPort,4,1);

    proxyLaout->addWidget(cbProxySameUser,0,3,1,3);
    proxyLaout->addWidget(lProxyLogin=new QLabel(tr("Login:"),proxyBox),1,3,1,1);
    proxyLaout->addWidget(proxyLogin,1,4,1,2);
    proxyLaout->addWidget(cbProxySamePass,2,3,1,3);
    proxyLaout->addWidget(proxyKeyLabel,3,3,1,1);
    proxyLaout->addWidget(proxyKey,3,4,1,1);
    proxyLaout->addWidget(pbOpenProxyKey,3,5,1,1);
    proxyLaout->addWidget(cbProxyAutologin,4,3,1,3);
    proxyLaout->addWidget(cbProxyKrbLogin,5,3,1,3);


#ifndef Q_WS_HILDON
    QGroupBox *deskSess=new QGroupBox ( tr ( "&Session type" ),this );
    QGridLayout* cmdLay=new QGridLayout ( deskSess );
#else
    QFrame* deskSess=this;
    QHBoxLayout* cmdLay=new QHBoxLayout ();
    cmdLay->addWidget ( new QLabel ( tr ( "Session type:" ),this ) );
#endif
    sessBox=new QComboBox ( deskSess );
    cmd=new QLineEdit ( deskSess );
    cmdCombo=new QComboBox ( deskSess );
    cmdCombo->setEditable ( true );
    sessBox->addItem ( "KDE" );
    sessBox->addItem ( "GNOME" );
    sessBox->addItem ( "LXDE" );
    sessBox->addItem ( "XFCE" );
    sessBox->addItem ( "MATE" );
    sessBox->addItem ( "UNITY" );
    sessBox->addItem ( "CINNAMON" );
    sessBox->addItem ( "TRINITY" );
    sessBox->addItem ( "OPENBOX" );
    sessBox->addItem ( "ICEWM" );
    sessBox->addItem ( tr ( "Connect to Windows terminal server" ) );
    sessBox->addItem ( tr ( "XDMCP" ) );
    sessBox->addItem ( tr ( "Connect to local desktop" ) );
    sessBox->addItem ( tr ( "Custom desktop" ) );
    sessBox->addItem ( tr ( "Single application" ) );
    sessBox->addItem ( tr ( "Published applications" ) );
    cmdLay->addWidget ( sessBox,0,1,Qt::AlignLeft );
    leCmdIp=new QLabel ( tr ( "Command:" ),deskSess );
    pbAdvanced=new QPushButton ( tr ( "Advanced options..." ),deskSess );
    cmdLay->addWidget ( leCmdIp,0,2 );
    cmdLay->setColumnStretch(6,1);
    cmdLay->addWidget ( cmd ,0,3);
    cmdLay->addWidget ( cmdCombo,0,4 );
    cmdLay->addWidget ( pbAdvanced ,0,5);
    cmdCombo->setSizePolicy ( QSizePolicy::Expanding,
                              QSizePolicy::Preferred );
    cmdCombo->hide();
    pbAdvanced->hide();
    cmdCombo->addItem ( "" );
    cmdCombo->addItems ( mainWindow->transApplicationsNames() );
    cmdCombo->lineEdit()->setText ( tr ( "Path to executable" ) );
    cmdCombo->lineEdit()->selectAll();
#ifndef Q_WS_HILDON
    sessLay->addLayout ( slay );
    sessLay->addLayout ( ilay );
    sessLay->addLayout ( pathLay );
    if ( !miniMode )
        sessLay->addSpacing ( 15 );
    sessLay->addWidget ( sgb );
    sessLay->addWidget ( deskSess );
#ifdef Q_OS_LINUX
    cbDirectRDP=new QCheckBox(tr("Direct RDP Connection"), deskSess);
    cmdLay->addWidget(cbDirectRDP,1,0,1,6);
    cbDirectRDP->hide();
    connect(cbDirectRDP,SIGNAL(clicked()), this, SLOT(slot_rdpDirectClicked()));
#endif

#else
    QVBoxLayout* sHildILay = new QVBoxLayout();
    sHildILay->addLayout ( slay );
    sHildILay->addLayout ( ilay );
    sHildILay->addStretch();
    QHBoxLayout* sHildLay = new QHBoxLayout();
    sHildLay->addLayout ( sHildILay );

    QFrame* vl=new QFrame;
    vl->setLineWidth ( 0 );
    vl->setFrameStyle ( QFrame::VLine|QFrame::Plain );
    sHildLay->addWidget ( vl );
    sHildLay->setSpacing ( 6 );
    sHildLay->addLayout ( sgbLay );
    sessLay->addLayout ( sHildLay );
    sessLay->addStretch();
    sessLay->addLayout ( cmdLay );
#endif
    sessLay->addStretch();

    connect ( icon,SIGNAL ( clicked() ),this,SLOT ( slot_getIcon() ) );
    connect ( openKey,SIGNAL ( clicked() ),this,SLOT ( slot_getKey() ) );
    connect ( pbAdvanced,SIGNAL ( clicked() ),this,
              SLOT ( slot_rdpOptions() ) );
    connect ( sessBox,SIGNAL ( activated ( int ) ),this,
              SLOT ( slot_changeCmd ( int ) ) );
    connect ( sessName,SIGNAL ( textChanged ( const QString & ) ),this,
              SIGNAL ( nameChanged ( const QString & ) ) );
    connect (server, SIGNAL(textChanged(const QString&)),this, SLOT(slot_emitSettings()));
    connect (uname, SIGNAL(textChanged(const QString&)),this, SLOT(slot_emitSettings()));
    connect (cbKrbLogin, SIGNAL(clicked(bool)), this, SLOT(slot_krbChecked()));
#ifdef Q_OS_LINUX
    connect (rdpPort, SIGNAL(valueChanged(int)),this, SLOT(slot_emitSettings()));
#endif

    connect ( pbOpenProxyKey,SIGNAL ( clicked() ),this,SLOT ( slot_proxyGetKey()) );
    connect ( proxyType, SIGNAL ( buttonClicked(int)) ,this,SLOT ( slot_proxyType()));
    connect (cbProxy, SIGNAL(clicked(bool)), this, SLOT(slot_proxyOptions()));
    connect (cbProxySameUser, SIGNAL(clicked(bool)), this, SLOT(slot_proxySameLogin()));
    connect ( pathButton, SIGNAL(clicked(bool)), this, SLOT(slot_openFolder()));

    readConfig();
}


SessionWidget::~SessionWidget()
{
}

void SessionWidget::slot_proxyGetKey()
{
    QString path;
    QString startDir=ONMainWindow::getHomeDirectory();
#ifdef Q_OS_WIN
    if ( ONMainWindow::getPortable() &&
            ONMainWindow::U3DevicePath().length() >0 )
    {
        startDir=ONMainWindow::U3DevicePath() +"/";
    }
#endif
    path = QFileDialog::getOpenFileName (
               this,
               tr ( "Open key file" ),
               startDir,
               tr ( "All files" ) +" (*)" );
    if ( path!=QString::null )
    {
#ifdef Q_OS_WIN
        if ( ONMainWindow::getPortable() &&
                ONMainWindow::U3DevicePath().length() >0 )
        {
            if ( path.indexOf ( ONMainWindow::U3DevicePath() ) !=0 )
            {
                QMessageBox::critical (
                    0l,tr ( "Error" ),
                    tr ( "x2goclient is running in "
                         "portable mode. You should "
                         "use a path on your usb device "
                         "to be able to access your data "
                         "whereever you are" ),
                    QMessageBox::Ok,QMessageBox::NoButton );
                slot_getKey();
                return;
            }
            path.replace ( ONMainWindow::U3DevicePath(),
                           "(U3)" );
        }
#endif
        proxyKey->setText ( path );
    }

}

void SessionWidget::slot_proxyOptions()
{
    proxyBox->setVisible(cbProxy->isChecked() && cbProxy->isVisible());
}

void SessionWidget::slot_proxySameLogin()
{
    lProxyLogin->setDisabled(cbProxySameUser->isChecked());
    proxyLogin->setDisabled(cbProxySameUser->isChecked());
}

void SessionWidget::slot_proxyType()
{
    bool isSsh=rbSshProxy->isChecked();
    cbProxyAutologin->setVisible(isSsh);
    cbProxyKrbLogin->setVisible(isSsh);
    proxyKey->setVisible(isSsh);
    proxyKeyLabel->setVisible(isSsh);
    pbOpenProxyKey->setVisible(isSsh);
}


#ifdef Q_OS_LINUX
void SessionWidget::slot_rdpDirectClicked()
{
    bool isDirectRDP=cbDirectRDP->isChecked();
    if (cbDirectRDP->isHidden())
        isDirectRDP=false;
    pbAdvanced->setVisible((!isDirectRDP) && (sessBox->currentIndex()==RDP));
    leCmdIp->setVisible(!isDirectRDP);
    cmd->setVisible(!isDirectRDP);
    key->setVisible(!isDirectRDP);
    cbAutoLogin->setVisible(!isDirectRDP);
    lKey->setVisible(!isDirectRDP);
    openKey->setVisible(!isDirectRDP);
    sshPort->setVisible(!isDirectRDP);
    rdpPort->setVisible(isDirectRDP);
    cbKrbDelegation->setVisible(!isDirectRDP);
    cbKrbLogin->setVisible(!isDirectRDP);

    cbProxy->setVisible(!isDirectRDP);
    proxyBox->setVisible(!isDirectRDP && cbProxy->isChecked());

    if (isDirectRDP)
    {
        lPort->setText(tr("RDP port:"));
    }
    else
    {
        lPort->setText(tr("SSH port:"));
    }

    emit directRDP(isDirectRDP);
    slot_emitSettings();
}
#endif

void SessionWidget::slot_getIcon()
{
    QString path= QFileDialog::getOpenFileName (
                      this,
                      tr ( "Open picture" ),
                      QDir::homePath(),
                      tr ( "Pictures" ) +" (*.png *.xpm *.jpg)" );
    if ( path!=QString::null )
    {
        sessIcon = wrap_legacy_resource_URIs (path);
        icon->setIcon ( QIcon ( sessIcon ) );
    }
}

void SessionWidget::slot_getKey()
{
    QString path;
    QString startDir=ONMainWindow::getHomeDirectory();
#ifdef Q_OS_WIN
    if ( ONMainWindow::getPortable() &&
            ONMainWindow::U3DevicePath().length() >0 )
    {
        startDir=ONMainWindow::U3DevicePath() +"/";
    }
#endif
    path = QFileDialog::getOpenFileName (
               this,
               tr ( "Open key file" ),
               startDir,
               tr ( "All files" ) +" (*)" );
    if ( path!=QString::null )
    {
#ifdef Q_OS_WIN
        if ( ONMainWindow::getPortable() &&
                ONMainWindow::U3DevicePath().length() >0 )
        {
            if ( path.indexOf ( ONMainWindow::U3DevicePath() ) !=0 )
            {
                QMessageBox::critical (
                    0l,tr ( "Error" ),
                    tr ( "x2goclient is running in "
                         "portable mode. You should "
                         "use a path on your usb device "
                         "to be able to access your data "
                         "whereever you are" ),
                    QMessageBox::Ok,QMessageBox::NoButton );
                slot_getKey();
                return;
            }
            path.replace ( ONMainWindow::U3DevicePath(),
                           "(U3)" );
        }
#endif
        key->setText ( path );
    }
}


void SessionWidget::slot_changeCmd ( int var )
{
    leCmdIp->setText ( tr ( "Command:" ) );
    pbAdvanced->hide();
#ifdef Q_OS_LINUX
    cbDirectRDP->hide();
#endif
    leCmdIp->show();
    cmd->show();
    if ( var==APPLICATION )
    {
        cmd->hide();
        cmdCombo->setVisible ( true );
        cmdCombo->setEnabled(true);
        cmdCombo->lineEdit()->selectAll();
        cmdCombo->lineEdit()->setFocus();
    }
    else
    {
        cmdCombo->hide();
        cmd->setVisible ( true );
        if ( var==OTHER || var == RDP || var == XDMCP )
        {
            cmd->setText ( "" );
            cmd->setEnabled ( true );
            cmd->selectAll();
            cmd->setFocus();
            if ( var==RDP )
            {
                leCmdIp->setText ( tr ( "Server:" ) );
                pbAdvanced->show();
                cmd->setText ( rdpServer );
#ifdef Q_OS_LINUX
                cbDirectRDP->show();
#endif
            }
            if ( var== XDMCP )
            {
                leCmdIp->setText ( tr ( "XDMCP server:" ) );
                cmd->setText ( xdmcpServer );
            }
        }
        else
        {
            cmd->setEnabled ( false );
            cmd->setText ( "" );
        }
    }
#ifdef Q_OS_LINUX
    slot_rdpDirectClicked();
#endif
}

void SessionWidget::slot_rdpOptions()
{
    bool ok;
    QString text = QInputDialog::getText (
                       this,
                       tr ( "Connect to Windows terminal server" ),
                       tr ( "rdesktop command line options:" ),
                       QLineEdit::Normal,
                       rdpOptions, &ok );
    rdpOptions= text;
}

void SessionWidget::readConfig()
{

    X2goSettings st ( "sessions" );
    QString name=st.setting()->value (
                     sessionId+"/name",
                     ( QVariant ) tr ( "New session" ) ).toString().trimmed();

    QStringList tails=name.split("/",QString::SkipEmptyParts);
    QString path;
    if(tails.count()>0)
    {
        name=tails.last();
        tails.pop_back();
        path=tails.join("/")+"/";
    }
    lPath->setText(path);
    if(newSession)
        lPath->setText(parent->getSessionExplorer()->getCurrentPath()+"/");

    sessName->setText (name);

    sessIcon = wrap_legacy_resource_URIs (st.setting ()->value (sessionId+"/icon",
                                                                (QVariant) ":/img/icons/128x128/x2gosession.png"
                                                               ).toString ().trimmed ());
    sessIcon = expandHome (sessIcon);
    icon->setIcon ( QIcon ( sessIcon ) );

    server->setText ( st.setting()->value (
                          sessionId+"/host",
                          ( QVariant ) QString::null ).toString().trimmed() );
    uname->setText ( st.setting()->value (
                         sessionId+"/user",
                         ( QVariant ) QString::null ).toString().trimmed() );
    key->setText ( st.setting()->value (
                       sessionId+"/key",
                       ( QVariant ) QString::null ).toString().trimmed() );
    cbAutoLogin->setChecked(st.setting()->value (
                                sessionId+"/autologin",
                                ( QVariant ) false ).toBool());
    cbKrbLogin->setChecked(st.setting()->value (
                               sessionId+"/krblogin",
                               ( QVariant ) false ).toBool());
    cbKrbDelegation->setChecked(st.setting()->value (
                                    sessionId+"/krbdelegation",
                                    ( QVariant ) false ).toBool());
    sshPort->setValue (
        st.setting()->value (
            sessionId+"/sshport",
            ( QVariant ) mainWindow->getDefaultSshPort().toInt()
        ).toInt() );
#ifdef Q_OS_LINUX
    rdpPort->setValue (
        st.setting()->value (
            sessionId+"/rdpport",3389
        ).toInt() );
#endif


    cbProxy->setChecked(st.setting()->value (
                            sessionId+"/usesshproxy",
                            false
                        ).toBool() );

    QString prtype= st.setting()->value (
                        sessionId+"/sshproxytype",
                        "SSH"
                    ).toString().trimmed() ;

    if(prtype=="HTTP")
    {
        rbHttpProxy->setChecked(true);
    }
    else
    {
        rbSshProxy->setChecked(true);
    }

    proxyLogin->setText(st.setting()->value (
                            sessionId+"/sshproxyuser",
                            QString()
                        ).toString().trimmed() );

    proxyKey->setText(st.setting()->value (
                          sessionId+"/sshproxykeyfile",
                          QString()
                      ).toString().trimmed() );

    proxyHost->setText(st.setting()->value (
                           sessionId+"/sshproxyhost",
                           QString()
                       ).toString().trimmed() );

    proxyPort->setValue(st.setting()->value (
                            sessionId+"/sshproxyport",
                            22
                        ).toInt() );

    cbProxySamePass->setChecked(st.setting()->value (
                                    sessionId+"/sshproxysamepass",
                                    false
                                ).toBool() );
    cbProxySameUser->setChecked(st.setting()->value (
                                    sessionId+"/sshproxysameuser",
                                    false
                                ).toBool() );
    cbProxyAutologin->setChecked(st.setting()->value (
                                     sessionId+"/sshproxyautologin",
                                     false
                                 ).toBool() );
    cbProxyKrbLogin->setChecked(st.setting()->value (
                                    sessionId+"/sshproxykrblogin",
                                    false
                                ).toBool() );

    if(proxyHost->text().indexOf(":")!=-1)
    {
        QStringList parts=proxyHost->text().split(":");
        proxyHost->setText(parts[0]);
        proxyPort->setValue(parts[1].toInt());
    }


    QTimer::singleShot(1, this,SLOT(slot_proxySameLogin()));
    QTimer::singleShot(2, this,SLOT(slot_proxyType()));
    QTimer::singleShot(3, this,SLOT(slot_proxyOptions()));


    QStringList appNames=st.setting()->value (
                             sessionId+"/applications" ).toStringList();
    bool rootless=st.setting()->value (
                      sessionId+"/rootless",false ).toBool();
    bool published=st.setting()->value (
                       sessionId+"/published",false ).toBool();

    QString
    command=st.setting()->value (
                sessionId+"/command",
                ( QVariant ) mainWindow->getDefaultCmd() ).toString().trimmed();

    rdpOptions=st.setting()->value ( sessionId+"/rdpoptions",
                                     ( QVariant ) "" ).toString().trimmed();
    rdpServer=st.setting()->value ( sessionId+"/rdpserver",
                                    ( QVariant ) "" ).toString().trimmed();
    xdmcpServer=st.setting()->value ( sessionId+"/xdmcpserver",
                                      ( QVariant ) "localhost" ).toString().trimmed();
#ifdef Q_OS_LINUX
    cbDirectRDP->setChecked(st.setting()->value (
                                sessionId+"/directrdp",false ).toBool());
#endif

    for ( int i=0; i<appNames.count(); ++i )
    {
        QString app=mainWindow->transAppName ( appNames[i] );
        if ( cmdCombo->findText ( app ) ==-1 )
            cmdCombo->addItem ( app );
    }
    if ( published )
    {
        sessBox->setCurrentIndex( PUBLISHED );
        cmdCombo->setDisabled(true);
        slot_changeCmd(PUBLISHED);
    }
    else if ( rootless )
    {
        sessBox->setCurrentIndex ( APPLICATION );
        QString app=mainWindow->transAppName ( command );
        cmdCombo->lineEdit()->setText ( app );
        slot_changeCmd ( APPLICATION );
    }
    else
    {
        if ( command=="KDE" )
        {
            sessBox->setCurrentIndex ( KDE );
            cmd->setEnabled ( false );
        }
        else if ( command=="GNOME" )
        {
            sessBox->setCurrentIndex ( GNOME );
            cmd->setEnabled ( false );
        }
        else if ( command=="LXDE" )
        {
            sessBox->setCurrentIndex ( LXDE );
            cmd->setEnabled ( false );
        }
        else if ( command=="UNITY" )
        {
            sessBox->setCurrentIndex ( UNITY );
            cmd->setEnabled ( false );
        }
        else if ( command=="XFCE" )
        {
            sessBox->setCurrentIndex ( XFCE );
            cmd->setEnabled ( false );
        }
        else if ( command=="SHADOW" )
        {
            sessBox->setCurrentIndex ( SHADOW );
            cmd->setEnabled ( false );
        }
        else if ( command=="RDP" )
        {
            leCmdIp->setText ( tr ( "Server:" ) );
            sessBox->setCurrentIndex ( RDP );
            cmd->setEnabled ( true );
            cmd->setText ( rdpServer );
            pbAdvanced->show();
#ifdef Q_OS_LINUX
            cbDirectRDP->show();
            slot_rdpDirectClicked();
#endif
        }
        else if ( command=="XDMCP" )
        {
            leCmdIp->setText ( tr ( "XDMCP server:" ) );
            sessBox->setCurrentIndex ( XDMCP );
            cmd->setEnabled ( true );
            cmd->setText ( xdmcpServer );
        }
        else
        {
            cmd->setText ( command );
            sessBox->setCurrentIndex ( OTHER );
            cmd->setEnabled ( true );
        }
    }
    if ( sessName->text() ==tr ( "New session" ) )
    {
        sessName->selectAll();
        sessName->setFocus();
    }
#ifdef Q_OS_LINUX
    slot_rdpDirectClicked();
#endif
    slot_krbChecked();
}

void SessionWidget::setDefaults()
{
    cmd->setText ( "" );
    sessBox->setCurrentIndex ( KDE );
    cmdCombo->clear();
    cmdCombo->addItem ( "" );
    cmdCombo->addItems ( mainWindow->transApplicationsNames() );
    cbAutoLogin->setChecked(false);
    cbKrbLogin->setChecked(false);
    cmdCombo->lineEdit()->setText (

        tr ( "Path to executable" ) );
    cmdCombo->lineEdit()->selectAll();
    slot_changeCmd ( 0 );
    cmd->setEnabled ( false );
    sessIcon=":/img/icons/128x128/x2gosession.png";
    icon->setIcon ( QIcon ( sessIcon ) );
    sshPort->setValue (
        mainWindow->getDefaultSshPort().toInt() );
#ifdef Q_OS_LINUX
    rdpPort->setValue (3389);
#endif



    cbProxy->setChecked(false);

    rbSshProxy->setChecked(true);


    proxyKey->setText(QString::null);


    proxyPort->setValue(22);

    cbProxySamePass->setChecked(false);
    cbProxySameUser->setChecked(false);
    cbProxyAutologin->setChecked(false);
    cbProxyKrbLogin->setChecked(false);

    QTimer::singleShot(1, this,SLOT(slot_proxySameLogin()));
    QTimer::singleShot(2, this,SLOT(slot_proxyType()));
    QTimer::singleShot(3, this,SLOT(slot_proxyOptions()));

}


void SessionWidget::saveSettings()
{

    X2goSettings st ( "sessions" );

    QString normPath=(lPath->text()+"/"+sessName->text()).split("/",QString::SkipEmptyParts).join("/");

    st.setting()->setValue ( sessionId+"/name",
                             ( QVariant ) normPath.trimmed() );
    st.setting()->setValue ( sessionId+"/icon",
                             ( QVariant ) sessIcon );
    st.setting()->setValue ( sessionId+"/host",
                             ( QVariant ) server->text().trimmed() );
    st.setting()->setValue ( sessionId+"/user",
                             ( QVariant ) uname->text().trimmed() );

    st.setting()->setValue ( sessionId+"/key",
                             ( QVariant ) key->text().trimmed() );
#ifdef Q_OS_LINUX
    st.setting()->setValue ( sessionId+"/rdpport",
                             ( QVariant ) rdpPort->value() );
#endif
    st.setting()->setValue ( sessionId+"/sshport",
                             ( QVariant ) sshPort->value() );
    st.setting()->setValue(sessionId+"/autologin",( QVariant ) cbAutoLogin->isChecked());
    st.setting()->setValue(sessionId+"/krblogin",( QVariant ) cbKrbLogin->isChecked());
    st.setting()->setValue(sessionId+"/krbdelegation",( QVariant ) cbKrbDelegation->isChecked());
#ifdef Q_OS_LINUX
    st.setting()->setValue(sessionId+"/directrdp",( QVariant ) cbDirectRDP->isChecked());
#endif
    QString command;
    bool rootless=false;
    bool published=false;


    if ( sessBox->currentIndex() < OTHER )
        command=sessBox->currentText();
    else
        command=cmd->text().trimmed();
    if ( sessBox->currentIndex() == RDP )
    {
        command="RDP";
        rdpServer=cmd->text().trimmed();
    }
    if ( sessBox->currentIndex() == XDMCP )
    {
        command="XDMCP";
        xdmcpServer=cmd->text().trimmed();
    }
    if ( sessBox->currentIndex() == SHADOW )
    {
        command="SHADOW";
    }

    QStringList appList;
    for ( int i=-1; i<cmdCombo->count(); ++i )
    {
        QString app;
        if ( i==-1 )
            app=mainWindow->internAppName (
                    cmdCombo->lineEdit()->text () );
        else
            app=mainWindow->internAppName ( cmdCombo->itemText ( i ) );
        if ( appList.indexOf ( app ) ==-1 && app!="" &&
                app!=tr ( "Path to executable" ) )
        {
            appList.append ( app );
        }
    }
    if ( sessBox->currentIndex() ==APPLICATION )
    {
        rootless=true;
        command=mainWindow->internAppName ( cmdCombo->lineEdit()->text().trimmed() );
    }
    if ( sessBox->currentIndex() == PUBLISHED)
        published=true;

    st.setting()->setValue ( sessionId+"/rootless", ( QVariant ) rootless );
    st.setting()->setValue ( sessionId+"/published", ( QVariant ) published );
    st.setting()->setValue ( sessionId+"/applications",
                             ( QVariant ) appList );
    st.setting()->setValue ( sessionId+"/command",
                             ( QVariant ) command );
    st.setting()->setValue ( sessionId+"/rdpoptions",
                             ( QVariant ) rdpOptions );
    st.setting()->setValue ( sessionId+"/rdpserver",
                             ( QVariant ) rdpServer );
    st.setting()->setValue ( sessionId+"/xdmcpserver",
                             ( QVariant ) xdmcpServer );


    st.setting()->setValue (
        sessionId+"/usesshproxy",cbProxy->isChecked());


    if(rbHttpProxy->isChecked())
    {
        st.setting()->setValue ( sessionId+"/sshproxytype","HTTP");
    }
    else
    {
        st.setting()->setValue ( sessionId+"/sshproxytype","SSH");
    }
    st.setting()->setValue (sessionId+"/sshproxyuser",proxyLogin->text().trimmed());
    st.setting()->setValue (sessionId+"/sshproxykeyfile",proxyKey->text().trimmed());
    st.setting()->setValue (sessionId+"/sshproxyhost",proxyHost->text().trimmed());
    st.setting()->setValue (sessionId+"/sshproxyport",proxyPort->value());
    st.setting()->setValue (sessionId+"/sshproxysamepass",cbProxySamePass->isChecked());
    st.setting()->setValue (sessionId+"/sshproxysameuser",cbProxySameUser->isChecked());
    st.setting()->setValue (sessionId+"/sshproxyautologin",cbProxyAutologin->isChecked());
    st.setting()->setValue (sessionId+"/sshproxykrblogin",cbProxyKrbLogin->isChecked());

    st.setting()->sync();
}

QString SessionWidget::sessionName()
{
    return sessName->text();
}

#ifdef Q_OS_LINUX
void SessionWidget::slot_emitSettings()
{
    emit settingsChanged(server->text(), QString::number( rdpPort->value()), uname->text());
}
#endif

void SessionWidget::slot_krbChecked()
{
    cbKrbDelegation->setEnabled(cbKrbLogin->isChecked());
}

void SessionWidget::slot_openFolder()
{
    FolderExplorer explorer(lPath->text(), parent->getSessionExplorer(), this);
    if(explorer.exec()==QDialog::Accepted)
    {
        lPath->setText(explorer.getCurrentPath());
    }
}
