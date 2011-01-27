//
// C++ Interface: configdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H
#include "x2goclientconfig.h"

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QRadioButton;
class QButtonGroup;
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/
class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    enum XServers{XMING,CYGWIN,CUSTOM};
    ConfigDialog(QWidget * parent, Qt::WFlags f = 0 );
    ~ConfigDialog();
#ifdef Q_OS_DARWIN
static    QString findXDarwin(QString& version, QString path="");
static    QString retMaxXDarwinVersion(QString v1, QString v2);
static    QString getXDarwinDirectory();
	  void    printXDarwinVersionWarning(QString version);
#endif
#ifdef WINDOWS    
static    void getXming(bool* found, QString* execName,QString* execDir, QString* options);
static    QString getCygwinDir(const QString& dir);
static    void getCygwin(bool* found, QString* execName,QString* execDir, QString* options);
static    void getXSettings(uint* display, QString* execName,QString* execDir, QString* options);
#endif

private:
    QCheckBox* useldap;
    QLineEdit* ldapBase;
    QLineEdit* ldapServer;
    QSpinBox*  port;
    QLineEdit* ldapServer1;
    QSpinBox*  port1;
    QLineEdit* ldapServer2;
    QSpinBox*  port2;
    QSpinBox*  clientSshPort;
    QPushButton* ok;
    QLineEdit* leXexec;
    QLineEdit* leCmdOpt;
    QSpinBox* sbDisp;
    QLineEdit* leXexecDir;
    QRadioButton* rbX[3];
    QPushButton* pbOpenExec;
    QButtonGroup* bgRadio;

public slots:
    void slot_accepted();
    void slot_checkOkStat();
private slots:
#ifdef WINDOWS	
    void slotDefaultXSettings();
    void slotGetExecDir();
    void slotGetExec();
    void slotXSelected(int id);
    void slotDispChanged(const QString& val);
#endif
#ifdef Q_OS_DARWIN
    void slot_selectXDarwin();
    void slot_findXDarwin();
#endif
};

#endif
