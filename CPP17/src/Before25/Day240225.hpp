// 이 프로그램이 어떤 운영체제에서 사용되는지
// unique_ptr
// shared_ptr
// optional
// regex
// chrono
// thread
#include <iostream>
#include <thread>
#include <memory>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <regex>
#include <optional>
#include <condition_variable>
#include <vector>
#include <algorithm>
#include <cstring>
#include <functional>
using namespace std;

namespace Day240225
{

    int check;
    condition_variable cv;
    mutex m;
    void run1()
    {
        std::cout << "Hello world!\n";
        std::vector<int> v(10);
        for_each(begin(v), end(v), [](int& c){ std::cout << c + 10 << " "; });
        return;
    }

    int thread1(int thread_id)
    {
        int t = 0;
        std::cout << "Thread 1 is : " << std::this_thread::get_id() << "\n";
        while(1)
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait_for(lk, std::chrono::milliseconds(10), [&](){return (check == -1 | check == thread_id); });

            if(check == -1)
            {
                m.unlock();
                std::cout << "Thread id : " << std::this_thread::get_id() << "will be terminated.\n";
                return 0;
            }            
            std::cout << "Thread id : " << std::this_thread::get_id() << " is running : b" << ++t << "\n";
        }
    }

    int thread2(int thread_id)
    {
        int v = 0;
        std::cout << "Thread 2 is : " << std::this_thread::get_id() << "\n";
        for(int i=0;i<10;++i)
        {
            std::cout << "Thread id : " << std::this_thread::get_id() << " is running : A" << ++v << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
        return 0;
    }

    void run2()
    {
        std::cout << "Hi\n";
        condition_variable cv;
        mutex m;

        std::thread t1(thread1, 1);
        std::thread t2(thread2, 2);

        for(int i=0;i<10;++i)
        {
            std::cout << "main thread : " << i << "\n";
            m.lock();
            check = 1;
            m.unlock();
            cv.notify_all();
        }
        m.lock();
        check = -1;
        m.unlock();
        cv.notify_all();

        t1.join();
        t2.join();

        std::cout << "End\n";
    }

    class MString
    {
    public:
        MString(const char* n)
            :data(nullptr), length(0)
        {
            SetString(n);
        }
        MString(const MString& my);
        ~MString();

        MString& operator=(const MString& other);
        void SetString(const char* n);
        const char* GetString() const;
        int GetLength() const;

    private:
        std::unique_ptr<char> data; // 동적 할당한 메모리를 가리키는 포인터
        int length;
    };

    MString::MString(const MString& other)
        :data(nullptr), length(0)
        {
            SetString(other.GetString());
        }
    MString::~MString()
    {
        if(nullptr != data)
        {
            data.reset(nullptr);
            length = 0;
        }
    }
    MString& MString::operator=(const MString& other)
    {
        if(this != &other)
        {
            data.reset();
            SetString(other.GetString());
        }
        return *this;
    }

    void MString::SetString(const char* n)
    {
        if(n == nullptr)return;

        int mLength = strlen(n);
        if(mLength == 0) return;

        data = std::make_unique<char>(mLength + 1);
        std::strcpy(data.get(), n);
        length = mLength;
        return;
    }

    const char* MString::GetString() const{
        return data.get();
    }

    int MString::GetLength() const{
        return length;
    }

    void display(MString& obj){
        std::cout << obj.GetString() << "\n";
    }

    int run3()
    {
        MString s1("Hello, World 2024!\n");
        display(s1);
        MString obj = s1;
        return 0;
    }

    class Album;

    class Song
    {
        std::string title_;
        std::string artist_;
        std::weak_ptr<Album> ptr;

    public:
        Song(std::string title, std::string artist, std::shared_ptr<Album>& ab);
        ~Song();
        const char* GetAlbum() const;
        const char* GetTitle() const;
        const char* GetArtist() const;       
    };

    class Album
    {
        std::string name_;
        std::vector<std::shared_ptr<Song>> ptr;
        friend const char* Song::GetAlbum() const;
    public:
        Album(const std::string& name) : name_(name) {}
        ~Album()
        {
            for(auto& p : ptr) {p.reset();}
        }

        const char* GetAlbum() const 
        {
            return name_.data();
        }

        void SetSong(const std::shared_ptr<Song>& sp){
            ptr.push_back(std::move(sp));
        }

        void GetSong()
        {
            for(auto& p : ptr)
            {
                std::cout << "Playing " << p->GetTitle() << " by " << p->GetArtist() << ", use count : " << p.use_count() << "\n";
            }
        }
    };

    Song::Song(std::string title, std::string artist, std::shared_ptr<Album>& ab)
        : title_(title), artist_(artist), ptr(ab)
    {
        cout << "Creator\n";
    }

    Song::~Song()
    {
        ptr.reset();
        std::cout << "Deletor\n";
    }

    const char* Song::GetAlbum() const
    {
        if(const std::shared_ptr<Album>& p = ptr.lock())return p->name_.c_str();
        return nullptr;
    }

    const char* Song::GetTitle() const
    {
        return title_.data();
    }

    const char* Song::GetArtist() const{
        return artist_.data();
    }

    int run5()
    {
        std::shared_ptr<Album> ab = std::make_shared<Album>("베토벤 1집");
        std::shared_ptr<Song> sp0(new Song("Ode to Joy", "베토벤", ab));
        ab->SetSong(sp0);
        ab->SetSong(std::make_shared<Song>("Yesterday", "비틀즈", ab));
        std::shared_ptr<Song> sp2(new Song("Blackbird", "비틀즈", ab));
        ab->SetSong(sp2);
        ab->SetSong(std::make_shared<Song>("GreensLeeves", "영국 전통 민요" , ab));

        std::shared_ptr<Song> sp4(nullptr);
        sp4 = std::make_shared<Song>("I'm still Standing", "엘튼 존", ab);
        ab->SetSong(sp4);

        ab->GetSong();
        return 0;
    }

    class EventPublisher
    {
    public:
        using EventHandler = std::function<void(int)>; // 이벤트 핸들러 타입 정의

        void subscribe(const EventHandler& handler)
        {
            listeners.push_back(handler);
        }

        void publish(int data)
        {
            for(auto& listener : listeners){
                listener(data);
            }
        }
    private:
        std::vector<EventHandler> listeners;
    };
}