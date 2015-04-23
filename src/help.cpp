/***************************************************************************
 *  Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de> +49 721 14595728 *
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, write to the                          *
 *  Free Software Foundation, Inc.,                                        *
 *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
 ***************************************************************************/

#include <QCoreApplication>
#include <QtDebug>
#include <QTextStream>
#include <QString>
#include <QFile>
#include <QObject>
#include <cstddef>
#include <algorithm>

/* For terminal size. */
#ifdef Q_OS_WIN
/* winsock2.h is not really needed, but including it silences a compile warning... */
#include <winsock2.h>
#include <windows.h>
#elif defined (Q_OS_UNIX)
#include <stdio.h>
#include <sys/ioctl.h>
#endif

#include "help.h"
#include "version.h"
#include "x2gologdebug.h"
#include "onmainwindow.h"

help::prelude_t help::cleanup_prelude (help::prelude_t prelude) {
  for (help::prelude_t::iterator it = prelude.begin (); it != prelude.end (); ++it) {
    *it = (*it).trimmed ();
  }

  return (prelude);
}

help::params_t help::cleanup_params (help::params_t params) {
  for (help::params_t::iterator it = params.begin (); it != params.end (); ++it) {
    (*it).first = (*it).first.trimmed ();
    (*it).second = (*it).second.trimmed ();
  }

  return (params);
}

help::prelude_t help::build_prelude () {
  help::prelude_t ret;

  QStringList args = QCoreApplication::arguments ();

  QString ver ("X2Go Client " + QString (VERSION));

  if (QFile::exists (":/txt/git-info")) {
    QFile file (":/txt/git-info");

    if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream (&file);

      QString git_info (stream.readAll ().trimmed ());

      if (!(git_info.isEmpty ())) {
        ver.append (" (Git information: " + git_info + ")");
      }
    }
  }

  ret.append (ver);
  ret.append ("Usage: " + QString (args.at (0)) + " [OPTION]...");
  ret.append ("Options:");
  ret.append ("");

  return (ret);
}

help::params_t help::build_params () {
  params_t ret;

# define ADD_OPT(param, desc) do { ret.append (params_elem_t (param, QObject::tr (desc))); } while (0)
# define NEWLINE "\n"

  ADD_OPT ("--help", "Shows this message.");
  ADD_OPT ("--version", "Prints version information.");

  if (QFile::exists (":/txt/changelog")) {
    ADD_OPT ("--changelog", "Shows the changelog.");
  }

  if (QFile::exists (":/txt/git-info")) {
    ADD_OPT ("--git-info", "Shows git information as used at compile this. [Deprecated: please use --version.]");
  }

  ADD_OPT ("--help-pack", "Shows available pack methods.");
  ADD_OPT ("--debug", "Enables extensive debug output to the console. On Windows also enables PulseAudio logging to a file in the \".x2go/pulse\" in the USERPROFILE directory." NEWLINE
                      "The logs are not deleted when X2Go Client terminates.");
  ADD_OPT ("--no-menu", "Hides menu bar.");
  ADD_OPT ("--no-session-edit", "Disables session editing.");
  ADD_OPT ("--maximize", "Starts maximized.");
  ADD_OPT ("--hide", "Starts hidden (minimized to system tray where available.)");
  ADD_OPT ("--portable", "Starts in \"portable\" mode.");
  ADD_OPT ("--pgp-card", "Forces OpenPGP smart card authentication.");
  ADD_OPT ("--xinerama", "Enables Xinerama by default.");
  ADD_OPT ("--ldap-printing", "Allows client side printing in LDAP mode.");
  ADD_OPT ("--thinclient", "Enables thinclient mode. Starts without a window manager.");
  ADD_OPT ("--haltbt", "Enables shutdown button.");
  ADD_OPT ("--add-to-known-hosts", "Adds RSA key fingerprint to \".ssh/known_hosts\" if authenticity of the server can't be determined.");
  ADD_OPT ("--ldap=<host:port:dn>", "Starts with LDAP support. Example: --ldap=ldapserver:389:o=organization,c=de");
  ADD_OPT ("--ldap1=<host:port>", "Defines the first LDAP failover server.");
  ADD_OPT ("--ldap2=<host:port>", "Defines the second LDAP failover server.");
  ADD_OPT ("--ssh-port=<port>", "Defines the remote SSH server port. Default: 22.");
  ADD_OPT ("--client-ssh-port=<port>", "Defines the local machine's SSH server port. Needed for Client-Side Printing and File Sharing support. Default: 22.");
  ADD_OPT ("--command=<cmd>", "Sets the default command. Default: 'KDE' (Desktop Session)");
  ADD_OPT ("--session=<session>", "Starts the session named \"session\".");
  ADD_OPT ("--user=<username>", "Sets the user name for connecting to the remote SSH server to \"username\".");
  ADD_OPT ("--geometry=<<W>x<H>|fullscreen>", "Sets the default window geometry. Default: 800x600.");
  ADD_OPT ("--dpi=<dpi>", "Sets the remote X2Go Agent's DPI value to \"dpi\". Default: same as local display.");
  ADD_OPT ("--link=<modem|isdn|adsl|wan|lan>", "Sets the default link type. Default: \"adsl\".");
  ADD_OPT ("--pack=<packmethod>", "Sets default pack method. Default: \"16m-jpeg-9\".");
  ADD_OPT ("--clipboard=<both|client|server|none>", "Sets the default clipboard mode. Default: \"both\".");
  ADD_OPT ("--kbd-layout=<layout>", "Sets the default keyboard layout to \"layout\". \"layout\" may be a comma-separated list.");
  ADD_OPT ("--kbd-type=<type>", "Sets the default keyboard type.");
  ADD_OPT ("--home=<dir>", "Sets the user's home directory.");
  ADD_OPT ("--set-kbd=<0|1>", "Enables or disables overwriting the current keyboard settings.");
  ADD_OPT ("--autostart=<app>", "Automatically launches the application \"app\" on session start in Published Applications mode.");
  ADD_OPT ("--session-conf=<file>", "Defines an alternative session config file path.");
  ADD_OPT ("--tray-icon", "Force-enables session system tray icon.");
  ADD_OPT ("--close-disconnect", "Automatically closes X2Go Client after a disconnect.");
  ADD_OPT ("--hide-foldersharing", "Hides all Folder-Sharing-related options.");
  ADD_OPT ("--broker-name=<name>", "Sets the broker name to display in X2Go Client. This parameter is optional.");
  ADD_OPT ("--broker-url=<protocol>://[username@]<host>[:port]/path", "Sets the URL of the session broker." NEWLINE
                                                                      "\"protocol\" must be one of \"http\", \"https\" or \"ssh\"." NEWLINE
                                                                      "If \"username@\" is provided, it will be pasted into the authorization dialog of X2Go Client." NEWLINE
                                                                      "URL examples are:" NEWLINE
                                                                      "    https://x2gobroker.org/cgi-bin/x2gobroker.cgi" NEWLINE
                                                                      "    ssh://user@x2gobroker.org:22/usr/lib/x2go/x2gobroker.pl");
  ADD_OPT ("--broker-ssh-key=<path to key>", "Sets the path to an SSH key to use for authentication against an SSH session broker. The client's behavior is undefined if this flag is used for non-SSH session brokers.");
  ADD_OPT ("--broker-autologin", "Enables the use of the default SSH key or SSH agent for authentication against an SSH session broker. The client's behavior is undefined if this flag is used for non-SSH session brokers.");
  ADD_OPT ("--broker-noauth", "Does not ask for user credentials during session broker authentication. This can be useful if you are using an HTTP(S) session broker without authentication. If you run an HTTP(S) server without authentication, but with user-specific profiles, then put the user name into the broker URL (refer to --broker-url.) The user name then will be extracted from the broker URL and be sent to the session broker. The client's behavior is undefined if this flag is used for non-HTTP(S) session brokers.");

# undef NEWLINE
# undef ADD_OPT

  return (ret);
}

help::data_t help::build_data () {
  return (help::data_t (help::cleanup_prelude (help::build_prelude ()), help::cleanup_params (help::build_params ())));
}

QString help::pretty_print () {
  return (help::pretty_print (help::build_data ()));
}

QString help::pretty_print (help::data_t data) {
  QString ret = "";
  QTextStream out (&ret);
  out << data.first.join ("\n") << "\n";

  std::size_t max_len = 0;

  /* Iterate over all parameter options and get max width. */
  for (help::params_t::const_iterator it = data.second.constBegin (); it != data.second.constEnd (); ++it) {
    max_len = std::max (max_len, static_cast<std::size_t> ((*it).first.size ()));
  }

  std::size_t terminal_cols = 0;

#ifdef Q_OS_WIN
  CONSOLE_SCREEN_BUFFER_INFO terminal_internal;
  HANDLE stderr_handle = GetStdHandle (STD_ERROR_HANDLE);
  if (stderr_handle && (stderr_handle != INVALID_HANDLE_VALUE)) {
    if (GetConsoleScreenBufferInfo (stderr_handle, &terminal_internal)) {
      terminal_cols = (terminal_internal.srWindow.Right - terminal_internal.srWindow.Left) + 1;
    }
  }
#elif defined (Q_OS_UNIX)
  struct winsize terminal_internal;
  ioctl (0, TIOCGWINSZ, &terminal_internal);
  terminal_cols = terminal_internal.ws_col;
#endif

  x2goDebug << "Terminal cols: " << terminal_cols << endl;

  for (help::params_t::const_iterator it = data.second.constBegin (); it != data.second.constEnd (); ++it) {
    std::size_t indent = (max_len - (*it).first.size ()) + 4;
    x2goDebug << "Indent: " << indent << "; max_len: " << max_len << "; param size: " << (*it).first.size () << endl;
    out << "  ";
    out << (*it).first;
    out << QString (" ").repeated (indent);

    /* Append first two spaces to the general indent level for upcoming lines. */
    indent += 2;
    std::ptrdiff_t remaining = 0;

    /* Split up description on newlines. */
    QStringList desc_split = (*it).second.split ("\n");

    for (QStringList::const_iterator desc_split_it = desc_split.constBegin (); desc_split_it != desc_split.constEnd (); ++desc_split_it) {
      std::size_t cur_len = (*desc_split_it).size ();
      x2goDebug << "Going to output a description " << (*desc_split_it).size () << " chars wide." << endl;
      if (0 != terminal_cols) {
        /*
         * Only set this the first time right after having written the parameter and indent spaces.
         * Don't change it after that.
         */
        if (desc_split_it == desc_split.constBegin ()) {
          remaining = terminal_cols - (indent + (*it).first.size ());
        }
        x2goDebug << "Still have " << remaining << " characters left on this line." << endl;

        /* Ran out of space? That's bad... print a newline and don't use any indentation level. */
        if (0 > remaining) {
          x2goDebug << "Ran out of space! Will break line and start the description on a new one." << endl;
          out << "\n";
          remaining = terminal_cols;
          indent = 0;
        }

        QString working_copy (*desc_split_it);

        while (!working_copy.isEmpty ()) {
          cur_len = working_copy.size ();
          x2goDebug << "Trying to fit a (remaining) description " << cur_len << " characters wide." << endl;

          /* Fits onto the current line. Great! */
          if (remaining > static_cast<std::ptrdiff_t> (cur_len)) {
            x2goDebug << "Fit onto the current line. Done." << endl;
            out << working_copy;
            working_copy = "";
          }
          else {
            /* Try to find the next split point. */
            std::ptrdiff_t split_point = working_copy.lastIndexOf (" ", remaining - 1);

            if (-1 == split_point) {
              /* No split point available. Just print it out and hope for better times... */
              out << working_copy;
              working_copy = "";
            }
            else {
              /* Yay, we can split. */
              x2goDebug << "Split onto " << working_copy.left (split_point);
              out << working_copy.left (split_point);

              x2goDebug << " and new part " << working_copy.mid (split_point + 1);
              working_copy = working_copy.mid (split_point + 1);

              /* Do the next chunk, if there are remaining characters. */
              if (!working_copy.isEmpty ()) {
                out << "\n";
                indent = terminal_cols - remaining;
                out << QString (" ").repeated (indent);
              }
            }
          }
        }
      }
      else {
        /* No idea what the terminal size is. Just print it all onto one line. */
        out << (*desc_split_it);
      }

      out << "\n";

      /* Add whitespace if description shall continue on next line. */
      if ((desc_split_it + 1) != desc_split.constEnd ()) {
        indent = 2 + max_len + 4;
        out << QString (" ").repeated (indent);
      }
    }
  }

  qCritical ().nospace () << qPrintable (ret);

  return (ret);
}
