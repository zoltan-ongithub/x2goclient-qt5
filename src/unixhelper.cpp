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

/* Necessary for Q_OS_UNIX! */

#include "unixhelper.h"

#ifdef Q_OS_UNIX

#include <csignal>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <cstdlib>
#include <cstdio>

/* For documentation please see unixhelper.h. */

namespace unixhelper {
  void kill_pgroup (const int signal) {
    pid_t pgid_to_kill = getpgrp ();

    if ((SIGHUP == signal) || (-1 == signal)) {
      /*
       * In order to not kill ourselves, we need to run this
       * code in a new process group.
       */
      pid_t tmp_pid = fork ();

      /* Child. */
      if (0 == tmp_pid) {
        /* Create new pgid. */
        int err = setpgid (0, 0);

        if (0 != err) {
          std::perror ("WARNING: unable to change PGID");
          std::cerr << "Continuing with normal operation, but process might kill itself before tree vanishes." << std::endl;
        }

        real_kill_pgroup (pgid_to_kill);
      }
      /* Error. */
      else if (-1 == tmp_pid) {
        std::perror ("WARNING: unable to fork off another process to kill original process group");
        std::cerr << "Proceeding with normal operation, but process might kill itself before tree vanishes." << std::endl;

        real_kill_pgroup (pgid_to_kill);
      }
      /* Parent. */
      else {
        /*
         * No need to do anything, just exit here in order to not
         * spawn a bunch of new processes due to subsequent calls
         * to kill_pgroup () from unix_cleanup ().
         */
        std::exit (EXIT_SUCCESS);
      }
    }
  }

  void real_kill_pgroup (const pid_t pgid) {
    /* Try to kill via SIGTERM first. */
    if (0 != killpg (pgid, SIGTERM)) {
      const int saved_errno = errno;
      std::cerr << "WARNING: unable to send SIGTERM to process group '" << pgid << "': " << std::strerror (saved_errno) << std::endl;
    }

    /* Grant a grace period of (at least) 10 seconds. */
    sleep (10);

    int kill_ret = killpg (pgid, SIGKILL);

    /*
     * Might be unreachable.
     * Depending upon which pgroup we just killed, this
     * code is either unreachable (because killpg () killed
     * itself already), or being executed.
     * Let's handle errors and exit, if necessary.
     */
    if (0 != kill_ret) {
      const int saved_errno = errno;
      std::cerr << "WARNING: failed to kill process group '" << pgid << "': " << std::strerror (saved_errno) << std::endl;
    }

    std::exit (EXIT_SUCCESS);
  }

  int unix_cleanup (const pid_t parent) {
    /*
     * Unblock all signals first.
     * Signal blocks are inherited, so you never you what is currently set.
     */
    sigset_t empty_set;
    if (0 != sigemptyset (&empty_set)) {
      const int saved_errno = errno;
      std::cerr << "Unable to fetch empty signal set: " << std::strerror (saved_errno) << std::endl;
      kill_pgroup (-1);

      /* Anything here shall be unreachable. */
    }

    if (0 != sigprocmask (SIG_SETMASK, &empty_set, NULL)) {
      const int saved_errno = errno;
      std::cerr << "Unable to set empty signal set: " << std::strerror (saved_errno) << std::endl;
      kill_pgroup (-1);

      /* Anything here shall be unreachable. */
    }

    std::vector<int> ignore_signals;
    ignore_signals.push_back (SIGINT);
    ignore_signals.push_back (SIGTERM);
    ignore_signals.push_back (SIGPIPE);
    ignore_signals.push_back (SIGQUIT);
    ignore_signals.push_back (SIGUSR1);
    ignore_signals.push_back (SIGUSR2);

    for (std::vector<int>::iterator it = ignore_signals.begin (); it != ignore_signals.end (); ++it) {
      struct sigaction sig_action;
      sig_action.sa_handler = SIG_IGN;
      sig_action.sa_mask = empty_set;
      sig_action.sa_flags = SA_RESTART;

      /* Set up signal handler to ignore the current signal. */
      if (0 != sigaction (*it, &sig_action, NULL)) {
        const int saved_errno = errno;
        std::cerr << "Unable to ignore signal " << strsignal (*it) << ": " << std::strerror (saved_errno) << std::endl;
        kill_pgroup (-1);

        /* Anything here shall be unreachable. */
      }
    }

    {
      struct sigaction sig_action;
      sig_action.sa_handler = kill_pgroup;
      sig_action.sa_mask = empty_set;
      sig_action.sa_flags = SA_RESTART;

      if (0 != sigaction (SIGHUP, &sig_action, NULL)) {
        const int saved_errno = errno;
        std::cerr << "Unable to set up signal handler for SIGHUP: " << std::strerror (saved_errno) << std::endl;
        kill_pgroup (-1);

        /* Anything here shall be unreachable. */
      }
    }

    /* Sleep forever... at least two seconds in each run. */
    for (;;) {
      pid_t cur_ppid = getppid ();

      /* cur_ppid should match parent, otherwise the parent died. */
      if (cur_ppid != parent) {
        kill_pgroup (SIGHUP);
      }

      sleep (2);
    }

    /*
     * Anything here shall be unreachable.
     * But make compilers happy by returning something.
     */
    return (EXIT_SUCCESS);
  }
}

#endif /* defined (Q_OS_UNIX) */
