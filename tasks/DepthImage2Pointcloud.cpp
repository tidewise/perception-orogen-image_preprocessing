/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "DepthImage2Pointcloud.hpp"

using namespace image_preprocessing;

DepthImage2Pointcloud::DepthImage2Pointcloud(std::string const& name)
    : DepthImage2PointcloudBase(name)
{
}

DepthImage2Pointcloud::DepthImage2Pointcloud(std::string const& name, RTT::ExecutionEngine* engine)
    : DepthImage2PointcloudBase(name, engine)
{
}

DepthImage2Pointcloud::~DepthImage2Pointcloud()
{
}

void DepthImage2Pointcloud::color_frameCallback(const base::Time &ts, const ::RTT::extras::ReadOnlyPointer< ::base::samples::frame::Frame > &color_frame_sample)
{
    color_frame = color_frame_sample;
}

void DepthImage2Pointcloud::frameCallback(const base::Time &ts, const ::RTT::extras::ReadOnlyPointer< ::base::samples::DistanceImage > &frame_sample)
{
    base::samples::Pointcloud pc;
    pc.time = frame_sample->time;
    Eigen::Vector3d v;
    if(color_frame.get()){
        assert(frame_sample->width == color_frame->getWidth() && frame_sample->height == color_frame->getHeight());
        for(int x = 0; x<frame_sample->width;x++){
            for(int y = 0; y<frame_sample->height;y++){
                    if(!frame_sample->getScenePoint(x,y,v)) continue;
                    pc.points.push_back(v);
                    
                    const uint8_t *pos = &color_frame->getImage()[(color_frame->getWidth()*3.0)*y+(x*3.0)]; 
                    pc.colors.push_back(Eigen::Vector4d((*pos)/255.0,(*(pos+1))/255.0,(*(pos+2))/255.0,1.0f));
            }
        }
    }else{
        for(int x = 0; x<frame_sample->width;x++){
            for(int y = 0; y<frame_sample->height;y++){
                    if(!frame_sample->getScenePoint(x,y,v)) continue;
                    pc.points.push_back(v);
            }
        }
    
    }
    _pointcloud.write(pc);
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See DepthImage2Pointcloud.hpp for more detailed
// documentation about them.

bool DepthImage2Pointcloud::configureHook()
{
    if (! DepthImage2PointcloudBase::configureHook())
        return false;
    return true;
}
bool DepthImage2Pointcloud::startHook()
{
    if (! DepthImage2PointcloudBase::startHook())
        return false;
    return true;
}
void DepthImage2Pointcloud::updateHook()
{
    DepthImage2PointcloudBase::updateHook();
}
void DepthImage2Pointcloud::errorHook()
{
    DepthImage2PointcloudBase::errorHook();
}
void DepthImage2Pointcloud::stopHook()
{
    DepthImage2PointcloudBase::stopHook();
}
void DepthImage2Pointcloud::cleanupHook()
{
    DepthImage2PointcloudBase::cleanupHook();
}
