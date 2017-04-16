#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <mutex>

using namespace std;

class time_stat {
  std::chrono::time_point<std::chrono::system_clock> start, end;
public:
  time_stat(){
    start = std::chrono::system_clock::now();
  }

  ~time_stat() {
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "  elapsed time: " << elapsed_seconds.count() << "s\n";
  }
};


template <typename T>
class lock_free_stack {
private:
/* Data structure the represent a node */
  std::mutex m_mutex;
  struct node {
    T data;
    node* next;

    node (T const& _data) {
      data = _data;
      next = NULL;
    }
  };
/* Atomic pointer that represents the top of a stack */
  atomic<node *> head;
  atomic_int _size;
  //node *head;
public:
  void push(T const& _data) {
    node * new_node = new node(_data);
    new_node->next = head.load();
    while(!head.compare_exchange_weak(new_node->next, new_node,
                                      std::memory_order_release,
                                      std::memory_order_relaxed));
    ++_size;
  }

  ~lock_free_stack()
  {
    cout << "COUNT  " << _size << endl;
  }

  T pop()
  {
    node *old_head = head.load();
    while(old_head &&
          !head.compare_exchange_weak(old_head, old_head->next));
    --_size;
    return old_head ? old_head->data : T();
  }

  T& head_data() {
    //cout << "LOCK " << head.is_lock_free() << endl;
    return head.load()->data;
  }

  size_t size() {return _size;}
};

class workers {
  //vector<string> m_folder_queue;
  lock_free_stack<string> m_folder_stack;
  mutex folder_lock;
  boost::atomic_int done;
  boost::atomic_int m_producer_count;
  boost::atomic_int m_consumer_count;
  string working_directory;
public:
  workers(string dir) :
    m_folder_stack(), done(3), m_producer_count(0),
    m_consumer_count(0), working_directory(dir) {
  }

  string getWD() { return working_directory;}
  void initialize()
  {
    producer(working_directory);
  }
  void producer(string dirname)
  {
    //folder_lock.lock();
    ++m_producer_count;
    m_folder_stack.push(dirname);
    //folder_lock.unlock();
    return;
  }

  void consumer(std::basic_string<char> *value)
  {
    //folder_lock.lock();
    string buffer = m_folder_stack.pop();
    ++m_consumer_count;
    value->assign(buffer);
    //folder_lock.unlock();
    return;
  }

  bool are_we_done()
  {
    //cout << this_thread::get_id() << " : SIZE " << m_folder_stack.size() << " Consumer " << m_consumer_count << "  Producer " << m_producer_count << endl;
    return (m_folder_stack.size() == 0) & (m_consumer_count == m_producer_count);
  }

  void print_stat()
  {
    cout << "Produced " << m_producer_count << endl;
    cout << "Consumed " << m_consumer_count << endl;
  }
};

void scanfunc(workers *obj) {
  while(!obj->are_we_done())
  {
      string next_folder;
      obj->consumer(&next_folder);
      struct dirent prevEntry;
      struct dirent *entry;
      DIR *dir = opendir(next_folder.c_str());
      if(!dir) {
        cerr << next_folder << " " << strerror(errno) <<endl;
        continue;
      }

      do {
        readdir_r(dir, &prevEntry, &entry);
        if(entry) {
          if(entry->d_name[0] == '.')
            continue;
          else
          {
            string filename = next_folder;
            filename += entry->d_name;
            struct stat statBuf;
            int ret = stat(filename.c_str(), &statBuf);
            if(ret)
            {
                cout << "Stat failure  " << filename << endl;
                std::cerr << "Error: " << strerror(errno)<< '\n';
                continue;
            }
            else{
              if(S_ISDIR(statBuf.st_mode) && !S_ISLNK(statBuf.st_mode))
              {
                filename += "/";
                obj->producer(filename);
              }
            }
          }
        }
      } while(entry != NULL); // THIS DIR IS HANDLED
      closedir(dir);
  } // ALL OF QUEUE HANDLED
}

#define THREADS 16

int main(int argc, char const *argv[]) {
  workers obj("/home/ritsz/testScan/");
  obj.initialize();
  cout << "Start" << endl;
  time_stat statistics;
  std::vector<std::thread> v(THREADS);

  for (size_t i = 0; i < THREADS; i++) {
    v.at(i) = std::thread(scanfunc, &obj);
  }

  for (size_t i = 0; i < THREADS; i++) {
    v.at(i).join();
  }
  //scanfunc(&obj);
  obj.print_stat();
  return 0;
}
