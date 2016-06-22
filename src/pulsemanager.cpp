/***************************************************************************
 *  Copyright (C) 2012-2015 by Mihai Moldovan <ionic@ionic.de>             *
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

#include <unistd.h>
#include <stdlib.h>
#include <QSysInfo>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif /* QT_VERSION >= 0x050000 */

#include "pulsemanager.h"
#include "x2gologdebug.h"
#include "x2goutils.h"

PulseManager::PulseManager () : app_dir_ (QApplication::applicationDirPath ()),
                                pulse_X2Go_ ("/.x2go/pulse"),
                                server_binary_ (QString ("")),
                                server_working_dir_ (QString ("")),
                                server_args_ (QStringList ()),
                                pulse_server_ (NULL),
                                state_ (QProcess::NotRunning),
                                pulse_port_ (4713),
                                esd_port_ (4714),
                                pulse_version_major_ (0),
                                pulse_version_minor_ (0),
                                pulse_version_micro_ (0),
                                pulse_version_misc_ (""),
                                record_ (true),
                                playback_ (true),
                                debug_ (false),
                                system_pulse_ (false) {
  pulse_dir_ = QDir (QDir::homePath ());
  pulse_dir_.mkpath (pulse_dir_.absolutePath () + pulse_X2Go_ + "/tmp");
  pulse_dir_.cd (pulse_X2Go_.mid (1));

  env_ = QProcessEnvironment::systemEnvironment ();
  env_.insert ("HOME", pulse_dir_.absolutePath ());
  env_.insert ("TEMP", pulse_dir_.absolutePath () + "/tmp");
#ifdef Q_OS_WIN
  env_.insert ("USERPROFILE", pulse_dir_.absolutePath ());
  env_.insert ("USERNAME", "pulseuser");
#endif // defined (Q_OS_WIN)

  /* Set server binary and working dir paths. */
#ifdef Q_OS_DARWIN
  /* Assume bundled PA first. */
  server_working_dir_ = QString (app_dir_ + "/../exe/");
  server_binary_ = QString (server_working_dir_ + "/pulseaudio");

#if QT_VERSION < 0x050000
  QProcessEnvironment tmp_env = QProcessEnvironment::systemEnvironment ();
  QString path_val = tmp_env.value ("PATH");

  QStringList to_front, to_back;
  to_front << "/opt/local/bin"; /* MacPorts default prefix, FIXME: might need to make that configurable. */
  to_back << "/usr/local/bin"; /* Homebrew or random stuff. Probably even both intermingled... */

  add_to_path (path_val, to_back);
  add_to_path (path_val, to_front, false);

  server_binary_ = find_binary (server_working_dir_, "pulseaudio");

  if (server_binary_.isEmpty ()) {
    server_binary_ = find_binary (path_val, "pulseaudio");

    if (server_binary_.isEmpty ()) {
      x2goErrorf (29) << "Unable to find PulseAudio binary. Neither bundled, nor found in $PATH nor additional directories.";
      show_RichText_ErrorMsgBox (tr ("Unable to find PulseAudio binary. Neither bundled, nor found in $PATH nor additional directories."),
                                 tr ("If you downloaded the bundled, pre-compiled version from the official home page, "
                                     "please report a bug on:\n"
                                     "<center><a href=\"https://wiki.x2go.org/doku.php/wiki:bugs\">"
                                       "https://wiki.x2go.org/doku.php/wiki:bugs"
                                     "</a></center>\n"),
                                 true);
      abort ();
    }
    else {
      system_pulse_ = true;
    }
  }
#else /* QT_VERSION < 0x050000 */
  QStringList search_paths;
  search_paths << server_working_dir_;

  server_binary_ = QStandardPaths::findExecutable ("pulseaudio", search_paths);

  if (server_binary_.isEmpty ()) {
    search_paths = QStringList ();
    search_paths << "/opt/local/bin"; /* MacPorts default prefix, FIXME: might need to make that configurable. */

    server_binary_ = QStandardPaths::findExecutable ("pulseaudio", search_paths);

    if (server_binary_.isEmpty ()) {
      search_paths = QStringList ();

      server_binary_ = QStandardPaths::findExecutable ("pulseaudio", search_paths);

      if (server_binary_.isEmpty ()) {
        search_paths = QStringList ();
        search_paths << "/usr/local/bin"; /* Homebrew or random stuff. Probably even both intermingled... */

        server_binary_ = QStandardPaths::findExecutable ("pulseaudio", search_paths);

        if (server_binary_.isEmpty ()) {
          x2goErrorf (28) << "Unable to find PulseAudio binary. Neither bundled, nor found in $PATH nor additional directories.";
          show_RichText_ErrorMsgBox (tr ("Unable to find PulseAudio binary. Neither bundled, nor found in $PATH nor additional directories."),
                                     tr ("If you downloaded the bundled, pre-compiled version from the official home page, "
                                         "please report a bug on:\n"
                                         "<center><a href=\"https://wiki.x2go.org/doku.php/wiki:bugs\">"
                                           "https://wiki.x2go.org/doku.php/wiki:bugs"
                                         "</a></center>\n"),
                                     true);
          abort ();
        }
      }
    }

    /*
     * The detection above either failed or succeeded.
     * Failure means that the program already stopped,
     * success means that all code reaches this point.
     * Be careful when refactoring this code.
     */
    system_pulse_ = true;
  }
#endif /* QT_VERSION < 0x050000 */

  QFileInfo tmp_file_info = QFileInfo (server_binary_);
  server_working_dir_ = tmp_file_info.canonicalPath ();

  x2goDebug << "Found PA binary as " << server_binary_ << endl;
  x2goDebug << "Corresponding working dir: " << server_working_dir_ << endl;

#elif defined (Q_OS_WIN)
  server_working_dir_ = QString (app_dir_ + "/pulse/");
  server_binary_ = QString (app_dir_ + "/pulse/pulseaudio.exe");
#elif defined (Q_OS_LINUX)
  std::size_t path_len = pathconf (".", _PC_PATH_MAX);

  if (-1 == path_len) {
    path_len = 1024;
  }

  char *buf, *ptr;

  for (buf = ptr = NULL; ptr == NULL; path_len += 20) {
    if (NULL == (buf = realloc (buf, path_len))) {
      x2goErrorf (16) << "Could not allocate buffer for getting current working directory!";
      show_RichText_ErrorMsgBox (tr ("Could not allocate buffer for getting current working directory!"),
                                 QString (),
                                 true);
      abort ();
    }

    memset (buf, 0, path_len);
    ptr = getcwd (buf, path_len);

    if ((NULL == ptr) && (ERANGE != erange)) {
      int saved_errno = errno;
      x2goErrorf (17) << "getcwd() failed: " << QString (strerror (saved_errno));
      show_RichText_ErrorMsgBox (tr ("getcwd() failed!"),
                                 QString (strerror (saved_errno)),
                                 true);
      abort ();
    }
  }

  server_working_dir_ = QString (buf);
  server_binary_ = QString ("pulseaudio");

  free (buf);
  buf = ptr = NULL;
#endif // defined (Q_OS_DARWIN)

  fetch_pulseaudio_version ();
}

PulseManager::~PulseManager () {
  if (pulse_server_ && is_server_running ())
    shutdown ();

  delete (pulse_server_);
}

void PulseManager::start () {
  if (is_server_running ()) {
    return;
  }

  delete (pulse_server_);

  pulse_server_ = new QProcess (0);
  state_ = QProcess::Starting;

  // Search for a free Pulse and EsounD port.
  // Note that there is no way we could find
  // an esd port, if the pulse port detection
  // failed. Better trust your compiler to
  // optimize this statement and save some
  // cycles.
  if ((find_port (false)) && (find_port (true))) {
#ifdef Q_OS_DARWIN
    start_osx ();
#elif defined (Q_OS_WIN)
    start_win ();
#elif defined (Q_OS_LINUX)
    start_linux ();
#endif // defined (Q_OS_DARWIN)
  }
}

void PulseManager::start_generic () {
  pulse_server_->setProcessEnvironment (env_);
  pulse_server_->setWorkingDirectory (server_working_dir_);

  pulse_server_->start (server_binary_, server_args_);

  /*
   * We may wait here, because PulseManager runs in a separate thread.
   * Otherwise, we'd better use signals and slots to not block the main thread.
   */
  if (pulse_server_->waitForStarted (-1)) {
    x2goDebug << "pulse started with arguments" << server_args_ << "; waiting for finish...";
    state_ = QProcess::Running;

    connect (pulse_server_, SIGNAL (finished (int)),
             this,          SLOT (slot_on_pulse_finished (int)));

    env_.insert ("PULSE_SERVER", "127.0.0.1:" + QString::number (pulse_port_));


    QString clean_pulse_dir = pulse_dir_.absolutePath ();

#ifdef Q_OS_WIN
    clean_pulse_dir = wapiShortFileName (clean_pulse_dir);
#endif /* defined (Q_OS_WIN) */

    QString tmp_auth_cookie = QDir::toNativeSeparators (clean_pulse_dir + "/.pulse-cookie");

    env_.insert ("PULSE_COOKIE", tmp_auth_cookie);

    if (debug_) {
      // Give PA a little time to come up.
      QTimer::singleShot (3000, this, SLOT (slot_play_startup_sound ()));
    }
  }
  else {
    x2goErrorf (27) << "PulseAudio failed to start! Sound support will not be available.";
  }
}

void PulseManager::start_osx () {
  server_args_ = QStringList ();
  server_args_ << "--exit-idle-time=-1" << "-n"
               << "-F" << pulse_dir_.absolutePath () + "/config.pa"
               << "-p"
               << QDir (app_dir_
                        + "/../Frameworks/pulse-"
                        + QString::number (pulse_version_major_)
                        + "."
                        + QString::number (pulse_version_minor_)
                        + "/modules").absolutePath ()
               << "--high-priority";
  if (debug_) {
    server_args_ << "--log-level=debug"
                 << "--verbose"
                 << "--log-target=file:" + pulse_dir_.absolutePath () + "/pulse.log";
  }

  if (generate_server_config () && generate_client_config ()) {
    cleanup_client_dir ();

    start_generic ();
  }
}

void PulseManager::start_win () {
/*
 * Some code in here is Windows-specific and will lead to compile
 * failures on other platforms. Make this a stub for everything non-Windows.
 */
#ifdef Q_OS_WIN
  server_args_ = QStringList ();
  server_args_ << "--exit-idle-time=-1" << "-n"
               << "-F" << QDir::toNativeSeparators (QDir (pulse_dir_.absolutePath ()
                                                          + "/config.pa").absolutePath ())
               << "-p" << QDir::toNativeSeparators (QDir (app_dir_ + "/pulse/lib/pulse-"
                                                          + QString::number (pulse_version_major_)
                                                          + "."
                                                          + QString::number (pulse_version_minor_)
                                                          + "/modules/").absolutePath ());
  if (debug_) {
    server_args_ << "--log-level=debug"
                 << "--verbose"
                 << "--log-target=file:" + pulse_dir_.absolutePath () + "\\pulse.log";
  }

  /*
   * Fix for x2goclient bug #526.
   * Works around PulseAudio bug #80772.
   * Tested with PulseAudio 5.0.
   * This argument will not cause PulseAudio 0.9.6 or 1.1 (the legacy versions)
   * to fail to launch.
   * However, 0.9.6 defaults to normal priority anyway,
   * and 1.1 ignores it for some reason.
   * So yes, the fact that 1.1 ignores it would be a bug in x2goclient if we
   * ever ship 1.1 again.
   */
  if ((QSysInfo::WindowsVersion == QSysInfo::WV_XP) || (QSysInfo::WindowsVersion == QSysInfo::WV_2003)) {
    x2goDebug << "Windows XP or Server 2003 (R2) detected."
              << "Setting PulseAudio to \"normal\" CPU priority.";

    server_args_ << "--high-priority=no";
  }

  if (generate_server_config () && generate_client_config ()) {
    create_client_dir ();

    start_generic ();
  }
#endif /* defined (Q_OS_WIN) */
}

void PulseManager::start_linux () {
  /* Do nothing - assumed to be already running. */
}

void PulseManager::fetch_pulseaudio_version () {
  QStringList args (QString ("--version"));
  QProcess tmp_server (this);

  /* Start PA with --version argument. */
  tmp_server.setWorkingDirectory (server_working_dir_);
  tmp_server.start (server_binary_, args);

  /* Wait until the process exited again. */
  if (tmp_server.waitForFinished ()) {
    /* Read stdout and split it up on newlines. */
    QByteArray ba (tmp_server.readAllStandardOutput ());
    QString stdout_data (ba.data ());
    QStringList stdout_list (stdout_data.split ("\n"));

    bool found = false;
    for (QStringList::const_iterator cit = stdout_list.begin (); cit != stdout_list.end (); ++cit) {
      /* Remove trailing whitespace, mostly carriage returns on Windows. */
      QString tmp_str (*cit);
      tmp_str = tmp_str.trimmed ();

      QString needle ("pulseaudio ");

      if (tmp_str.startsWith (needle)) {
        /* Drop first part. */
        tmp_str = tmp_str.mid (needle.size ());

        /* We should be at a digit now. */
        bool numbers_found[3] = { false, false, false };
        QString tmp_ret_str = QString ("");
        for (QString::const_iterator cit = tmp_str.begin (); cit != tmp_str.end (); ++cit) {
          if (!(numbers_found[0])) {
            if (((*cit) >= '0') && ((*cit) <= '9')) {
              tmp_ret_str.append (*cit);
            }
            else if ((*cit) == '.') {
              /* First number part complete, let's convert the string and skip the period. */
              numbers_found[0] = true;
              bool convert_success = false;
              pulse_version_major_ = tmp_ret_str.toUInt (&convert_success, 10);

              if (!convert_success) {
                x2goErrorf (20) << "Unable to convert major version number string to integer.";
                show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                           tr ("Unable to convert major version number string to integer."),
                                           true);
                abort ();
              }

              tmp_ret_str = QString ("");
            }
            else {
              x2goErrorf (21) << "Unexpected character found when parsing version string for major version number: '" << QString (*cit) << "'.";
              show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                         tr ("Unexpected character found when parsing version string for major version number")
                                         + ": '" + QString (*cit) + "'.",
                                         true);
              abort ();
            }
          }
          else if (!(numbers_found[1])) {
            if (((*cit) >= '0') && ((*cit) <= '9')) {
              tmp_ret_str.append (*cit);
            }
            else if (((*cit) == '.') || ((*cit) == '-')) {
              /*
               * Second number part complete, let's convert the string and then check whether
               * we stopped at a period or a dash character.
               */
              numbers_found[1] = true;
              bool convert_success = false;
              pulse_version_minor_ = tmp_ret_str.toUInt (&convert_success, 10);

              if (!convert_success) {
                x2goErrorf (22) << "Unable to convert minor version number string to integer.";
                show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                           tr ("Unable to convert minor version number string to integer."),
                                           true);
                abort ();
              }

              tmp_ret_str = QString ("");

              if ((*cit) == '-') {
                /*
                 * There will be no micro version, skip it entirely and assume the default
                 * value of zero.
                 */
                numbers_found[2] = true;
              }
            }
            else {
              x2goErrorf (23) << "Unexpected character found when parsing version string for minor version number: '" << QString (*cit) << "'.";
              show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                         tr ("Unexpected character found when parsing version string for minor version number")
                                         + ": '" + QString (*cit) + "'.",
                                         true);
              abort ();
            }
          }
          else if (!(numbers_found[2])) {
            if (((*cit) >= '0') && ((*cit) <= '9')) {
              tmp_ret_str.append (*cit);
            }
            else if ((*cit) == '-') {
              /* Third number part complete, let's convert the string and skip the period. */
              numbers_found[2] = true;
              bool convert_success = false;
              pulse_version_micro_ = tmp_ret_str.toUInt (&convert_success, 10);

              if (!convert_success) {
                x2goErrorf (24) << "Unable to convert micro version number string to integer.";
                show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                           tr ("Unable to convert micro version number string to integer."),
                                           true);
                abort ();
              }

              tmp_ret_str = QString ("");
            }
            else {
              x2goErrorf (25) << "Unexpected character found when parsing version string for micro version number: '" << QString (*cit) << "'.";
              show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                         tr ("Unexpected character found when parsing version string for micro version number")
                                         + ": '" + QString (*cit) + "'.",
                                         true);
              abort ();
            }
          }
          else {
            /* Numbers should be good by now, let's fetch everything else. */
            tmp_ret_str.append (*cit);
          }
        }

        found = ((numbers_found[0]) && (numbers_found[1]) && (numbers_found[2]));

        if (found) {
          /* Misc version part will be set to the trailing string. */
          pulse_version_misc_ = tmp_ret_str;
          break;
        }
      }
      else {
        /* No need to look any further. */
        continue;
      }
    }

    if (!found) {
      x2goErrorf (19) << "Unable to fetch PulseAudio version - unexpected format. Exiting.";
      show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                                 tr ("Unexpected format encountered."),
                                 true);
      abort ();
    }
  }
  else {
    x2goErrorf (18) << "Unable to start PulseAudio to fetch its version number. Exiting.";
    show_RichText_ErrorMsgBox (tr ("Error fetching PulseAudio version number!"),
                               tr ("Unable to start PulseAudio binary."),
                               true);
    abort ();
  }
}

bool PulseManager::find_port (bool search_esd) {
  QTcpSocket tcpSocket (0);
  bool free = false;
  std::uint16_t search_port = pulse_port_;
  std::uint16_t other_port = esd_port_;

  // If the search_esd parameter is true, find a free port
  // for the PulseAudio emulation.
  if (search_esd) {
    search_port = esd_port_;
    other_port = pulse_port_;
  }

  do {
    // Skip this port, if it's reserved for the counterpart.
    if (search_port == other_port) {
      ++search_port;
      continue;
    }

    tcpSocket.connectToHost ("127.0.0.1", search_port);

    if (tcpSocket.waitForConnected (1000)) {
      tcpSocket.close ();
      free = false;
      ++search_port;
    }
    else {
      free = true;
    }
  } while ((!free) && (search_port > 1023));

  if (!search_esd) {
    pulse_port_ = search_port;
  }
  else {
    esd_port_ = search_port;
  }

  return (free);
}

bool PulseManager::generate_server_config () {
  QString config_file_name (pulse_dir_.absolutePath () + "/config.pa");
  QTemporaryFile config_tmp_file (pulse_dir_.absolutePath () + "/tmp/tmpconfig");
  bool ret = false;

  if (config_tmp_file.open ()) {
    QTextStream config_tmp_file_stream (&config_tmp_file);

    config_tmp_file_stream << "load-module module-native-protocol-tcp port="
                            + QString::number (pulse_port_);

    /*
     * Reference:
     * http://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/Modules/#index22h3
     *
     * Setting auth-cookie fixes bug #422.
     *
     * PulseAudio 6.0 changed the path that auth-cookie is relative to, so
     * Tanu Kaskinen recommended we specify the absolute path instead.
     * The absolute path works with at least 5.0 and 6.0.
     */
    if (pulse_version_major_ > 2) {
      QString clean_pulse_dir = pulse_dir_.absolutePath ();

#ifdef Q_OS_WIN
      clean_pulse_dir = wapiShortFileName (clean_pulse_dir);
#endif /* defined (Q_OS_WIN) */

      QString tmp_auth_cookie = QDir::toNativeSeparators (clean_pulse_dir + "/.pulse-cookie");

#ifdef Q_OS_WIN
      /* Double backslashes are required in config.pa. */
      tmp_auth_cookie.replace ("\\", "\\\\");
#endif /* defined (Q_OS_WIN) */

      config_tmp_file_stream << " auth-cookie=" + tmp_auth_cookie;
    }

    config_tmp_file_stream << endl;

#ifdef Q_OS_UNIX
    config_tmp_file_stream << "load-module module-native-protocol-unix" << endl;
    config_tmp_file_stream << "load-module module-esound-protocol-unix" << endl;
#endif // defined(Q_OS_UNIX)

    config_tmp_file_stream << "load-module module-esound-protocol-tcp port="
                           << QString::number (esd_port_)
                           << endl;

#ifdef Q_OS_DARWIN
    config_tmp_file_stream << "load-module module-coreaudio-detect";
#elif defined (Q_OS_WIN)
    config_tmp_file_stream << "load-module module-waveout";
// FIXME Linux
#endif // defined (Q_OS_DARWIN)

    config_tmp_file_stream << " record=";
    if (!record_) {
      config_tmp_file_stream << "0";
    }
    else {
      config_tmp_file_stream << "1";
    }

    config_tmp_file_stream << " playback=";
    if (!playback_) {
      config_tmp_file_stream << "0";
    }
    else {
      config_tmp_file_stream << "1";
    }
    config_tmp_file_stream << endl;

    QFile config_file (config_file_name);
    if (QFile::exists (config_file_name))
      QFile::remove (config_file_name);

    config_tmp_file.copy (config_file_name);
    config_tmp_file.remove ();

    ret = true;
  }

  return (ret);
}

bool PulseManager::generate_client_config () {
  QTemporaryFile client_config_tmp_file (pulse_dir_.absolutePath ()
                                         + "/tmp/tmpconfig");
  QString client_config_file_name (pulse_dir_.absolutePath ()
                                   + "/.pulse/client.conf");
  bool ret = false;

  if (client_config_tmp_file.open ()) {
    QTextStream config_tmp_file_stream (&client_config_tmp_file);

    config_tmp_file_stream << "autospawn=no" << endl;
#ifdef Q_OS_WIN
    config_tmp_file_stream << "default-server=localhost:" << pulse_port_ << endl;
#endif // defined (Q_OS_WIN)
    config_tmp_file_stream << "daemon-binary="
                           << QDir::toNativeSeparators (QDir (server_binary_).absolutePath ())
                           << endl;

    if (QFile::exists (client_config_file_name))
      QFile::remove (client_config_file_name);

    QDir client_config_dir (pulse_dir_.absolutePath () + "/.pulse/");
    client_config_dir.mkpath (client_config_dir.absolutePath ());

    client_config_tmp_file.copy (client_config_file_name);
    client_config_tmp_file.remove ();

    ret = true;
  }

  return (ret);
}

void PulseManager::cleanup_client_dir () {
  // PA expects $HOME/.pulse/$HOST-runtime to be a symbolic link
  // and will fail, if it's just a plain directory on Mac OS X.
  // Delete it first.
  QDir machine_dir (pulse_dir_.absolutePath () + "/.pulse/"
                    + QHostInfo::localHostName () + "-runtime");

  if (QFile::exists (machine_dir.absolutePath () + "/pid"))
    QFile::remove (machine_dir.absolutePath () + "/pid");

  if (machine_dir.exists ())
    machine_dir.remove (machine_dir.absolutePath ());
}

void PulseManager::create_client_dir () {
  QDir machine_dir (pulse_dir_.absolutePath () + "/.pulse/"
                    + QHostInfo::localHostName () + "-runtime");

  if (!machine_dir.exists ())
    machine_dir.mkpath (machine_dir.absolutePath ());

  if (QFile::exists (machine_dir.absolutePath () + "/pid"))
    QFile::remove (machine_dir.absolutePath () + "/pid");
}

void PulseManager::slot_play_startup_sound () {
  if (debug_) {
    QProcess play_file (0);
    QString play_file_binary (app_dir_);
    QString play_file_file (play_file_binary);

#ifdef Q_OS_DARWIN
    play_file_binary += "/../exe/paplay";
    play_file_file += "/../Resources/startup.wav";
#elif defined (Q_OS_WIN)
    play_file_binary += "/pulse/paplay.exe";
    play_file_file += "/startup.wav";
#else
    /* FIXME: implement Linux section. */
#endif // defined (Q_OS_DARWIN)

    QStringList args (play_file_file);
    play_file.setWorkingDirectory (server_working_dir_);
    play_file.setProcessEnvironment (env_);
    play_file.start (play_file_binary, args);

    if (play_file.waitForStarted (-1)) {
      play_file.waitForFinished ();
    }
    else {
      x2goErrorf (26) << "Unable to play startup sound! Something may be wrong.";
    }
  }
}

void PulseManager::slot_on_pulse_finished (int exit_code) {
  if (!exit_code)
    x2goDebug << "Warning! Pulseaudio's exit code is non-zero.";

  QByteArray ba (pulse_server_->readAllStandardOutput ());
  char *data = ba.data ();
  x2goDebug << data;
  ba = pulse_server_->readAllStandardError ();
  data = ba.data ();
  x2goDebug << data;

  // Clean up
  QDir work_dir (app_dir_);

#ifdef Q_OS_WIN
  work_dir.remove (pulse_dir_.absolutePath ()
                   + "/pulse-pulseuser/pid");
  work_dir.rmdir (pulse_dir_.absolutePath ()
                  + "/pulse-pulseuser");
#endif // defined (Q_OS_WIN)

#if defined (Q_OS_DARWIN) || defined (Q_OS_WIN)
  // Remove server config file, otherwise the directory won't be empty.
  if (!debug_) {
    work_dir.remove (QDir::toNativeSeparators (QDir (pulse_dir_.absolutePath ()
                                                     + "/config.pa").absolutePath ()));
    work_dir.remove (QDir::toNativeSeparators (QDir (pulse_dir_.absolutePath ()
                                                     + "/pulse.log").absolutePath ()));
  }
#else // Linux
  // FIXME.
#endif

  work_dir.rmdir (pulse_dir_.absolutePath ());

  state_ = QProcess::NotRunning;
  emit (sig_pulse_server_terminated ());
}

bool PulseManager::is_server_running () {
  if (pulse_server_)
    return (pulse_server_->state () == QProcess::Running);
  else
    return (false);
}

void PulseManager::shutdown () {
  QEventLoop loop;

  connect (this,  SIGNAL (sig_pulse_server_terminated ()),
           &loop, SLOT (quit ()));

  // Console applications without an event loop can only be terminated
  // by QProcess::kill() on Windows (unless they handle WM_CLOSE, which
  // PA obviously doesn't.)
#ifdef Q_OS_WIN
  pulse_server_->kill ();
#else // defined (Q_OS_WIN)
  pulse_server_->terminate ();
#endif // defined (Q_OS_WIN)

  loop.exec ();
}

std::uint16_t PulseManager::get_pulse_port () {
  return (pulse_port_);
}

std::uint16_t PulseManager::get_esd_port () {
  return (esd_port_);
}

bool PulseManager::get_record () {
  return (record_);
}

bool PulseManager::get_playback () {
  return (playback_);
}

QDir PulseManager::get_pulse_dir () {
  return (pulse_dir_);
}

bool PulseManager::set_pulse_port (std::uint16_t pulse_port) {
  bool ret = false;

  if (!(is_server_running ())) {
    pulse_port_ = pulse_port;
    ret = true;
  }

  return (ret);
}

bool PulseManager::set_esd_port (std::uint16_t esd_port) {
  bool ret = false;

  if (!(is_server_running ())) {
    esd_port_ = esd_port;
    ret = true;
  }

  return (ret);
}

bool PulseManager::set_record (bool record) {
  bool ret = false;

  if (!(is_server_running ())) {
    record_ = record;
    ret = true;
  }

  return (ret);
}

bool PulseManager::set_playback (bool playback) {
  bool ret = false;

  if (!(is_server_running ())) {
    playback_ = playback;
    ret = true;
  }

  return (ret);
}

void PulseManager::set_debug (bool debug) {
  debug_ = debug;
}

void PulseManager::restart () {
  if (pulse_server_ && is_server_running ())
    shutdown ();

  x2goDebug << "restarting pulse";
  start ();
}

QProcess::ProcessState PulseManager::state () {
  return (state_);
}
