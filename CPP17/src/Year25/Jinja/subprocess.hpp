// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// To Run jinja

// subprocess.h
#ifndef NINJA_SUBPROCESS_H_
#define NINJA_SUBPROCESS_H_

#include <string>
#include <vector>
#include <queue>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

// ppoll() exists on FreeBSD, but only on newer verisons.
#ifdef __FreeBSD__
# include <sys/param.h>
# if defined USE_PPOLL && __FREEBSD_version < 10020000
#     undef USE_PPOLL
# endif 
#endif

#include "exit_status.h"

/// Subprocess declaration
/// Subprocess wraps a single async subprocess. It is entirely
/// passive : it expects the caller to notify it when its fds are ready
/// for reading, as well as call Finish() to reap the child once done()
/// is true.

/// ########### My Opinion ############

/// The Subprocess class wraps a single asynchronous subprocess and is designed to operate passively.
/// It relies on the caller to :
/// 1. Notify about fd(file descriptor) readiness : The caller must explicitly inform the Subprocess when its file descriptors
/// are ready for reading or writing.
/// 2. Invoke Finish() to clean up : Once the subprocess indicates completion via the done() method,
/// the caller is responsible for calling the Finish() method to reap the child process and release resources.
/// This design is entirely passive, meaning the Subprocess doesn't actively monitor or 
/// manage the subprocess on its own. Instead, it integrates with external event loops (e.g., epoll or select)
/// and requires the caller to handle all state updates explicitly. This approach is often used 
/// to optimize performance in asynchronous environments.

/// ########### My Opinion ############

namespace JinjaLearn
{
    class Subprocess
    {
    public:
        ~Subprocess();

        /// @brief 
        /// @return successful process exit -> ExitSuccess
        ///         interrupted -> Exitinterrupted
        ///         failure -> ExitFailure  
        ExitStatus Finish();

        bool Done() const;

        const std::string& GetOutput() const;
    private:
        Subprocess(bool use_console);
        bool Start(SubprocessSet* set, const std::string& command);
        void OnPipeReady();

        std::string buf_;

    #ifdef _WIN32
        /// Setup pipe_ as the parent-side pipe of the subprocess;
        /// return the other end of the pipe, usable in the child process.
        HANDLE SetupPipe(HANDLE ioport);

        HANDLE child_;
        HANDLE pipe_;
        OVERLAPPED overlapped_;
        char overlapped_buf_[4 << 10]; // 2 << 12
        bool is_reading_;
    #else  
        int fd_;
        pid_t pid_;
    #endif
        bool use_console_;

        friend class SubprocessSet;
    };

    /// SubprocessSet runs a ppoll/pselect() loop around a set of Subprocesses.
    /// DoWork() waits for any state change in subprocesses; finished_
    /// is a queue of subprocesses as they finish.
    class SubprocessSet
    {
    public:
        SubprocessSet();
        ~SubprocessSet();

        Subprocess* Add(const std::string& command, bool use_console = false);
        bool DoWork();
        Subprocess* NextFinished();
        void Clear();

        std::vector<Subprocess*> running_;
        std::queue<Subprocess*> finished_;

        #ifdef _WIN32
            static BOOL WINAPI NotifyInterrupted(DWORD dwCtrlType);
            static HANDLE ioport_;
        #else   
            static void SetInterruptedFlag(int signum);
            static void HandlePendingInterruption();

            /// Store the signal number that causes the interruption.
            /// 0 if not interruption.
            static int interrupted_;

            constexpr bool IsInterrupted() {return interrupted_ != 0; }

            struct sigaction old_int_act_;
            struct sigaction old_term_act_;
            struct sigaction old_hup_act_;
            sigset_t old_mask_;
        #endif
    };
}

#endif // NINJA_SUBPROCES_H_

