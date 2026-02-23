/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AutoGrayscaleTask.hpp"

using namespace image_preprocessing;

AutoGrayscaleTask::AutoGrayscaleTask(std::string const& name,
    TaskCore::TaskState initial_state)
    : AutoGrayscaleTaskBase(name, initial_state)
{
}

AutoGrayscaleTask::~AutoGrayscaleTask()
{
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See AutoGrayscaleTask.hpp for more detailed
// documentation about them.

bool AutoGrayscaleTask::configureHook()
{
    if (!AutoGrayscaleTaskBase::configureHook())
        return false;
    return true;
}
bool AutoGrayscaleTask::startHook()
{
    if (!AutoGrayscaleTaskBase::startHook())
        return false;
    return true;
}
void AutoGrayscaleTask::updateHook()
{
    AutoGrayscaleTaskBase::updateHook();
}
void AutoGrayscaleTask::errorHook()
{
    AutoGrayscaleTaskBase::errorHook();
}
void AutoGrayscaleTask::stopHook()
{
    AutoGrayscaleTaskBase::stopHook();
}
void AutoGrayscaleTask::cleanupHook()
{
    AutoGrayscaleTaskBase::cleanupHook();
}
