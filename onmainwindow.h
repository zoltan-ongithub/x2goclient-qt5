/***************************************************************************
 *   Copyright (C) 2005 by Oleksandr Shneyder   *
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
#ifndef ONMAINWINDOW_H
#define ONMAINWINDOW_H

#include "x2goclientconfig.h"
//#include "CallbackInterface.h"
#include <QMainWindow>
#include <QList>
#include <QPushButton>
#include <QPixmap>
#include <QProcess>
#include "LDAPSession.h"
#include "embedwidget.h"
#include <QToolBar>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
/**
@author Oleksandr Shneyder
*/

class QLineEdit;
class QFrame;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class UserButton;
class QTextEdit;
class SessionButton;
class QLabel;
class QProcess;
class QFile;
class SVGFrame;
class SessionButton;
class QAction;
class QCheckBox;
class QTreeView;
class QModelIndex;
class sshProcess;
class IMGFrame;
class QStandardItemModel;
struct user
{
	int uin;
	QString uid;
	QString name;
	QPixmap foto;
	static bool lessThen ( user u1,user u2 )
	{
		return u1.uid < u2.uid;
	}
};

struct directory
{
	QString key;
	QString dstKey;
	QString dirList;
	bool isRemovable;
	sshProcess* proc;
};

struct serv
{
	QString name;
	float factor;
	float sess;
	bool connOk;
	bool operator < ( const struct serv it )
	{
		return ( it.sess < sess );
	}
	static bool lt ( const struct serv it, const struct serv it1 )
	{
		return it.sess<it1.sess;
	}
};

struct x2goSession
{
	QString agentPid;
	QString sessionId;
	QString display;
	QString server;
	QString status;
	QString crTime;
	QString cookie;
	QString clientIp;
	QString grPort;
	QString sndPort;
	QString fsPort;
	int colorDepth;
	enum{DESKTOP,ROOTLESS,SHADOW} sessionType;
	QString command;
	void operator = ( const x2goSession& s );
};

struct ConfigFile
{
	QString session;
	QString server;
	QString user;
	QString sshport;
	QString command;
	bool rootless;
};
//wrapper to send mouse events under windows in embedded mode
#ifdef Q_OS_WIN
class WWrapper : public QPushButton
{
		friend class ONMainWindow;
};
#include <QThread>
#include <QMutex>
class ONMainWindow;
class WinServerStarter: public QThread
{
	public:
		enum daemon{X,SSH,PULSE};
		WinServerStarter ( daemon server, ONMainWindow * par );
		void run();
	private:
		daemon mode;
		ONMainWindow* parent;
};
#endif

class ClickLineEdit;
#ifndef Q_OS_DARWIN
class ONMainWindow : public QMainWindow, public EmbedWidget
#else
class ONMainWindow : public QMainWindow
#endif
{
		Q_OBJECT
	public:
		enum
		{
			S_DISPLAY,
			S_STATUS,
			S_COMMAND,
			S_TYPE,
			S_SERVER,
			S_CRTIME,
			S_IP,
			S_ID
		};
		enum
		{
			PULSE,
			ARTS,
			ESD
		};
		ONMainWindow ( QWidget *parent = 0 );
		~ONMainWindow();
		QString iconsPath ( QString fname );
		const QList<SessionButton*> * getSessionsList()
		{
			return &sessions;
		}
		void startNewSession();
		void suspendSession ( QString user,QString host,QString pass,
		                      QString key, QString sessId );
		bool termSession ( QString user,QString host,QString pass,
		                   QString key, QString sessId );
		void setStatStatus ( QString status=QString::null );
		x2goSession getNewSessionFromString ( const QString& string );
		void runCommand();
		bool retUseLdap() {return useLdap;}
		bool retMiniMode() {return miniMode;}
		QString retLdapServer() {return ldapServer;}
		int retLdapPort() {return ldapPort;}
		QString retLdapDn() {return ldapDn;}
		QString retLdapServer1() {return ldapServer1;}
		int retLdapPort1() {return ldapPort1;}
		QString retLdapServer2() {return ldapServer2;}
		int retLdapPort2() {return ldapPort2;}
		QHBoxLayout* mainLayout() {return mainL;}
		QWidget* mainWidget() {return ( QWidget* ) fr;}


		QString getDefaultCmd()
		{
			return defaultCmd;
		}
		QString getDefaultSshPort()
		{
			return defaultSshPort;
		}
		QString getDefaultKbdType()
		{
			return defaultKbdType;
		}
		QString getDefaultLayout()
		{
			return defaultLayout;
		}
		QString getDefaultPack()
		{
			return defaultPack;
		}
		int getDefaultQuality()
		{
			return defaultQuality;
		}

		uint getDefaultDPI()
		{
			return defaultDPI;
		}

		bool getDefaultSetDPI()
		{
			return defaultSetDPI;
		}

		bool getEmbedMode()
		{
			return embedMode;
		}

		int getDefaultLink()
		{
			return defaultLink;
		}
		int getDefaultWidth()
		{
			return defaultWidth;
		}
		int getDefaultHeight()
		{
			return defaultHeight;
		}
		bool getDefaultSetKbd()
		{
			return defaultSetKbd;
		}
		bool getDefaultUseSound()
		{
			return defaultUseSound;
		}
		bool getDefaultFullscreen()
		{
			return defaultFullscreen;
		}

		void showHelp();
		void showHelpPack();
		void exportDirs ( QString exports,bool removable=false );
		void reloadUsers();
		void setWidgetStyle ( QWidget* widget );
		QStringList internApplicationsNames()
		{
			return _internApplicationsNames;
		}
		QStringList transApplicationsNames()
		{
			return _transApplicationsNames;
		}
		QString transAppName ( const QString& internAppName,
		                       bool* found=0l );
		QString internAppName ( const QString& transAppName,
		                        bool* found=0l );
		void setEmbedSessionActionsEnabled ( bool enable );
#ifdef Q_OS_WIN
		static QString cygwinPath ( const QString& winPath );
		void startXOrg();
		void startSshd();
		void startPulsed();
#endif

	private:
		QStringList _internApplicationsNames;
		QStringList _transApplicationsNames;
		bool drawMenu;
		bool extStarted;
		bool startMaximized;
		bool startHidden;
		bool defaultUseSound;
		bool cardStarted;
		bool defaultSetKbd;
		bool showExport;
		bool usePGPCard;
		bool miniMode;
		bool embedMode;
		QString statusString;
		QString sessionConfigFile;
		int defaultLink;
		int defaultQuality;
		int defaultWidth;
		int defaultHeight;
		bool defaultFullscreen;
		bool acceptRsa;
		bool startEmbedded;
		bool extLogin;
		bool printSupport;
		bool showTbTooltip;
		QString sshPort;
		QString clientSshPort;
		QString defaultSshPort;

		QString defaultPack;
		QString defaultLayout;
		QString defaultKbdType;
		QString defaultCmd;
		bool defaultSetDPI;
		uint defaultDPI;
		QStringList listedSessions;
		QString appDir;
		QString localGraphicPort;
		QString homeDir;
		int retSessions;
		QList<serv> x2goServers;

		QPushButton* bSusp;
		QPushButton* sbExp;
		QPushButton* bTerm;
		QPushButton* bNew;

		QLabel* selectSessionLabel;
		QTreeView* sessTv;

		IMGFrame* fr;
		SVGFrame *bgFrame;
		QLineEdit* uname;
		ClickLineEdit* pass;
		ClickLineEdit* login;
		QFrame* uframe;
		SVGFrame *passForm;
		QSize mwSize;
		bool mwMax;
		QPoint mwPos;
		SVGFrame *selectSessionDlg;
		SVGFrame *sessionStatusDlg;
		QLabel* u;
		QLabel* fotoLabel;
		QLabel* nameLabel;
		QLabel* passPrompt;
		QLabel* loginPrompt;
		QLabel* slName;
		QLabel* slVal;
		QPushButton* ok;
		QPushButton* cancel;
		QString readExportsFrom;
		QString readLoginsFrom;
		QPushButton* sOk;
		QPushButton* sbSusp;
		QPushButton* sbTerm;
		QCheckBox* sbAdv;
		QPushButton* sCancel;
		QString resolution;
		QString kdeIconsPath;
		QScrollArea* users;
		QVBoxLayout* userl;
		QHBoxLayout* mainL;
		QList<UserButton*> names;
		QList<SessionButton*> sessions;
		UserButton* lastUser;
		SessionButton* lastSession;
		QString prevText;
		QString onserver;
		QString id;
		QString selectedCommand;
		QString currentKey;
		QTimer *exportTimer;
		QTimer *extTimer;
		QTimer *agentCheckTimer;
		QTimer *spoolTimer;
		QTimer *proxyWinTimer;
		QStyle* widgetExtraStyle;
		bool isPassShown;
		bool xmodExecuted;
		long proxyWinId;
		bool embedControlChanged;
		bool embedTbVisible;
		QLabel* statusLabel;
		ConfigFile config;
		QStandardItemModel* model;

		QAction *act_set;
		QAction *act_abclient;
		QAction *act_shareFolder;
		QAction *act_suspend;
		QAction *act_terminate;
		QAction *act_embedContol;
		QAction *act_embedToolBar;

		QToolBar *stb;


		QString sessionStatus;
		QString spoolDir;
		QString sessionRes;
		QHBoxLayout* username;
		QList <user> userList;
		QList <directory> exportDir;
		QString nick;
		QString nfsPort;
		QString mntPort;
		QProcess* ssh;
		QProcess* soundServer;
		QProcess* scDaemon;
		QProcess* gpgAgent;
		QString sshAgentPid;
		QProcess* gpg;
		LDAPSession* ld;
		long embedParent;
		long embedChild;
		bool proxyWinEmbedded;
		bool useLdap;
		bool showToolBar;
		bool newSession;
		bool ldapOnly;
		bool isScDaemonOk;
		bool parecTunnelOk;

		bool startSessSound;
		int startSessSndSystem;

		bool fsInTun;
		bool fsTunReady;

		QString fsExportKey;
		bool fsExportKeyReady;

		QString ldapServer;
		int ldapPort;
		QString ldapServer1;
		int ldapPort1;
		QString ldapServer2;
		int ldapPort2;
		QString ldapDn;
		QString sessionCmd;

		QString LDAPSndSys;
		QString LDAPSndPort;
		bool LDAPSndStartServer;

		bool  LDAPPrintSupport;

		QAction *act_edit;
		QAction *act_new;
		QAction *act_sessicon;
		QProcess *nxproxy;
#ifdef Q_OS_WIN
		QProcess *xorg;
		PROCESS_INFORMATION sshd;
		
		QProcess* pulseServer;
		int xDisplay;
		int sshdPort;
		bool winServersReady;
		QString oldEtcDir;
		QString oldBinDir;
		QString oldTmpDir;
		QString pulseDir;
		int pulsePort;
		int esdPort;
		bool maximizeProxyWin;
		int proxyWinWidth;
		int proxyWinHeight;
		QTimer* xorgLogTimer;
		QString xorgLogFile;
		QMutex xorgLogMutex;
#endif
		QString lastFreeServer;
		QString cardLogin;
		QTextEdit* stInfo;
		SVGFrame* ln;
		sshProcess* tunnel;
		sshProcess* sndTunnel;
		sshProcess* fsTunnel;
		QList<x2goSession> selectedSessions;
		x2goSession resumingSession;
		bool startSound;
		bool restartResume;
		int firstUid;
		int lastUid;
		QStringList sshEnv;
		QString agentPid;
		bool cardReady;
		bool useSshAgent;
		void loadSettings();
		void showPass ( UserButton* user );
		void clean();
		bool defaultSession;
		QString defaultSessionName;
		QString defaultSessionId;
		QString defaultUserName;
		bool defaultUser;
		SessionButton* createBut ( const QString& id );
		void placeButtons();
		QString getKdeIconsPath();
		QString findTheme ( QString theme );
		bool initLdapSession ( bool showBox=true );
		bool startSession ( const QString& id );
		x2goSession getSessionFromString ( const QString& string );
		void resumeSession ( const x2goSession& s );
		void selectSession ( const QStringList& sessions );
		x2goSession getSelectedSession();
		bool parseParam ( QString param );
		bool link_par ( QString value );
		bool geometry_par ( QString value );
		bool setKbd_par ( QString value );
		bool ldap_par ( QString value );
		bool ldap1_par ( QString value );
		bool ldap2_par ( QString value );
		bool pack_par ( QString value );
		bool sound_par ( QString val );
		void printError ( QString param );
		void exportDefaultDirs();
		QString createRSAKey();
		directory* getExpDir ( QString key );
		bool findInList ( const QString& uid );
		void setUsersEnabled ( bool enable );
		void externalLogout ( const QString& logoutDir );
		void externalLogin ( const QString& loginDir );
		void startGPGAgent ( const QString& login,
		                     const QString& appId );

	protected:
		virtual void closeEvent ( QCloseEvent* event );
#ifndef Q_OS_WIN
		virtual void mouseReleaseEvent ( QMouseEvent * event );
#else
	private slots:
		void slotSetWinServersReady();
		void startWinServers();
		void slotCheckXOrgLog();
#endif
	private slots:
		void slot_showPassForm();
		void displayUsers();
		void slot_resize ( const QSize sz );
		void slotUnameChanged ( const QString& text );
		void slotPassEnter();

		void readUsers();
		void slotSelectedFromList ( UserButton* user );
		void slotUnameEntered();
		void slotClosePass();
		void slot_readSessions();
		void slot_manage();
		void displayToolBar ( bool );
		void showSessionStatus();

	public slots:
		void slot_config();
		void slotNewSession();
		void slotDeleteButton ( SessionButton * bt );
		void slot_edit ( SessionButton* );
		void slot_createDesktopIcon ( SessionButton* bt );
		void exportsEdit ( SessionButton* bt );
		void slotUpdateEmbed();
		void slotEmbedControlAction();
		void slotDetachProxyWindow();
		void slotActivateWindow();

	private slots:
		void slotSnameChanged ( const QString& );
		void slotSelectedFromList ( SessionButton* session );
		void slotSessEnter();
		void slotCloseSelectDlg();
		void slot_activated ( const QModelIndex& index );
		void slotResumeSess();
		void slotSuspendSess();
		void slotTermSessFromSt();
		void slotSuspendSessFromSt();
		void slotTermSess();
		void slotNewSess();
		void slot_cmdMessage ( bool result,QString output,
		                       sshProcess* );
		void slot_listSessions ( bool result,QString output,
		                         sshProcess* );
		void slot_retSuspSess ( bool value,QString message,
		                        sshProcess* );
		void slot_retTermSess ( bool result,QString output,
		                        sshProcess* );
		void slot_retResumeSess ( bool result,QString output,
		                          sshProcess* );
		void slot_tunnelFailed ( bool result,QString output,
		                         sshProcess* );
		void slot_fsTunnelFailed ( bool result,QString output,
		                           sshProcess* );
		void slot_sndTunnelFailed ( bool result,QString output,
		                            sshProcess* );
		void slot_copyKey ( bool result,QString output,sshProcess* );
		void slot_tunnelOk();
		void slot_fsTunnelOk();
		void slot_proxyerror ( QProcess::ProcessError err );
		void slot_proxyFinished ( int result,QProcess::ExitStatus st );
		void slot_proxyStderr();
		void slot_proxyStdout();
		void slot_resumeDoubleClick ( const QModelIndex& );
		void slotShowAdvancedStat();
		void slot_restartNxProxy();
		void slot_testSessionStatus();
		void slot_retRunCommand ( bool result, QString output,
		                          sshProcess* );
		void slot_getServers ( bool result, QString output,
		                       sshProcess* );
		void slot_listAllSessions ( bool result,QString output,
		                            sshProcess* );
		void slot_retExportDir ( bool result,QString output,
		                         sshProcess* );
		void slot_resize();
		void slot_exportDirectory();
		void slot_exportTimer();
		void slot_about_qt();
		void slot_about();
	private slots:
		void slot_checkPrintSpool();
		void slot_rereadUsers();
		void slotExtTimer();
		void slot_startPGPAuth();
		void slot_scDaemonOut();
		void slot_scDaemonError();
		void slot_gpgFinished ( int exitCode,
		                        QProcess::ExitStatus exitStatus );
		void slot_scDaemonFinished ( int exitCode,
		                             QProcess::ExitStatus exitStatus );
		void slot_gpgError();
		void slot_checkScDaemon();
		void slot_gpgAgentFinished ( int exitCode,
		                             QProcess::ExitStatus exitStatus );
		void slot_checkAgentProcess();
		void slot_execXmodmap();
		void slot_sudoErr ( QString errstr, sshProcess* pr );
		void slotCreateSessionIcon();
		void slotFindProxyWin();
		void slotAttachProxyWindow();
		void slotEmbedIntoParentWindow();
		void slotEmbedWindow();
		void slotStartParec ();
		void slotSndTunOk();
		void slotPCookieReady (	bool result,QString output,sshProcess* proc );
		void slotEmbedToolBar();
		void slotEmbedToolBarToolTip();
		void slotHideEmbedToolBarToolTip();
	private:
		void addToAppNames ( QString intName, QString transName );
		bool checkAgentProcess();
		bool isColorDepthOk ( int disp, int sess );
		void check_cmd_status();
		int startSshFsTunnel();
		void startX2goMount();
		void cleanPrintSpool();
		void cleanAskPass();
		void initWidgetsEmbed();
		void initWidgetsNormal();
		QString getCurrentUname();
		QString getCurrentPass();
		void processSessionConfig();
		void startSshAgent();
		void addKey2SshAgent();
		void finishSshAgent();
		void processCfgLine ( QString line );
		void initSelectSessDlg();
		void initStatusDlg();
		void initPassDlg();
		void printSshDError();
		void loadPulseModuleNativeProtocol();
		void initEmbedToolBar();
		bool isServerRunning ( int port );
#ifdef Q_OS_WIN
		void generateHostDsaKey();
		void generateEtcFiles();
		void saveCygnusSettings();
		void restoreCygnusSettings();
#endif
#if defined  (Q_OS_WIN) || defined (Q_OS_DARWIN)
		QString getXDisplay();
#endif
};

#endif
