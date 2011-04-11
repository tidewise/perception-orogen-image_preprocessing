/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "MonoTask.hpp"

using namespace image_preprocessing;
using namespace base::samples::frame;

MonoTask::MonoTask(std::string const& name, TaskCore::TaskState initial_state)
    : MonoTaskBase(name, initial_state)
{
    out_frame.reset(new Frame);
}

MonoTask::~MonoTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See MonoTask.hpp for more detailed
// documentation about them.

// bool MonoTask::configureHook()
// {
//     if (! MonoTaskBase::configureHook())
//         return false;
//     return true;
// }

bool MonoTask::startHook()
{
    if (! MonoTaskBase::startHook())
        return false;
    
    //TODO
    //calc calibration matrix
    //from scale factors
    frame_helper.setCalibrationParameters(calibration);
    return true;
}

void MonoTask::updateHook()
{
    MonoTaskBase::updateHook();

    //read all new frames
    while(_frame.read(in_frame) == RTT::NewData)
    {
        Frame* pout_frame = out_frame.write_access();
        pout_frame->init((in_frame->getWidth()-offset_x)*scale_x,(in_frame->getHeight()-offset_y)*scale_y,data_depth,frame_mode,false);
        frame_helper.convert(*in_frame,*pout_frame,offset_x,offset_y,INTER_LINEAR,calibrate);
        out_frame.reset(pout_frame);
        _oframe.write(out_frame);
    }
}


// void MonoTask::errorHook()
// {
//     MonoTaskBase::errorHook();
// }
// void MonoTask::stopHook()
// {
//     MonoTaskBase::stopHook();
// }
// void MonoTask::cleanupHook()
// {
//     MonoTaskBase::cleanupHook();
// }

