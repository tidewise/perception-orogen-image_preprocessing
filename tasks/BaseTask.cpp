/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "BaseTask.hpp"

using namespace image_preprocessing;

BaseTask::BaseTask(std::string const& name, TaskCore::TaskState initial_state)
    : BaseTaskBase(name, initial_state)
{
}

BaseTask::~BaseTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See BaseTask.hpp for more detailed
// documentation about them.

// bool BaseTask::configureHook()
// {
//     if (! BaseTaskBase::configureHook())
//         return false;
//     return true;
// }
//
bool BaseTask::startHook()
{
    if (! BaseTaskBase::startHook())
        return false;

    scale_x = _scale_x.value();
    scale_y = _scale_y.value();
    frame_mode = _frame_mode.value();
    calibrate = _calibrate.value();
    offset_x = _offset_x.value();
    offset_y = _offset_y.value();
    data_depth = _data_depth.value();

    return true;
}

// void BaseTask::updateHook()
// {
//     BaseTaskBase::updateHook();
// }
// void BaseTask::errorHook()
// {
//     BaseTaskBase::errorHook();
// }
// void BaseTask::stopHook()
// {
//     BaseTaskBase::stopHook();
// }
// void BaseTask::cleanupHook()
// {
//     BaseTaskBase::cleanupHook();
// }

