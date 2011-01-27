//
// C++ Interface: sessionbutton
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SESSIONBUTTON_H
#define SESSIONBUTTON_H

#include "SVGFrame.h"
#include <QPushButton>
class ONMainWindow;
class QLabel;
class QComboBox;
class QPushButton;

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/
class SessionButton : public SVGFrame
{
    Q_OBJECT
public:
    SessionButton(ONMainWindow* mw, QWidget* parent,QString id);
    ~SessionButton();
    QString id() {  return sid; }
    void redraw();
    static bool lessThen(const SessionButton* b1, const SessionButton* b2);
    QString name();
private:
    QString sid;
    QLabel* sessName;
    QLabel* icon;
    QComboBox* cmdBox;
    QLabel* cmd;
    QLabel* serverIcon;
    QLabel* geomIcon;
    QLabel* cmdIcon;
    QLabel* server;
    QPushButton* editBut;
    QPushButton* rmBut;
    QLabel* geom;
    QComboBox* geomBox;
    QPushButton* sound;
    QLabel* soundIcon;
    ONMainWindow* par;
    bool rootless;

private slots:
    void slotClicked();
    void slotEdit();
    void slot_soundClicked();
    void slot_cmd_change(const QString& command);
    void slot_geom_change(const QString& new_g);
    void slotRemove();
signals:
    void sessionSelected(SessionButton*);
    void signal_edit(SessionButton*);
    void signal_remove(SessionButton*);
    void clicked();
protected:
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
};

#endif
