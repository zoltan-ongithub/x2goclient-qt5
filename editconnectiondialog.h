//
// C++ Interface: editconnectiondialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef EDITCONNECTIONDIALOG_H
#define EDITCONNECTIONDIALOG_H

#include "x2goclientconfig.h"
#include <QStringList>
#include <QDialog>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QRadioButton;
class QSlider;
class QLabel;
class QTabWidget;
class ONMainWindow;
class QStandardItemModel;
class QTreeView;

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/
class EditConnectionDialog : public QDialog
{
    Q_OBJECT
public:
    EditConnectionDialog(QString id, QWidget * par,  int ind=0, Qt::WFlags f = 0);
    ~EditConnectionDialog();
private:
    QLineEdit* sessName;
    QLineEdit* uname;
    QLineEdit* server;
    QSpinBox* sshPort;
    QLineEdit* key;
    QSpinBox* width;
    QSpinBox* height;
    QLineEdit* cmd;
    QComboBox* cmdCombo;
    QRadioButton* fs;
    QCheckBox* kbd;
    QLineEdit* layout;
    QLineEdit* type;
    QTreeView* expTv;
    QRadioButton* custom;
    QRadioButton* arts;
    QRadioButton* esd;
    QCheckBox* sound;
    QString sessIcon;
    QPushButton* icon;
    QComboBox* packMethode;
    QComboBox* sessBox;
    QSpinBox* quali;
    
    QStandardItemModel* model;
    QLabel *ldir;
    
    QLabel* qualiLabel;
    QSlider *spd;
    QString sessionId;    
    QLabel* widthLabel;
    QLabel* heightLabel;
    QLabel* layoutLabel;
    QLabel* typeLabel;
    QStringList qualiList;
    QTabWidget *fr;
    ONMainWindow* parent;
    
private:
    void readConfig();
    void loadPackMethods();

private slots:
    void slot_changeCaption(const QString& newName);
    void slot_changePack(const QString& pc);
    void slot_getIcon();
    void slot_getKey();
    void slot_accepted();
    void slot_default();
    void slot_changeCmd(int var);
    void slot_openDir();
    void slot_addDir();
    void slot_delDir();
};

#endif
