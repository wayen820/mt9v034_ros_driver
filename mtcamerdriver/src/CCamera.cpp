#include "CCamera.h"

    CCamera::CCamera():G_WIDTH(752),G_HEIGHT(480),READDATABYTES(752*480)
    {
        thread_receiver_ready=false;
        thread_event_ready=false;
        f_capture=false;
        m_iCount=0;
        m_pOutData=new byte[READDATABYTES];
        m_pInData=new byte[READDATABYTES+G_WIDTH+3];
        m_pReadBuff=new byte[READDATABYTES];
        memset(m_pInData,0,(READDATABYTES+G_WIDTH+3)*sizeof(byte));

        disqueue=new wqueue<imgframe*>();

    }
    CCamera::~CCamera()
    {
        f_capture=false;
        disqueue->reset();

        thread_event.join();
        thread_receiver.join();

        delete disqueue;
        cout<<"ccamer destruction...."<<endl;
    }

    bool CCamera::start()
    {
        thread_event_ready=false;
        if(thread_event.start(thread_proc_callback,(void*)this)==false)
                return false;
        while(thread_event_ready==false){
            msleep(10);
        }
        thread_receiver_ready=false;
        if(thread_receiver.start(thread_proc_receiver,(void*)this,160*1024)==false){
            return false;
        }
        while(thread_receiver_ready==false){
            msleep(10);
        }
        return true;

    }
    void CCamera::proc_receiver()
    {
        cout<<"proc_receiver enter...."<<endl;
        thread_receiver_ready=true;
        int r=cyusb_open();;

        int usbNum=0;
        if(r<0)
        {
            std::cout<<"error opening lib,do you install cyusb??"<<endl;
            cyusb_close();
            return;
        }
        else if(r==0)
        {
            std::cout<<"no device found!"<<endl;
            cyusb_close();
            return;
        }
        hl=cyusb_gethandle(usbNum);
        r=cyusb_kernel_driver_active(hl,0);
        if(r!=0)
        {
            std::cout<<"kernel driver active,exiting"<<endl;
            cyusb_close();
            return;
        }
        //r=cysub_claim_interace(hl,0);
        if(r!=0)
        {
            std::cout<<"error in claiming interface\n\t";
            cyusb_close();
            return;
        }
        int transferred=0;
        unsigned char data[4];
        data[0]=(G_WIDTH&0xff<<8)>>8;
        data[1]=G_WIDTH&0xff;
        data[2]=(G_HEIGHT&0xff<<8)>>8;
        data[3]=G_HEIGHT&0xff;
        cyusb_control_write(hl,0x40,0xD1,0,0,data,4,100);
        f_capture=true;
        while (f_capture) {
            //if(cv::waitKey(1)=='e')break;

            r = cyusb_bulk_transfer(hl, 0x86, m_pReadBuff, READDATABYTES, &transferred,1);
            if(transferred>=0)
            {
                Input(m_pReadBuff,transferred);
            }
            usleep(10);
        }
        if(m_pOutData!=NULL)
        {
            delete[] m_pOutData;
            //m_pOutData=NULL;
        }
        if(m_pInData!=NULL)
        {
            delete[] m_pInData;
            //m_pInData=NULL;
        }
        delete[] m_pReadBuff;

        cyusb_close();
        hl=NULL;
        f_capture=false;
        cout<<"proc_receiver exit...."<<endl;
    }

    void* CCamera::thread_proc_receiver(void* arg)
    {
       ((CCamera*)arg)->proc_receiver();
        return NULL;
    }
    void CCamera::proc_callback()
    {
        cout<<"proc_callback enter...."<<endl;
        this->thread_event_ready=true;

        imgframe* item;
        while(!disqueue->stop)
        {
            item=disqueue->remove();//will loop to get item until success
            if(item!=NULL)
                OnSensorData(*item);
            delete item;
        }
        cout<<"proc_callback exit...."<<endl;
    }

    void* CCamera::thread_proc_callback(void* arg)
    {
        ((CCamera*)arg)->proc_callback();
        return NULL;
    }
    int CCamera::Close()
    {
        if(f_capture==false)
            return -1;
        f_capture=false;

        thread_receiver.join();
        thread_event.join();
        return 0;
    }
    int CCamera::Input(byte* lpData,unsigned int dwSize)
    {
        int iBytes=0;
        iBytes=dwSize+m_iCount;//m_iCount上一次拷贝剩余数据
        if(m_pInData==NULL)
        {
            int i=10;
        }
        if(dwSize>(READDATABYTES+G_WIDTH+3))
        {
            int j=10;
        }
        //memset(m_pInData,100,(ReadDataBytes+g_width+3));
        memcpy(m_pInData+m_iCount,lpData,dwSize);
        volatile int Cam_Num=-1;
        volatile int Che_Row=-1;
        volatile int Che_Column=-1;
        int i;
        for(i=0;i<iBytes;++i)
        {
            if(m_pInData[i]==0x55)
            {
                m_bFindDbFive=true;
                continue;
            }

            if(m_bFindDbFive&&(0xaa==m_pInData[i]))
            {
                if((i+G_WIDTH+2)>=iBytes)//如果剩下的最后几个数据长度小于video_width*2+2行号个，不足以构成完整一行，拷贝到下一缓存
                {
                    m_iCount=iBytes-i;
                    memcpy(m_pInData,m_pInData+i,m_iCount);
                    return 0;
                }
                m_iRowIndex=m_pInData[i+1];		//行号高8位
                m_iRowIndex<<=8;
                m_iRowIndex+=m_pInData[i+2];	//行号低8位
                if(m_iRowIndex>G_HEIGHT+2){
                    //AfxMessageBox(L"行号出错");
                    return 0;
                    //exit(1);
                }
                memcpy(m_pOutData+m_iRowIndex*G_WIDTH,m_pInData+i+3,G_WIDTH);
                if(m_iRowIndex>=(G_HEIGHT-1))
                {
                    //memset(m_pOutData,110,sizeof(byte)*g_height*g_width);
                  //  m_pDataProcess->Input(m_pOutData,g_height*g_width);
                   // cv::Mat frame0(g_height,g_width,CV_8UC1,m_pOutData);
                    //cv::imshow(wname,frame0);
                    //m_pDataProcess->pseudoColor(frame0);
                    //cv::waitKey(1);
                    imgframe* inputframe=new imgframe(G_WIDTH,G_HEIGHT,0);
                    memcpy(inputframe->imgBuf,m_pOutData,G_WIDTH*G_HEIGHT);
                    disqueue->add(inputframe);
                    //usleep(1);

                }
                i=i+G_WIDTH+2;

            }
             m_bFindDbFive=false;//找到0x55后，无论下个数据是不是0xaa都置状态位为FALSE,然后重新找0x55
        }

        return 0;
    }
