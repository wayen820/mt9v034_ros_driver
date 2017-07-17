#ifndef __CCamera_H_
#define __CCamera_H_

#include <stdint.h>
#include "wqueue.h"
#include "imgframe.h"
#include "cyusb.h"
#include <ros/ros.h>
#include "commbase.h"


class CCamera
{
public:
    const int G_WIDTH;
    const int G_HEIGHT;
    const long READDATABYTES;

public :
    CCamera();
    ~CCamera();

    bool start();
    static void* thread_proc_receiver(void* arg);
    static void* thread_proc_callback(void* arg);
    void proc_receiver();
    void proc_callback();
    int Close();
    int Input(byte* lpData,unsigned int dwsize);
protected:
    virtual void OnSensorData(const imgframe& img){}
private:
    int m_iCount;  //data index
    int m_iRowIndex; //row index
    bool m_bFindDbFive;//data flag is find 55
    byte* m_pInData;//receive data buffer
    byte* m_pOutData;//send data buffer
    wqueue<imgframe*> *disqueue;
    imgframe* inputframe;
    bool f_capture;
    byte* m_pReadBuff;
    cyusb_handle* hl;
    Mutex mutex_camera;
    Thread thread_receiver;
    bool thread_receiver_ready,thread_event_ready;
    Thread thread_event;
};
#endif
