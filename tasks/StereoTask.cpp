/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "StereoTask.hpp"

using namespace image_preprocessing;

StereoTask::StereoTask(std::string const& name, TaskCore::TaskState initial_state)
    : StereoTaskBase(name, initial_state)
{
}

StereoTask::~StereoTask()
{
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
// bool StereoTask::startHook()
// {
//     if (! StereoTaskBase::startHook())
//         return false;
//     return true;
// }
// void StereoTask::updateHook()
// {
//     StereoTaskBase::updateHook();
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

