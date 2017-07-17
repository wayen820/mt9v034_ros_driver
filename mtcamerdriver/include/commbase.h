#ifndef __Commbase_h_
#define __Commbase_h_

#include<unistd.h>
#include<pthread.h>

typedef unsigned char byte;

class Mutex
{
    pthread_mutex_t mutex;
public:
    Mutex(){
        pthread_mutex_init(&mutex,NULL);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }
    inline void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    inline void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
};

class LockGuard
{
    Mutex& mutex;
public:
    LockGuard(Mutex& m):mutex(m){
        mutex.lock();
    }
    ~LockGuard(){
        mutex.unlock();
    }

};
class Thread
{
    pthread_t thread;
public:
    Thread():thread(0)
    {
    }
    bool start(void*(*thread_proc)(void*),void* arg,size_t stack_size=16*1024){
        pthread_attr_t attr;
        size_t stacksize;
        pthread_attr_init(&attr);

        pthread_attr_setstacksize(&attr,stack_size);
        int res=pthread_create(&thread,&attr,thread_proc,(void*)arg);
        if(res!=0)
        {
            thread=0;
        }
        return res==0;
    }
    void join()
    {
        if(thread!=0){
            pthread_join(thread,NULL);
        }
        thread=0;
    }
};
static void msleep(unsigned int msec)
{
    usleep(msec*1000);
}
#endif
