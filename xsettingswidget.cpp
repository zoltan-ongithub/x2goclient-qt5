/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2005-2012 Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "xsettingswidget.h"
#include "x2gosettings.h"
#include <QFileDialog>

XSettingsWidget::XSettingsWidget(QWidget* parent)
{
    setupUi(this);

    X2goSettings st ( "settings" );
    rbXming->setChecked(st.setting()->value("useintx",true).toBool());
    rbOther->setChecked(!(st.setting()->value("useintx",true).toBool()));
    leExec->setText(st.setting()->value("xexec","C:\\program files\\vcxsrv\\vcxsrv.exe").toString());
    leCmdOptions->setText(st.setting()->value("options","-multiwindow -notrayicon -clipboard").toString());

    cbOnstart->setChecked(true);
    cbOnstart->setChecked(st.setting()->value("onstart",true).toBool());

    leWinMod->setText(st.setting()->value("optionswin","-screen 0 %wx%h -notrayicon -clipboard").toString());
    leFSMod->setText(st.setting()->value("optionsfs","-fullscreen -notrayicon -clipboard").toString());
    leSingApp->setText(st.setting()->value("optionssingle","-multiwindow -notrayicon -clipboard").toString());
//     spDelay->setValue(st.setting()->value("delay",3).toInt());
    pbExec->setIcon( QPixmap ( ":/icons/16x16/file-open.png" ) );
}

XSettingsWidget::~XSettingsWidget()
{
}

void XSettingsWidget::slotSetExecutable()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                       "C:\\",
                       tr("Executable (*.exe)"));

    if (fileName.length())
        leExec->setText(fileName);

}

void XSettingsWidget::setDefaults()
{
    rbXming->setChecked(true);
    leExec->setText("C:\\program files\\vcxsrv\\vcxsrv.exe");
    leCmdOptions->setText("-multiwindow -notrayicon -clipboard");
    cbOnstart->setChecked(true);
    leWinMod->setText("-screen 0 %wx%h -notrayicon -clipboard");
    leFSMod->setText("-fullscreen -notrayicon -clipboard");
    leSingApp->setText("-multiwindow -notrayicon -clipboard");
//     spDelay->setValue(3);
  
}

void XSettingsWidget::saveSettings()
{
    X2goSettings st ( "settings" );
    st.setting()->setValue("useintx",rbXming->isChecked());
    st.setting()->setValue("xexec",leExec->text());
    st.setting()->setValue("options",leCmdOptions->text());
    st.setting()->setValue("onstart",cbOnstart->isChecked());

    st.setting()->setValue("optionswin",leWinMod->text());
    st.setting()->setValue("optionsfs",leFSMod->text());    
    st.setting()->setValue("optionssingle",leSingApp->text());
//     st.setting()->setValue("delay",spDelay->value());
    st.setting()->sync();
}

