#ifndef _MAIN_THREAD_HPP_
#define _MAIN_THREAD_HPP_

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
#define create_thread(func, ...) {\
    main_thread.thread_list.push_back(std::thread(func, ##__VA_ARGS__));\
}
*/

#define create_thread(func, ...) {\
    std::thread th(func, ##__VA_ARGS__);\
    th.detach();\
}

class Main_Thread {
public:
    std::list<std::thread> thread_list;
    std::mutex mutex_main, mutex_child;
    std::condition_variable cond_var;
    Main_Thread();
    void start();
    void notify();
};

#endif // _MAIN_THREAD_HPP_
