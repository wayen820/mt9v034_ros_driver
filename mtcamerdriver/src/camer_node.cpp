#include <ros/ros.h>
#include <CCamera.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>
#include <image_transport/camera_publisher.h>
#include <sensor_msgs/fill_image.h>
#include "mycommbase/GetImage.h"

#include <boost/thread.hpp>

class CameraDriverForRos:public CCamera
{
private:
    ros::NodeHandle nh_;
    ros::NodeHandle nh_priv_;
    std::string camera_name_,camera_info_url_;
    sensor_msgs::Image img_;
    image_transport::CameraPublisher camera_pub;
    boost::shared_ptr<camera_info_manager::CameraInfoManager> cinfo_;
    ros::ServiceServer srv_server_;
    boost::mutex g_image_mutex;
public:
    CameraDriverForRos():CCamera(),nh_priv_("~")
    {
        image_transport::ImageTransport itp(nh_priv_);
        camera_pub=itp.advertiseCamera("image_raw",1);
        srv_server_=nh_priv_.advertiseService<>("get_image",&CameraDriverForRos::requestCallback,this);
        nh_priv_.param("camera_name",camera_name_,std::string("mt9v034"));
        nh_priv_.param("camera_info_url",camera_info_url_,std::string(""));
        nh_priv_.param("camera_frame_id",img_.header.frame_id,std::string("camera"));
        cinfo_.reset(new camera_info_manager::CameraInfoManager(nh_priv_,camera_name_,camera_info_url_));
        if(!cinfo_->isCalibrated())
        {
            cinfo_->setCameraName(camera_name_);
            sensor_msgs::CameraInfo camera_info;
            camera_info.header.frame_id=img_.header.frame_id;
            camera_info.width=G_WIDTH;
            camera_info.height=G_HEIGHT;
            cinfo_->setCameraInfo(camera_info);
        }
    }

    ~CameraDriverForRos()
    {

    }
    bool requestCallback(mycommbase::GetImage::Request& req,mycommbase::GetImage::Response& rsp)
    {
        boost::mutex::scoped_lock lock(g_image_mutex);
        rsp.success=true;
        rsp.image=img_;
        ROS_INFO("requestCallback be called...");
        return true;
    }

    bool initialize()
    {
        if(start()==false)
            return false;
        return true;
    }

    void publish_topic(const imgframe& img)
    {
        boost::mutex::scoped_lock lock(g_image_mutex);
        sensor_msgs::fillImage(img_,"mono8",img.m_heigth,img.m_width,img.m_width,img.imgBuf);
        img_.header.stamp=ros::Time::now();
        //camera_img_raw_pub_.publish(msg_image);
        sensor_msgs::CameraInfoPtr ci(new sensor_msgs::CameraInfo(cinfo_->getCameraInfo()));
        ci->header.frame_id=img_.header.frame_id;
        ci->header.stamp=img_.header.stamp;

        camera_pub.publish(img_,*ci);
    }

private:
    void OnSensorData(const imgframe& img)
    {
        publish_topic(img);
    }
};
int main(int argc,char** argv)
{
    ros::init(argc,argv,"camera_driver");
    CameraDriverForRos camera;
    if(camera.initialize()==false)
    {
        ROS_ERROR("%s\n","Initialize() returns false,please check you devices.");
        return 0;
    }
    ros::spin();
    return 0;
}
