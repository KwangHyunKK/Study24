#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>

namespace Day240128
{
    void run1()
    {
        const int SIZE = 4096;
        const char* name = "OS";

        int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, SIZE);


        void *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

        pid_t pid = fork();

        if(pid == -1)
        {
            std::cerr << "Fork failed\n";
            return;
        }
        else if(pid > 0)
        {
            const char* msg = "Hello From Parent!";
            memcpy(ptr, msg, sizeof(msg));
            wait(nullptr);
        }
        else
        {
            sleep(2);
            std::cout << "Child reads : " << static_cast<char*>(ptr) << "\n";
        }

        shm_unlink(name);
        return;
    }
}