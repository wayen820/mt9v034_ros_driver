
#ifndef __imgframe_h__
#define __imgframe_h__

#include "commbase.h"
#include <string.h>

class imgframe
{
public:
    imgframe(int width,int height,int camNum):m_width(width),m_heigth(height),m_camNum(camNum)
    {
        imgBuf=new byte[height*width];
    }
    ~imgframe()
    {
        if(imgBuf!=NULL)
            delete imgBuf;
    }
    imgframe(const imgframe& frame)
    {
        m_width=frame.m_width;
        m_heigth=frame.m_heigth;
        imgBuf=new byte[frame.m_heigth*frame.m_heigth];
        memcpy(imgBuf,frame.imgBuf,m_width*m_heigth);
    }
    int m_width;
    int m_heigth;
    int m_camNum;
    long timestamp;
    byte* imgBuf;
};

#endif
