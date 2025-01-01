#include <iostream>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <wait.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>

namespace Day240211
{
    struct shm_t
    {
        pthread_mutex_t mtx;
        char str[1024];
    };

    void signalHandler(int sigNum)
    {
        if (sigNum == SIGTERM)
        {
            std::cout << "[SIGTERM] SIGTERM Received. Perform cleanup & exiting\n";
            exit(0);
        }
    }

    int run1()
    {
        pthread_mutexattr_t attr;
        key_t key = ftok("smile", 65);
        int shmid = shmget(key, sizeof(shm_t), IPC_CREAT | 0666);
        if (shmid < 0)
        {
            std::cout << "shmget Error() !\n";
            return 0;
        }

        shm_t *g_shm = (shm_t *)shmat(shmid, nullptr, 0);
        if (g_shm == nullptr)
        {
            std::cout << "shmat Error()!\n";
            return 0;
        }

        if (pthread_mutexattr_init(&attr))
        {
            std::cout << "pthread_mutexattr_init error!\n";
            return 0;
        }

        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED))
        {
            std::cout << "pthread setpshareed mutexattr error!\n";
            return 0;
        }
        // g_shm->mtx의 mutex를 가지고 lock을 진행한다.

        if (pthread_mutex_init(&g_shm->mtx, &attr))
        {
            std::cout << "init mutex error!\n";
            return 0;
        }

        pid_t childId = fork();
        if (childId == 0)
        {
            signal(SIGTERM, signalHandler);

            // shmid = shmget(key, sizeof(shm_t), IPC_CREAT | 0666);
            shm_t *c_shm = (shm_t *)shmat(shmid, nullptr, 0);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            while (1)
            {
                std::cout << "[Process 2] (" << getpid() << ") : In while\n";

                pthread_mutex_lock(&c_shm->mtx);
                std::cout << "[Process 2] Get Data : " << c_shm->str;
                pthread_mutex_unlock(&c_shm->mtx);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        for (int i = 0; i < 10; ++i)
        {
            pthread_mutex_lock(&g_shm->mtx);
            sprintf(g_shm->str, "Hello shared memory %d\n", i);
            pthread_mutex_unlock(&g_shm->mtx);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (kill(childId, 0) == 0)
        {
            if (kill(childId, SIGTERM) == 0)
            {
                std::cout << "Terminate Read Process!\n";
            }
            else
            {
                if (kill(childId, SIGKILL) != 0)
                    std::cout << "Failed to Terminate Process\n";
            }
        }

        std::cout << "Finish Write Process\n";

        return 0;
    }

}