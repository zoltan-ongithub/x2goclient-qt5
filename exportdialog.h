//
// C++ Interface: exportdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "x2goclientconfig.h"
#include <QDialog>
class QListView;
class QPushButton;
class QModelIndex;
class ONMainWindow;


/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/
class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(QString sid,QWidget * par, Qt::WFlags f = 0);

    ~ExportDialog();
    QString getExport(){return directory;}
private:
    QListView* sessions;
    QPushButton* editSession;
    QPushButton* exportDir;
    QPushButton* newDir;
    QString directory;
    ONMainWindow* parent;
    void loadSessions();
    QString sessionId;
private slots:
    void slot_activated(const QModelIndex& index);
    void slotNew();
    void slot_edit();
    void slot_dclicked(const QModelIndex& index);
    void slot_accept();
};

#endif
