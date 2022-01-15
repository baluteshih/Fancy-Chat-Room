#include "main_thread.hpp"
#include "helper.hpp"

Main_Thread::Main_Thread() {}

void Main_Thread::start() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mutex_main);
            cond_var.wait(lock);
        }
        /*for (auto it = thread_list.begin(); it != thread_list.end();) {
            if (it->joinable()) {
                _helper_msg("joined thread");
                it->join();
                it = thread_list.erase(it);
            }
            else {
                ++it;
            }
        }*/
    }
}

void Main_Thread::notify() {
    /*std::lock_guard<std::mutex> lock(mutex_child);
    cond_var.notify_one();*/
}
