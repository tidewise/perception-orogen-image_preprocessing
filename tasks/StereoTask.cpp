/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "StereoTask.hpp"

using namespace image_preprocessing;
using namespace base::samples::frame;

StereoTask::StereoTask(std::string const& name, TaskCore::TaskState initial_state)
    : StereoTaskBase(name, initial_state)
{
//    out_frame_pair->reset(new FramePair);
}

StereoTask::~StereoTask()
{
}


bool StereoTask::startHook()
{
    if (! StereoTaskBase::startHook())
        return false;
    return true;
}

void StereoTask::updateHook()
{
    StereoTaskBase::updateHook();

    RTT::extras::ReadOnlyPointer<base::samples::frame::Frame> left_frame;
    RTT::extras::ReadOnlyPointer<base::samples::frame::Frame> right_frame;

    bool newLeft  = _frame_left.read(left_frame) == RTT::NewData;
    bool newRight = _frame_right.read(right_frame) == RTT::NewData;
//std::cerr << "newLeft = " << newLeft << " , newRight = " << newRight << std::endl;

    // check if both frames have been validated before doint anything else with the pointers!
    if((left_frame.valid() && right_frame.valid()) && (newLeft || newRight))
    {
        uint64_t left_timestamp = left_frame->time.microseconds;
        uint64_t right_timestamp = right_frame->time.microseconds;

//        std::cerr << "left frame ts: " << left_timestamp << std::endl;
//        std::cerr << "right frame ts: " << right_timestamp << std::endl;

        uint64_t diff;

        if(left_timestamp >= right_timestamp)
            diff = left_timestamp - right_timestamp;
        else
            diff = right_timestamp - left_timestamp;

//        std::cerr << "diff = " << diff << " µs" << std::endl;

        if(diff < 500)
        {
           FramePair *pout_frame_pair = out_frame_pair.write_access();
           //set bayer mode to the right bayer pattern if not specified 
           if(frame_mode == MODE_BAYER && left_frame->isBayer())
               frame_mode = left_frame->getFrameMode();
           if(frame_mode == MODE_BAYER && right_frame->isBayer())
               frame_mode = right_frame->getFrameMode();

//          std::cerr << "timestamps match, writing FramePair ... ";
            pout_frame_pair->first.init((left_frame->getWidth()-offset_x)*scale_x, (left_frame->getHeight()-offset_y)*scale_y, left_frame->getDataDepth(), frame_mode, false);
            frame_helper.setCalibrationParameter(_calibration_left.value());
            frame_helper.convert(*left_frame, pout_frame_pair->first, offset_x, offset_y, resize_algorithm, calibrate);
            pout_frame_pair->second.init((right_frame->getWidth()-offset_x)*scale_x, (right_frame->getHeight()-offset_y)*scale_y, right_frame->getDataDepth(), frame_mode, false);
            frame_helper.setCalibrationParameter(_calibration_right.value());
            frame_helper.convert(*right_frame, pout_frame_pair->second, offset_x, offset_y, resize_algorithm, calibrate);
            out_frame_pair.reset(pout_frame_pair);
            _oframe_pair.write(out_frame_pair);
        }

    }
}


/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See StereoTask.hpp for more detailed
// documentation about them.

// bool StereoTask::configureHook()
// {
//     if (! StereoTaskBase::configureHook())
//         return false;
//     return true;
// }
// void StereoTask::errorHook()
// {
//     StereoTaskBase::errorHook();
// }
// void StereoTask::stopHook()
// {
//     StereoTaskBase::stopHook();
// }
// void StereoTask::cleanupHook()
// {
//     StereoTaskBase::cleanupHook();
// }

