/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
*                              <o.shneyder@phoca-gmbh.de>                 *
*   Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de>                 *
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

#ifndef X2GOUTILS_H
#define X2GOUTILS_H

#include <QString>
#include <QFont>
#include <QMessageBox>

#define UNUSED(x) do { (void) x; } while (0)

QString expandHome (QString path);

QString fixup_resource_URIs (const QString &res_path);

QString wrap_legacy_resource_URIs (const QString &res_path);

QString convert_to_rich_text (const QString &text, bool force = false);

void show_RichText_Generic_MsgBox (QMessageBox::Icon icon, const QString &main_text, const QString &informative_text);
void show_RichText_WarningMsgBox (const QString &main_text, const QString &informative_text = "");
void show_RichText_ErrorMsgBox (const QString &main_text, const QString &informative_text = "");

QString git_changelog_extract_commit_sha (const QString &gitlog);

bool font_is_monospaced (const QFont &font);

#ifdef Q_OS_DARWIN
void show_XQuartz_not_found_error ();
void show_XQuartz_start_error ();
void show_XQuartz_generic_error (const QString &main_error, const QString &additional_info);
#endif /* defined (Q_OS_DARWIN) */

#endif /* !defined (X2GOUTILS_H) */
