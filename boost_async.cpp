#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <queue>
#include <mutex>
#include <boost/asio.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define SCAN_CONTENT "/home/ritsz/testScan"

using namespace std;
using namespace boost;

std::atomic<std::uint_least64_t> fileread(0);

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

template <typename element>
class WorkQueue {
    queue<element> m_workqueue;
    mutex mtx;
public:
    
    WorkQueue() {}
    virtual ~WorkQueue() {}

    void addWork(const element& object)
    {
        m_workqueue.push(object);
    }
    
    void getWork(element& object)
    {
        mtx.lock();
        if(m_workqueue.size() > 0)
        {
            object = m_workqueue.front();
            m_workqueue.pop();
        }
        mtx.unlock();
    }

    size_t workSize()
    {
        mtx.lock();
        size_t size = m_workqueue.size();
        mtx.unlock();
        return size;
    }

    bool workPresent()
    {
        return (workSize() > 0);
    }

    bool hasProducerFinished()
    {
        return done;
    }

    void setDone()
    {
        done = true;
    }
    
    bool done;
};

WorkQueue<string> workQueue;

class DIR_Wrapper
{
    DIR *m_dir;
    string m_dirName;
    error_t errCache;
    struct dirent prevEntry;
    struct dirent *entry;
    string m_childFile;
    unsigned long numFiles;
    int m_savedfd;
 public:
    DIR_Wrapper(const string &name)
    {
        numFiles = 0;
        m_dirName = name;
        m_dir = opendir(m_dirName.c_str());
        errCache = errno;

        if(!errno)
        {
            m_savedfd=open(".", O_DIRECTORY);
            if(m_savedfd <= 0)
                errCache = errno;
            else
            {
                if(chdir(m_dirName.c_str()))
                    errCache = errno;
            }
        }

    }

    ~DIR_Wrapper()
    {
        if(m_dir != NULL)
            closedir(m_dir);

        if(m_savedfd > 0)
            fchdir(m_savedfd);
    }

    bool IsOpened()
    {
        return (m_dir != NULL);
    }

    string ErrorAsString()
    {
        cout<<"FILE: " << m_childFile << endl;
        return strerror(errCache);
    }

    int GetNextEntry()
    {
        int ret = readdir_r(m_dir, &prevEntry, &entry);
        if(ret)
        {
            cout << "Readdir failure." << endl;
            errCache = errno;
            return ret;
        }
        
        if (!entry)
            return -1;
        m_childFile = entry->d_name;
        workQueue.addWork(m_childFile);
        //struct stat buf;
        //stat(m_childFile.c_str(), &buf);
        //fileread++;
        prevEntry = *entry;
        numFiles++;
        return 0;
    }

    string FileName() { return m_childFile;}
    unsigned long NumFiles() { return numFiles; }
};

void Producer()
{
    DIR_Wrapper scanner(SCAN_CONTENT);
    cout<<SCAN_CONTENT << " is opened." << endl;

    while(scanner.GetNextEntry() == 0)
        continue;

    cout<<"Added number of files: " << scanner.NumFiles() << endl;
            workQueue.setDone();
            cout<<"Producer said done" <<endl;
}

void Consumer()
{
    cout<<"Consuming with :"<< std::this_thread::get_id() << endl;
    struct stat buf;
    while(!workQueue.hasProducerFinished() ||  workQueue.workPresent())
    {
        string file;
        workQueue.getWork(file);
        if(!file.empty())
        {
            stat(file.c_str(), &buf);
            fileread++;
        }
        else 
        {
            if(workQueue.hasProducerFinished())
                break;
            if(!workQueue.workPresent())
                std::this_thread::yield();
        }
    }
}

int main()
{
    PerfTime timer;
    asio::io_service consumers;
    asio::io_service producer;

    producer.post(Producer);
    consumers.post(Consumer);
    consumers.post(Consumer);

    std::thread producer_thread( [&] {producer.run();} );
    std::thread consumer_thread( [&] {consumers.run();} );
    std::thread consumer_thread2( [&] {consumers.run();} );

    producer_thread.join();
    consumer_thread.join();
    consumer_thread2.join();
    cout<<"Read files : " << fileread << endl;
    cout<<"DONE"<<endl;
}
