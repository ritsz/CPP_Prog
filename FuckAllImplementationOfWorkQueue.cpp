#include <iostream>
#include <chrono>
#include <list>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#define SCAN_CONTENT "/home/ritsz/"

using namespace std;
 
class PerfTime {
    std::chrono::steady_clock::time_point clock_end, clock_start;
    string useCase;
public:
    PerfTime() : useCase("")
    {
        clock_start = chrono::steady_clock::now();
    }

    PerfTime(const string copy) : useCase(copy)
    {
        clock_start = chrono::steady_clock::now();
    }

    ~PerfTime()
    {
        clock_end = chrono::steady_clock::now();
        auto diff = clock_end - clock_start;
        cout << "[Performance Counters " << useCase.c_str() << ":] Code ran for " << chrono::duration<double, milli> (diff).count() << " ms" << endl;
    }
};

template <typename T> class wqueue
{ 
    list<T>   m_queue;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_condv;

public:
    wqueue() {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_condv, NULL);
    }
    ~wqueue() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_condv);
    }
    void add(T item) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(item);
        pthread_cond_signal(&m_condv);
        pthread_mutex_unlock(&m_mutex);
    }
    T remove() {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.size() == 0) {
            pthread_cond_wait(&m_condv, &m_mutex);
        }
        if(m_queue.size() == 0)
        {
            pthread_mutex_unlock(&m_mutex);
            return 0;
        }
        T item = m_queue.front();
        m_queue.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return item;
    }
    int size() {
        pthread_mutex_lock(&m_mutex);
        int size = m_queue.size();
        pthread_mutex_unlock(&m_mutex);
        return size;
    }
};

class WorkItem
{
    string m_message;

    public:
    WorkItem(const char* message) 
        : m_message(message) {}
    ~WorkItem() {}

    const char* getMessage() { return m_message.c_str(); }
};


class Thread
{
    public:
        Thread();
        virtual ~Thread();

        int start();
        int join();
        int detach();
        pthread_t self();

        virtual void* run() = 0;

    private:
        pthread_t  m_tid;
        int        m_running;
        int        m_detached;
};

Thread::Thread() : m_tid(0), m_running(0), m_detached(0) {}
Thread::~Thread()
{
    if (m_running == 1 && m_detached == 0) {
        pthread_detach(m_tid);
    }
    if (m_running == 1) {
        pthread_cancel(m_tid);
    }
}

static void* runThread(void* arg);

int Thread::start()
{
    int result = pthread_create(&m_tid, NULL, runThread, this);
    if (result == 0) {
        m_running = 1;
    }
    return result;
}

static void* runThread(void* arg)
{
    return ((Thread*)arg)->run();
}

int Thread::join()
{
    int result = -1;
    if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 1;
        }
    }
    return result;
}

int Thread::detach()
{
    int result = -1;
    if (m_running == 1 && m_detached == 0) {
        result = pthread_detach(m_tid);
        if (result == 0) {
            m_detached = 1;
        }
    }
    return result;
}

pthread_t Thread::self() {
    return m_tid;
}

class ConsumerThread : public Thread
{
    wqueue<WorkItem*>& m_queue;

    public:
    ConsumerThread(wqueue<WorkItem*>& queue) : m_queue(queue) {}

    void* run() {
        for (int i = 0;; i++) {
            WorkItem* item = (WorkItem*)m_queue.remove();
            struct stat stat_buf;
            stat(item->getMessage(), &stat_buf);
            cout<<"STAT : " << item->getMessage() << endl;
            delete item;
        }
        return NULL;
    }
};


int main()
{
    struct dirent prevEntry;
    struct dirent *entry;
    wqueue<WorkItem*>  queue;
    WorkItem *item;
    DIR *dir = opendir(SCAN_CONTENT);
    
    if(!dir)
    {
        cout << strerror(errno) << endl;
        return -1;
    }

    ConsumerThread* thread1 = new ConsumerThread(queue);
    ConsumerThread* thread2 = new ConsumerThread(queue);
    
    int fd = open(".", O_DIRECTORY);
    chdir(SCAN_CONTENT);

    PerfTime("WorkQueue Scan");
    thread1->start();
    thread2->start();
    while(readdir_r(dir, &prevEntry, &entry) == 0)
    {
        if(!entry)
            continue;
        item = new WorkItem(entry->d_name);
        queue.add(item);
        prevEntry = *entry;
    }
    thread1->join();
    thread2->join();

    cout<<"Enter Ctrl-C to end the program...\n";
    fchdir(fd);
}
