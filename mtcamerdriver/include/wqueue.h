#ifndef __wqueue_h__
#define __wqueue_h__

#include <list>
#include <pthread.h>
#include <unistd.h>
#include "commbase.h"
#include <iostream>

using namespace std;



template <typename T> class wqueue
{
    list<T> m_queue;
    Mutex m_mutex;
    //pthread_cond_t m_condv;

public:
    wqueue()
    {
        stop=false;
    }
    ~wqueue()
    {
        clear();
    }
    void add(T item)
    {
        LockGuard _l(m_mutex);
        m_queue.push_back(item);
    }
    T remove()
    {
        while(m_queue.size()==0)
        {
            if(stop)
                return NULL;
            msleep(5);
        }
        LockGuard _l(m_mutex);
        T item=m_queue.front();
        m_queue.pop_front();
        return item;
    }
    int size()
    {
        LockGuard _l(m_mutex);
        int size=m_queue.size();
        return size;
    }
    volatile bool stop;
    int reset()
    {
        stop=true;
        return 0;
    }
    void clear()
    {
        std::cout<<"wqueue clear....delete imgframe "<<m_queue.size()<<std::endl;
        while(m_queue.size()!=0)
        {
            T item=m_queue.front();
            m_queue.pop_front();
            delete item;
        }
    }
};

#endif
