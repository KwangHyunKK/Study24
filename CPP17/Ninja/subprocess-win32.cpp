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

namespace NinjaLearn
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
    /// @brief 
    /// 1.Named Pipe Creation (Step 2)
    /// The parent creates a named pipe to communicate with the child process. 
    /// The pipe allows for biderecitonal data transfer, enabling the parent to monitor or communicate with the child.

    /// 2. Handle Duplication (Step 7)
    /// THe write handle of the pipe is duplicated to make it inheritable by the child process.
    /// This ensures the child can send data to the parent process.
    /// @param ioport 
    /// @return 
    HANDLE Subprocess::SetupPipe(HANDLE ioport)
    {
        // # TODO : char to string
        // Step 1. Create a unique name for the pipe using the PID and this pointer.
        char pipe_name[100];
        snprintf(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\ninja_pid%lu_sp%p", GetCurrentProcessId(), this);

        // Step 2. Create a named pipe for inbound communication (reading data from the child process)
        pipe_ = ::CreateNamedPipeA(pipe_name,
                                   PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, // Asynchronous access
                                   PIPE_TYPE_BYTE,
                                   PIPE_UNLIMITED_INSTANCES, // Unlimited instances of the pipe
                                   0, 0, INFINITE, nullptr);

        // Step 3. Check for errors during pipe creation
        if(pipe_ == INVALID_HANDLE_VALUE)Win32Fatal("CreateNamedPipe");

        // Step 4. Associate the pipe with specified I/O completion port for asynchronous notifications.
        if(!CreateIoCompletionPort(pipe_, ioport, (ULONG_PTR)this, 0))
            Win32Fatal("CreateIOCompletionPort");

        // Step 5. Initialize the OVERLAPPED structure and connect the pipe asynchronously.
        memset(&overlapped_, 0, sizeof(overlapped_));
        if(!ConnectNamedPipe(pipe_, &overlapped_) && GetLastError() != ERROR_IO_PENDING)
            Win32Fatal("ConnectedNamedPipe");

        // Step 6. Create the write end of the pipe for the child process.

        // Get the write end of the pipe as a handle inheritable across processes.
        HANDLE output_write_handle = CreateFileA(pipe_name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        HANDLE output_write_child;

        // Step 7. Duplicate the handle to make it inheritable by the child process.
        if(!DuplicateHanle(GetCurrentProcess(), output_write_handle, GetCurrentProcess(), &output_write_child, 0, true, DUPLICATE_SAME_ACCESS))
            Win32Fatal("DuplicateHandle");

        // Step 8. Close the original handle, as only the duplicated handle is needed.
        CloseHandle(output_write_handle);

        return output_write_child;
    }

    /// @brief 
    /// 1. I/O Redirection (Step 4)
    /// The child process's standard input, output and error streams are redirected to the pipe, allowing the parent to capture the child's output and errors.
    /// 2. Asynchronous Operaitons
    /// The pipe is created with `FILE_FLAG_OVERLAPPED` flag to supprot asynchronous I/O operations, improving performance and resposniveness.
    /// 3. Error Handling 
    /// The code includes detailed error checks and provides hints for common issues like invalid command lines or missing files.
    /// @param set 
    /// @param command 
    /// @return 
    bool SubProcess::Start(SubprocessSet* set, const string& command)
    {
        // Step 1. Setup a pipe for communication between the parent and child processes.
        HANDLE child_pipe = SetupPipe(set->ioport_);
        
        // Step 2. Set up security attributes to allow the child process to inherit handles.
        SECURITY_ATTRIBUTES security_attributes;
        memset(&security_attributes, 0, sizeof(SECURITY_ATTRIBUTES));
        security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        security_attributes.bInheritHandle = true;

        // Step 3. Create a handle to the null device(NUL) for standard input.
        // This is used to suppress input for the child process.

        // Must be inheritable so subprocesses can duplicate to children.
        HANDLE nul = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &security_attributes, OPEN_EXISTING, 0, nullptr);
        if(null == INVALID_HANDLE_VALUE)Fatal("Couldn't open nul");

        // Step 4. Initialize STARTUPINFOA to configure the child process's standard handles.
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

        // Step 5. Initialize the PROCESS_INFORMATION structure to retrieve process details.

        PROCESS_INFORMATION process_info;
        memset(&process_info, 0, sizeof(process_info));

        // Step 6. Setthe process creation flags.
        // CREATE_NEW_PROCESS_GROUP ensures that Ctrl + c signals are handled by the parent, not the child.

        // Ninja handles ctrl-c, except for subprocesses in console pools.
        DWORD process_flags = use_console_ ? 0 : CREATE_NEW_PROCESS_GROUP;

        // Don't prepend `cmd /c` on Windows, this breaks command lines
        // greater than 8,191 chars.
        // Step 7. Check CreateProcess returns true
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

        // Step 8. If no errors occur, close pipe channel and thread handle.
        // Close pipe channel only used by the child.

        if(child_pipe)CloseHandle(child_pipe);
        CloseHandle(nul);

        // Close the thread handle of the create process, as it's not needed.
        /// The process's handle (hProcess) is maintained by the parent because 
        /// it is used to manage or terminate the child process, 
        /// but the thread handle (hThread) is closed because the parent has no direct use.
        CloseHandle(process_info.hThread);
        // store the process handle to manage or terminate the child process later.
        /// Stored handles allow parents to manage or terminate the child process.
        child_ = process_info.hProcess;

        return true;
    }

    void Subprocess::OnPipeReady()
    {
        DWORD bytes;
        
        // Check if the aysnchronous operation (reading from the pipe) completed successfully.
        if(!GetOverlappedResult(pipe_, &overlapped_, &bytes, true))
        {
            // Handle broken pipe error, which indicates that the child process has closed its write handle.
            if(GetLastError() == ERROR_BROKEN_PIPE)
            {
                CloseHandle(pipe_); // Close the pipe handle as it's no longer usable.
                pipe_ = nullptr;
                return;
            }
            Win32Fatal("GetOverlappedResult"); // Handle unexpected errors.
        }

        // If reading is in progress and data is available, append it to the buffer.
        if(is_reading_ && bytes)
        {
            buf_.append(overlapped_buf_, bytes); // Append read bytes to the output buffer.
        }

        // Reset the OVERLAPPED structure for the next asynchonous read.
        memset(&overlapped_, 0, sizeof(overlapped_)); // Can be changed c++ sytle?
        is_reading_ = true;
        // Initiate a new asynchronous read operation on the pipe.
        if(!::ReadFile(pipe_, overlapped_buf_ sizeof(overlapped_buf_), &bytes, &overlapped_))
        {
            if(GetLastError() == ERROR_BROKEN_PIPE)
            {
                CloseHandle(pipe_);
                pipe_ = nullptr;
                return;
            }

            // Handle other errors except for the expected ERROR_IO_PENDING.
            if(GetLastError() != ERROR_IO_PENDING)
                Win32Fatal("ReadFile");
        }

        // Even if we read any bytes in the readfile call, we'll enter this
        // funciton again later and get them at that point.
    }

    ExitStatus Subprocess::Finish()
    {
        if (!child_) return ExitFailure; // If there is no child process, return a failure status.

        // Wait indefinitely for the child process to finish execution.
        WaitForSingleObject(child_, INFINITE);

        DWORD exit_code = 0;
        // Retrieve the exit code of the child process.
        GetExitCodeProcess(child_, &exit_code);

        // Close the handle to the child process and clean up resources.
        CloseHandle(child_);
        child_ = nullptr;

        // Return the exit code, mapping CONTROL_C_EXIT to an interrupted status.
        return exit_code == CONTROL_C_EXIT ? ExitInterrupted : static_cast<ExitStatus>(exit_code);
    }

    bool Subprocess::Done() const{
        return pipe_ == nullptr;
    }

    const string& Subprocess::GetOutput() const
    {
        return buf_; 
    }

    HANDLE SubprocessSet::ioport_;

    SubprocessSet::SubprocessSet()
    {
        ioport_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
        if(!ioport_)
            Win32Fatal("CreateIoCompletionPort");
        // Register a handler to handle CTRL+C and CTRL+BREAK signals
        if(!SetConsoleCtrlHandler(NotifyInterrupted, true))
            Win32Fatal("SetConsoleCtrlHandler");
    }

    SubprocessSet::~SubprocessSet()
    {
        Clear();

        SetConsoleCtrlHandler(NotifyInterrupted, false);
        CloseHandle(ioport_);
    }

    bool WINAPI SubprocessSet::NotifyInterrupted(DWORD dwCtrlType)
    {
        if(dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
        {
            if(!PostQueedCompletionStatus(ioport_, 0, 0, NULL))
                Win32Fatal("PostQueuedCompletionStatus");
            return true;
        }

        return false;
    }

    Subprocess *Subprocess::Add(const string& command, bool use_console)
    {
        Subprocess *subprocess = new Subprocess(use_console);
        if(!subprocess->Start(this, command))
        {
            delete subprocess;
            return 0;
        }
        if(subprocess->child_)running_.push_back(subprocess);
        else  finished_.push(subprocess);
        return subprocess;
    }

    bool SubprocessSet::Dowork()
    {
        DWORD bytes_read;
        Subprocess* subproc;
        OVERLAPPED* overlapped;

        if(!GetqueuedCompletionStatus(ioport_, &bytes_read, (PULONG_PTR)&subproc, &overlapped, INFINITE))
        {
            if(GetLastError() != ERROR_BROKEN_PIPE)
            {
                Win32Fatal("GetQueuedCompletionStatus");
            }
        }

        if(!subproc)return true;

        subproc->OnPipeReady();

        if(subproc->Done())
        {
            vector<Subprocess*>::iterator end = remove(running_.begin(), running_.end(), subproc);
            if(running_.end() != end)
            {
                finished_.push(subproc);
                running_.resize(end - running_.begin());
            }
        }
        return false;
    }

    Subprocess* Subprocesset::NextFinished()
    {
        if(finished_.empty())
        {
            return NULL;
        }
        Subprocess* subproc = finished_.front();
        finished_.pop();
        return subproc;
    }

    void SubprocessSet::Clear()
    {
        for(vector<Subprocess*>::iterator i = running_.begin()i!=running_.end();++i)
        {
            if((*i)->child && !(*i)->use_console_)
            {
                if(!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, GetProcessId((*i)->child_)))
                    Win32Fatal("GeneratedConsoleCtrlEvent");
            }
        }
        for (vector<Subprocess*>::iterator i = running_.begin();i != running_.end(); ++i)
            delete *i;
        running_.clear();
    }
}