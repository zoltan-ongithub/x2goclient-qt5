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
#include <cstddef>

/* For terminal size. */
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined (Q_OS_UNIX)
#include <stdio.h>
#include <sys/ioctl.h>
#endif

#include "help.h"
#include "version.h"

help::prelude_t help::cleanup_prelude (help::prelude_t prelude) {
  for (help::prelude_t::iterator it = prelude.begin (); it != prelude.end (); ++it) {
    *it = (*it).trimmed ();
  }

  return (prelude);
}

help::params_t help::cleanup_params (help::params_t params) {
  for (help::params_t::const_iterator params_it = params.constBegin (); params_it != params.constEnd (); ++params_it) {
    (*params_it).first = (*params_it).first.trimmed ();
    (*params_it).second = (*params_it).second.trimmed ();
  }

  return (params);
}

help::prelude_t build_prelude () {
  help::prelude_t ret ();

  QStringList args = QCoreApplication::arguments ();

  QString ver ("X2Go Client " << VERSION);

  if (QFile::exists (":/txt/git-info")) {
    QFile file (":/txt/git-info");

    if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream (&file);

      QString git_info (stream.readAll ().trimmed ());

      if (!(git_info.isEmpty ())) {
        ver << " (Git information: " << git_info << ")";
      }
    }
  }

  ret.append (ver);
  ret.append ("Usage: " << args.at (0) << " [OPTION]...");
  ret.append ("Options:");
  ret.append ("");

  return (ret);
}

help::params_t help::build_params () {

}

help::data_t help::build_data () {
  return (help::data_t (help::cleanup_prelude (help::build_prelude ()), help::cleanup_params (help::build_params ())));
}

void help::pretty_print (help::data_t data) {
  help::data_t data = help::build_data ();

  QTextStream out << data.first.join ("\n") << "\n";

  std::size_t max_len = 0;

  /* Iterate over all parameter options and get max width. */
  for (help::params_t::const_iterator it = data.second.constBegin (); it != data.second.constEnd (); ++it) {
    max_len = std::max (max_len, (*it).first.size ());
  }

  std::size_t terminal_cols = 0;

#ifdef Q_OS_WIN
  CONSOLE_SCREEN_BUFFER_INFO terminal_internal;
  HANDLE stderr_handle = GetStdHandle (STD_ERROR_HANDLE);
  if (stderr_handle && (stderr_handle != INVALID_HANDLE_VALUE)) {
    if (GetConsoleScreenBufferInfo (stderr_handle, &terminal_internal)) {
      terminal_cols = (terminal_internal.srWindow.Right - terminal_internal.Left) + 1;
    }
  }
#elif defined (Q_OS_UNIX)
  struct winsize terminal_internal;
  ioctl (0, TIOCGWINSZ, &terminal_internal);
  terminal_cols = terminal_internal.ws_col;
#endif

  for (help::params_t::const_iterator it = data.second.constBegin (); it != data.second.constEnd (); ++it) {
    std::size_t indent = (max_len - (*it).first.size ()) + 4;
    out << "  ";
    out << (*it).first;
    out << QString (" ").repeated (indent);

    indent += 2;
    std::ptrdiff_t remaining = 0;
    std::size_t cur_len = (*it).second.size ();
    if (0 != terminal_cols) {
      remaining = terminal_cols - indent;

      /* Ran out of space? That's bad... print a newline and don't use any indentation level. */
      if (0 > remaining) {
        out << "\n";
        remaining = terminal_cols;
        indent = 0;
      }

      QString working_copy ((*it).second);

      do {
        cur_len = working_copy.size ();

        /* Fits onto the current line. Great! */
        if (remaining > cur_len) {
          out << working_copy;
        }
        else {
          /* Try to find the next split point. */
          std::ptrdiff_t split_point_white = working_copy.lastIndexOf (" ", remaining);
          std::ptrdiff_t split_point_hyphen = working_copy.lastIndexOf ("-", remaining);
          std::ptrdiff_t split_point = std::max (split_point_white, split_point_hyphen);

          if (-1 == split_point) {
            /* No split point available. Just print it out and hope for better times... */
            out << working_copy;
            working_copy = "";
          }
          else {
            /* Yay, we can split. */
            out << working_copy.left (split_point);

            /* If we split at a hyphen, don't lose it. */
            if (working_copy.at (split_point).compare ("-") == 0) {
              out << "-"
            }

            working_copy = working_copy.mid (split_point);

            /* Do the next chunk, if there are remaining characters. */
            if (!working_copy.isEmpty ()) {
              out << "\n"
              out << QString (" ").repeated (indent);
            }
          }
        }
      } while (!working_copy.isEmpty ());
    }
    else {
      /* No idea what the terminal size is. Just print it all onto one line. */
      out << (*it).second;
    }

    out << "\n";
  }
}
