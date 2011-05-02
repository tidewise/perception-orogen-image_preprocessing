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
    
    frame_helper.setCalibrationParameter(_calibration.value());
    return true;
}

void MonoTask::updateHook()
{
    MonoTaskBase::updateHook();

    //read all new frames
    while(_frame.read(in_frame) == RTT::NewData)
    {
        //set bayer mode to the right bayer pattern if not specified 
        if(frame_mode == MODE_BAYER && in_frame->isBayer())
            frame_mode = in_frame->getFrameMode();

        Frame* pout_frame = out_frame.write_access();
        pout_frame->init((in_frame->getWidth()-offset_x)*scale_x,
                (in_frame->getHeight()-offset_y)*scale_y,
                in_frame->getDataDepth(),frame_mode,false);
        frame_helper.convert(*in_frame,*pout_frame,offset_x,offset_y,resize_algorithm,calibrate);
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

