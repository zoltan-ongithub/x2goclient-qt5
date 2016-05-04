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

#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H

#ifdef NDEBUG
#define NDEBUG_DEFINE
#undef NDEBUG
#endif // defined (NDEBUG)

#include <iostream>
#include <QDir>
#include <QTcpSocket>
#include <QEventLoop>
#include <QTemporaryFile>
#include <QHostInfo>
#include <QProcess>
#include <QApplication>
#include <QTimer>
#include <assert.h>
#include <cstdint>

#include "x2gosettings.h"

class PulseManager: public QObject {
  Q_OBJECT;

  public:
    PulseManager ();
    ~PulseManager ();

    QProcess::ProcessState state ();

    std::uint16_t get_pulse_port ();
    std::uint16_t get_esd_port ();
    bool get_record ();
    bool get_playback ();
    QDir get_pulse_dir ();

    bool set_pulse_port (std::uint16_t pulse_port);
    bool set_esd_port (std::uint16_t esd_port);
    bool set_record (bool record);
    bool set_playback (bool playback);


  public slots:
    void start ();
    void restart ();


  private:
    PulseManager (const PulseManager &other);

    void start_osx ();
    void start_win ();
    // FIXME
    void start_linux ();
    void start_generic ();

    void fetch_pulseaudio_version ();

    bool find_port (bool search_esd = false);

    bool generate_server_config ();
    bool generate_client_config ();

    void create_client_dir ();
    void cleanup_client_dir ();

    void shutdown ();

    bool is_server_running ();


  private slots:
    void on_pulse_finished (int exit_code);
    void slot_play_startup_sound ();


  signals:
    void sig_pulse_server_terminated ();


  private:
    QString app_dir_;
    QString pulse_X2Go_;
    QDir pulse_dir_;
    QString server_binary_;
    QString server_working_dir_;

    QProcessEnvironment env_;
    QStringList server_args_;
    QProcess *pulse_server_;
    QProcess::ProcessState state_;

    std::uint16_t pulse_port_;
    std::uint16_t esd_port_;

    std::uint32_t pulse_version_major_;
    std::uint32_t pulse_version_minor_;
    std::uint32_t pulse_version_micro_;
    QString pulse_version_misc_;

    bool record_;
    bool playback_;
};

#ifdef NDEBUG_DEFINE
#define NDEBUG
#undef NDEBUG_DEFINE
#endif // defined (NDEBUG_DEFINE)

#endif // PULSEMANAGER_H
