#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <ctime>
#include <ratio>
#include <vector>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#define SCAN_CONTENT "/home/ritsz/testScan"

unsigned long statedFile = 0;
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

std::mutex mtx;
vector<std::string> future_stat;

struct stat myStat(const string &fileName)
{
    struct stat statBuf;
    int ret = stat(fileName.c_str(), &statBuf);
    if(ret)
    {
        cout << "Stat failure" << endl;
        exit(0);
    }
    return statBuf;
}

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
        prevEntry = *entry;
        return 0;
    }

    void doStatInFuture()
    {
        future_stat.push_back(m_childFile);
        numFiles++;
    }

    string FileName() { return m_childFile;}
    unsigned long NumFiles() { return numFiles; }
};

void doParallelStats()
{
    size_t size = 0;
    
    while (true){
        mtx.lock();
        size = future_stat.size();
        if(size == 0)
        {
            mtx.unlock();
            return;
        }
        string ftr = std::move(future_stat.back());
        future_stat.pop_back();
        statedFile++;
        mtx.unlock();
        myStat(ftr);
    }
}

int main()
{
    future_stat.reserve(100000);
    DIR_Wrapper scanContent(SCAN_CONTENT);
    if (!scanContent.IsOpened())
    {
        cout << "[Error]: " << scanContent.ErrorAsString() << endl;
        return -1;
    }
    else
    {
        cout << SCAN_CONTENT << " is opened." << std::endl;
        while(scanContent.GetNextEntry() == 0)
        {
            scanContent.doStatInFuture();
        }

        
        PerfTime pt("Basic Scan");
        vector<std::thread> thread_pool;
        for(int i = 0; i < 64; i++)
        {
            thread_pool.push_back(std::thread(doParallelStats));
        }

        for(auto &thread: thread_pool)
            thread.join();
         
        //for(string file : future_stat)
        //    myStat(file);
        cout<<"Completed Files : " << scanContent.NumFiles() << " vs " << statedFile << " " << scanContent.ErrorAsString() << endl;
    }
}
