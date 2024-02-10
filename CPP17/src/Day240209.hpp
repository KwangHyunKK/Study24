#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

namespace Day240209
{
    namespace Prac1
    {

        int run1()
        {
            // ftok to gen unique key
            key_t key = ftok("shmfile", 65);

            // shmget returns an identifier in shmid
            int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

            // shmat to attach to shared mem
            char *str = (char *)shmat(shmid, (void *)0, 0);

            std::cout << "Write Data : "
                      << "Hello world\n";
            str = "Hello, World\n";

            std::cout << "Data written in memory : " << str << "\n";

            pid_t childId = fork();

            if (childId == 0)
            {
                key_t key = ftok("shmfile", 65);

                int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

                char *str = (char *)shmat(shmid, (void *)0, 0);

                std::cout << "Data read from memory: " << str;

                shmdt(str);

                shmctl(shmid, IPC_RMID, NULL);

                std::cout << "Finish Read process!\n";
            }

            sleep(1);

            shmdt(str);

            if (kill(childId, 0) == 0)
            {
                if (kill(childId, SIGKILL) == 0)
                {
                    std::cout << "Terminate Read Process!\n";
                }
                else
                {
                    std::cout << "Failed to Terminate Process\n";
                }
            }

            std::cout << "Finish Write Process\n";

            return 0;
        }

        int run2()
        {
        }

    }
}