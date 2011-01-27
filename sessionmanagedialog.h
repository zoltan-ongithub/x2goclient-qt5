//
// C++ Interface: sessionmanagedialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SESSIONMANAGEDIALOG_H
#define SESSIONMANAGEDIALOG_H
#include "x2goclientconfig.h"

#include <QDialog>
class QListView;
class QPushButton;
class QModelIndex;
class ONMainWindow;
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/

class SessionManageDialog : public QDialog
{
    Q_OBJECT
public:
    SessionManageDialog(QWidget * parent, Qt::WFlags f=0);
    ~SessionManageDialog();
    void loadSessions();
private:
    QListView* sessions;
    QPushButton* editSession;
    QPushButton* removeSession;
    ONMainWindow* par;
private slots:
    void slot_activated(const QModelIndex& index);
    void slotNew();
    void slot_edit();
    void slot_delete();
    void slot_dclicked(const QModelIndex& index);
};

#endif
