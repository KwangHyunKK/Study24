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

#include "subprocess.hpp"

#include <assert.h>
#include <stdio.h>
#include <algorithm>
#include "util.h"

namespace JinjaLearn
{
    using namespace std;

    Subprocess::Subprocess(bool use_console) : child_(nullptr), overlapped_(),
                                               is_reading_(false), 
                                               use_console_(use_console) {}

    Subprocess::~Subprocess()
    {
        if(pipe_)
        {
            if(!CloseHandle(pipe_))Win32Fatal("CloseHandle");
        }

        // Reap child if forgotten
        if(child_)Finish();
    }

    HANDLE Subprocess::SetupPipe(HANDLE ioport)
    {
        // # TODO : char to string
        // 1. Create a unique name for the pipe using the PID and this pointer.
        char pipe_name[100];
        snprintf(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\ninja_pid%lu_sp%p", GetCurrentProcessId(), this);

        // 2. Create a named pipe for inbound communication (reading data from the child process)
        pipe_ = ::CreateNamedPipeA(pipe_name,
                                   PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, // Asynchronous access
                                   PIPE_TYPE_BYTE,
                                   PIPE_UNLIMITED_INSTANCES, // Unlimited instances of the pipe
                                   0, 0, INFINITE, nullptr);

        // 3. Check for errors during pipe creation
        if(pipe_ == INVALID_HANDLE_VALUE)Win32Fatal("CreateNamedPipe");

        // 4. Associate the pipe with specified I/O completion port for asynchronous notifications.
        if(!CreateIoCompletionPort(pipe_, ioport, (ULONG_PTR)this, 0))
            Win32Fatal("CreateIOCompletionPort");

        // 5. Initialize the OVERLAPPED structure and connect the pipe asynchronously.
        memset(&overlapped_, 0, sizeof(overlapped_));
        if(!ConnectNamedPipe(pipe_, &overlapped_) && GetLastError() != ERROR_IO_PENDING)
            Win32Fatal("ConnectedNamedPipe");

        // 6. Create the write end of the pipe for the child process.

        // Get the write end of the pipe as a handle inheritable across processes.
        HANDLE output_write_handle = CreateFileA(pipe_name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        HANDLE output_write_child;

        // 7. Duplicate the handle to make it inheritable by the child process.
        if(!DuplicateHanle(GetCurrentProcess(), output_write_handle, GetCurrentProcess(), &output_write_child, 0, true, DUPLICATE_SAME_ACCESS))
            Win32Fatal("DuplicateHandle");

        // 8. Close the original handle, as only the duplicated handle is needed.
        CloseHandle(output_write_handle);

        return output_write_child;
    }

    bool SubProcess::Start(SubprocessSet* set, const string& command)
    {
        // 1. Setup a pipe for communication between the parent and child processes.
        HANDLE child_pipe = SetupPipe(set->ioport_);
        
        // 2. Set up security attributes to allow the child process to inherit handles.
        SECURITY_ATTRIBUTES security_attributes;
        memset(&security_attributes, 0, sizeof(SECURITY_ATTRIBUTES));
        security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        security_attributes.bInheritHandle = true;

        // 3. Create a handle to the null device(NUL) for standard input.
        // This is used to suppress input for the child process.

        // Must be inheritable so subprocesses can duplicate to children.
        HANDLE nul = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &security_attributes, OPEN_EXISTING, 0, nullptr);
        if(null == INVALID_HANDLE_VALUE)Fatal("Couldn't open nul");

        // 4. Initialize STARTUPINFOA to configure the child process's standard handles.
        STARTUPINFOA startup_info;
        memset(&startup_info, 0, sizeof(startup_info));
        startup_info.cb = sizeof(STARTUPINFO);
        if (!use_console_) {
            // Redirect the child process's standard I/O, and error to the pipe.
            startup_info.dwFlags = STARTF_USESTDHANDLES;
            startup_info.hStdInput = nul;  // Input is suppressed
            startup_info.hStdOutput = child_pipe; // Output redirected to the pipe
            startup_info.hStdError = child_pipe; // Error redirected to the pipe
        }

        // In the console case, child_pipe is stil inherited by the child and closed
        // when the subprocesses finishes, which then notifies ninja.

        // 5. Initialize the PROCESS_INFORMATION structure to retrieve process details.

        PROCESS_INFORMATION process_info;
        memset(&process_info, 0, sizeof(process_info));

        // 6. Setthe process creation flags.
        // CREATE_NEW_PROCESS_GROUP ensures that Ctrl + c signals are handled by the parent, not the child.

        // Ninja handles ctrl-c, except for subprocesses in console pools.
        DWORD process_flags = use_console_ ? 0 : CREATE_NEW_PROCESS_GROUP;

        // Don't prepend `cmd /c` on Windows, this breaks command lines
        // greater than 8,191 chars.
        if(!CreateProcessA(NULL, (char*)command.c_str(), nullptr, nullptr, true, process_flags, nullptr, nullptr, &startup_info, &process_info))
        {
            DWORD error = GetLastError();

            // Handle the case where the specified program is not found.
            if(error == ERROR_FILE_NOT_FOUND)
            {
                // File (program) not found error is treated as a normal build action failure.

                if(child_pipe)CloseHandle(child_pipe);
                CloseHandle(pipe_);
                CloseHandle(nul);
                pipe_ = nullptr;
                buf_ = "CreateProcess failed : The system can't find the file specified.\n";
                return true;
            }
            else
            {
                // Handle other errors and provide additional debugging information.
                fprintf(stderr, "\nCreateProcess failed. Command attempted:\n"%s\"\n", command.c_str());
                const char* hint = nullptr;

                // ERROR_INVALID_PARAMETER means the command line was formatted incorrectly.
                // This can be caused by a command line being too long or leading whitespace 
                // in the command. Give extra context for this case.
                if(error == ERROR_INVALID_PARAMETER)
                {
                    if(command.length() > 0 && (command[0] == ' ' || command[0] == '\t'))
                        hint = "command contains leading whitespace";
                    else
                        hint = "is the command line too long?";
                }
                Win32Fatal("CreateProcess", hint);
            }
        }

        // Close pipe channel only used by the child.
        if(child_pipe)CloseHandle(child_pipe);
        CloseHandle(nul);

        // Close the thread handle of the create process, as it's not needed.
        CloseHandle(process_info.hThread);
        // store the process handle to manage or terminate the child process later.
        child_ = process_info.hProcess;

        return true;
    }
}