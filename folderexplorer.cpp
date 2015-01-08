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
#include "folderexplorer.h"
#include "x2gologdebug.h"
#include "sessionexplorer.h"
#include "folderbutton.h"
#include <QMenu>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

FolderExplorer::FolderExplorer(QString path, SessionExplorer* explorer, QWidget* parent):QDialog(parent)
{
    setupUi(this);
    this->explorer=explorer;
    root=new QTreeWidgetItem(treeWidget);
    root->setText(0,"/");
    root->setIcon(0,QIcon(":icons/128x128/folder.png"));
    currentPath=path;
    initFolders(root, "");
    root->setExpanded(true);
    if(currentPath=="/")
        root->setSelected(true);
    root->setData(0,Qt::UserRole, "/");

}

void FolderExplorer::initFolders(QTreeWidgetItem* parent, QString path)
{
    FolderButton* b;
    foreach(b, *(explorer->getFoldersList()))
    {
        if(b->getPath()==path)
        {
            QTreeWidgetItem* it=new QTreeWidgetItem(parent);
            it->setText(0,b->getName());
            it->setIcon(0, QIcon(*(b->folderIcon())));
            QString normPath=(b->getPath()+"/"+b->getName()).split("/",QString::SkipEmptyParts).join("/");
            it->setData(0,Qt::UserRole, normPath+"/");
            if(normPath+"/"==currentPath)
            {

                it->setSelected(true);
                QTreeWidgetItem* p=it->parent();
                while(p!=root)
                {
                    p->setExpanded(true);
                    p=p->parent();
                }
            }
            initFolders(it, normPath);
        }
    }
}

void FolderExplorer::slotContextMenu(QPoint p)
{
    menuItem=treeWidget->itemAt(p);
    if(!menuItem)
        return;

    QMenu menu(treeWidget);
    connect(menu.addAction(tr("Create New Folder")), SIGNAL(triggered(bool)), this, SLOT(slotNewFolder()));

    if(menuItem!=root)
    {
        connect(menu.addAction(tr("Rename Folder...")), SIGNAL(triggered(bool)), this, SLOT(slotChangeName()));
        connect(menu.addAction(tr("Change Icon...")), SIGNAL(triggered(bool)), this, SLOT(slotChangeIcon()));
        connect(menu.addAction(tr("Delete Folder...")), SIGNAL(triggered(bool)), this, SLOT(slotDeleteFolder()));
    }
    menu.exec(treeWidget->viewport()->mapToGlobal(p));
}

void FolderExplorer::slotItemSelected(QTreeWidgetItem* it, int)
{
    currentPath=it->data(0,Qt::UserRole).toString();
}

void FolderExplorer::slotChangeIcon()
{
    QString path= QFileDialog::getOpenFileName (
                      this,
                      tr ( "Open picture" ),
                      QDir::homePath(),
                      tr ( "Pictures" ) +" (*.png *.xpm *.jpg)" );
    if ( path!=QString::null )
    {
        explorer->setFolderIcon(menuItem->data(0, Qt::UserRole).toString(), path);
        menuItem->setIcon(0, QIcon(path));
    }
}

void FolderExplorer::slotChangeName()
{
    bool ok;
    QString oldPath=menuItem->data(0,Qt::UserRole).toString();
    QStringList parts=oldPath.split("/",QString::SkipEmptyParts);
    QString text = QInputDialog::getText(this, tr("X2Go Client"),
                                         tr("Folder Name:"), QLineEdit::Normal,
                                         parts.last(), &ok);
    if (ok && !text.isEmpty())
    {
        menuItem->setText(0,text);
        parts.pop_back();
        parts<<text;
        currentPath= parts.join("/")+"/";
        menuItem->setData(0,Qt::UserRole, currentPath);
        explorer->renameFolder(oldPath, currentPath);
    }
}

void FolderExplorer::slotDeleteFolder()
{
    if(!explorer->isFolderEmpty(menuItem->data(0, Qt::UserRole).toString()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Unable to remove \"")+menuItem->text(0)+
                              tr("\". Folder is not empty. Please remove content of folder and try again."));
        return;
    }
    if(QMessageBox::question(this, "X2Go Client", tr("Delete folder \"")+menuItem->text(0)+"\"?",QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
    {
        explorer->deleteFolder(menuItem->data(0, Qt::UserRole).toString());
        currentPath="/";
        delete menuItem;
    }
}

void FolderExplorer::slotNewFolder()
{
    QTreeWidgetItem* it=new QTreeWidgetItem(menuItem);
    QString name=tr("New Folder");
    it->setText(0,name);
    it->setIcon(0, QIcon(":icons/128x128/folder.png"));
    QString normPath=(menuItem->data(0,Qt::UserRole).toString()+"/"+name).split("/",QString::SkipEmptyParts).join("/");
    it->setData(0,Qt::UserRole, normPath+"/");
    treeWidget->clearSelection();
    it->setSelected(true);
    QTreeWidgetItem* p=it->parent();
    while(p!=root)
    {
        p->setExpanded(true);
        p=p->parent();
    }
    slotItemSelected(it,0);
    explorer->createNewFolder(normPath);
}
