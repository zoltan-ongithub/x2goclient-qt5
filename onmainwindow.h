/***************************************************************************
 *   Copyright (C) 2005 by Oleksandr Shneyder   *
 *   oleksandr.shneyder@treuchtlingen.de   *
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
	int colorDepth;
	enum{DESKTOP,ROOTLESS,SHADOW} sessionType;
	QString command;
	void operator = ( const x2goSession& s );
};

class ONMainWindow : public QMainWindow
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
		ONMainWindow ( QWidget *parent = 0 );
		~ONMainWindow();
		QString iconsPath ( QString fname );
		const QList<SessionButton*> * getSessionsList()
		{
			return &sessions;
		}
		void startNewSession();
		void showSessionStatus();
		void suspendSession ( QString user,QString host,QString pass,QString key, QString sessId );
		void termSession ( QString user,QString host,QString pass,QString key, QString sessId );
		void setStatStatus ( const QString& status );
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
		QStringList internApplicationsNames() {return _internApplicationsNames;}
		QStringList transApplicationsNames() {return _transApplicationsNames;}
		QString transAppName ( const QString& internAppName, bool* found=0l );
		QString internAppName ( const QString& transAppName, bool* found=0l );

	private:
		QStringList _internApplicationsNames;
		QStringList _transApplicationsNames;
		bool drawMenu;
		bool extStarted;
		bool startMaximized;
		bool defaultUseSound;
		bool cardStarted;
		bool defaultSetKbd;
		bool showExport;
		bool usePGPCard;
		bool useEsd;
		bool miniMode;
		int defaultLink;
		int defaultQuality;
		int defaultWidth;
		int defaultHeight;
		bool defaultFullscreen;
		bool acceptRsa;
		bool extLogin;
		QString sshPort;
		QString clientSshPort;
		QString defaultSshPort;
		QString artsCmd;
		QString esdCmd;

		QString defaultPack;
		QString defaultLayout;
		QString defaultKbdType;
		QString defaultCmd;
		QStringList listedSessions;
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
		QLineEdit* pass;
		QLineEdit* login;
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
		QString sndport;
		QString selectedCommand;
		QString currentKey;
		QTimer *exportTimer;
		QTimer *extTimer;
		QTimer *agentCheckTimer;
		QStyle* widgetExtraStyle;
		QString qpass;
		bool isPassShown;
		bool xmodExecuted;


		QString sessionStatus;
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
		QProcess* gpg;
		LDAPSession* ld;
		bool useLdap;
		bool showToolBar;
		bool newSession;
		bool ldapOnly;
		bool isScDaemonOk;
		QString ldapServer;
		int ldapPort;
		QString ldapServer1;
		int ldapPort1;
		QString ldapServer2;
		int ldapPort2;
		QString ldapDn;
		QString sessionCmd;
		QString netSound;
		QAction *act_edit;
		QAction *act_new;
		QProcess *nxproxy;
		QProcess *artsd;
		QString lastFreeServer;
		QString cardLogin;
		QTextEdit* stInfo;
		SVGFrame* ln;
		sshProcess* tunnel;
		sshProcess* sndTunnel;
		QList<x2goSession> selectedSessions;
		x2goSession resumingSession;
		bool startSound;
		bool restartResume;
		int firstUid;
		int lastUid;
		QStringList sshEnv;
		QString agentPid;
		bool cardReady;
		void loadSettings();
		void showPass ( UserButton* user );
		void clean();
// 		void createTmpXconf ( QString );
// 		void nxrun ( QString );
// 		void startDirect ( QFile& );
// 		int numOfSessions ( QString );
		SessionButton* createBut ( const QString& id );
		void placeButtons();
// 		QString getFreeServer ( QString server=QString() );
		QString getKdeIconsPath();
		QString findTheme ( QString theme );
// 		void findSession();
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
		void startGPGAgent ( const QString& login, const QString& appId );

	protected:
		virtual void closeEvent ( QCloseEvent* event );

	private slots:
		void slot_showPassForm();
		void displayUsers();
		void slot_showWidgets();
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

	public slots:
		void slot_config();
		void slotNewSession();
		void slotDeleteButton ( SessionButton * bt );
		void slot_edit ( SessionButton* );
		void exportsEdit ( SessionButton* bt );
	private slots:
		void slotSnameChanged ( const QString& );
		void slotSelectedFromList ( SessionButton* session );
		void slotSessEnter();
		void slotCloseSelectDlg();
		void slot_showSelectSessionWidgets();
		void slot_activated ( const QModelIndex& index );
		void slotResumeSess();
		void slotSuspendSess();
		void slotTermSessFromSt();
		void slotSuspendSessFromSt();
		void slotTermSess();
		void slotNewSess();
		void slot_cmdMessage ( bool result,QString output,sshProcess* );
		void slot_listSessions ( bool result,QString output,sshProcess* );
		void slot_retSuspSess ( bool value,QString message,sshProcess* );
		void slot_retTermSess ( bool result,QString output,sshProcess* );
		void slot_retResumeSess ( bool result,QString output,sshProcess* );
		void slot_tunnelFailed ( bool result,QString output,sshProcess* );
		void slot_sndTunnelFailed ( bool result,QString output,sshProcess* );
		void slot_copyKey ( bool result,QString output,sshProcess* );
		void slot_tunnelOk();
		void slot_proxyerror ( QProcess::ProcessError err );
		void slot_proxyFinished ( int result,QProcess::ExitStatus st );
		void slot_proxyStderr();
		void slot_proxyStdout();
		void slot_resumeDoubleClick ( const QModelIndex& );
		void slotShowAdvancedStat();
		void slot_showStatusWidgets();
		void slot_restartNxProxy();
		void slot_testSessionStatus();
		void slot_retRunCommand ( bool result, QString output,sshProcess* );
		void slot_getServers ( bool result, QString output,sshProcess* );
		void slot_listAllSessions ( bool result,QString output,sshProcess* );
		void slot_retExportDir ( bool result,QString output,sshProcess* );
		void slot_resize();
		void slot_exportDirectory();
		void slot_exportTimer();
		void slot_about_qt();
		void slot_about();
	private slots:
		void slot_rereadUsers();
		void slotExtTimer();
		void slot_startPGPAuth();
		void slot_scDaemonOut();
		void slot_scDaemonError();
		void slot_gpgFinished ( int exitCode, QProcess::ExitStatus exitStatus );
		void slot_scDaemonFinished ( int exitCode, QProcess::ExitStatus exitStatus );
		void slot_gpgError();
		void slot_checkScDaemon();
		void slot_gpgAgentFinished ( int exitCode, QProcess::ExitStatus exitStatus );
		void slot_checkAgentProcess();
		void slot_execXmodmap();
		void slot_sudoErr ( QString stderr, sshProcess* proc );
	private:
		void cartReady();
	private:
		void addToAppNames ( QString intName, QString transName );
		bool checkAgentProcess();
		bool isColorDepthOk ( int disp, int sess );
		void check_cmd_status();
#ifdef WINDOWS
		QString transform2cygwinPath ( const QString& winPath, bool trunc=false );
		QString transform2winPath ( const QString& winPath );
		QString cygwinHomePath();
#endif
#if defined  (WINDOWS) || defined (Q_OS_DARWIN)
		QString getXDisplay();
#endif
};

#endif
