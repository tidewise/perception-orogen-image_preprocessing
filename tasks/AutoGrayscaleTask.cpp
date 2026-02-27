/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AutoGrayscaleTask.hpp"
#include <frame_helper/FrameHelper.h>
#include <opencv2/imgproc.hpp>

using namespace base::samples::frame;
using namespace frame_helper;
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

    m_on_trigger = _on_trigger.get();
    m_off_trigger = _off_trigger.get();
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

    if (_frame.read(m_frame) != RTT::NewData) {
        return;
    }

    if (m_frame->isGrayscale()) {
        _oframe.write(m_frame);
        updateState(States::NO_OP);
        return;
    }

    if (FrameHelper::getOpenCvType(*m_frame) != CV_8UC3) {
        throw std::runtime_error("only 3 one-byte channel images are supported");
    }

    Frame* frame = m_frame.write_access();
    cv::Mat cv_frame = FrameHelper::convertToCvMat(*frame);
    auto [brightness, gray_frame] = avgBrightness(cv_frame, frame->getFrameMode());
    States next_state = evaluate(brightness);
    if (next_state == GRAYSCALE_ON) {
        // TODO: evaluate grayscale conversions from
        // https://cadik.posvete.cz/color_to_gray_evaluation/cadik08perceptualEvaluation.pdf
        cv::cvtColor(gray_frame, cv_frame, cv::COLOR_GRAY2RGB);
        frame->setFrameMode(frame_mode_t::MODE_GRAYSCALE);
    }

    m_frame.reset(frame);
    _oframe.write(m_frame);
    updateState(next_state);
}

void AutoGrayscaleTask::updateState(States next_state)
{
    if (state() != next_state) {
        state(next_state);
    }

    if (state() == RUNNING) {
        state(GRAYSCALE_OFF);
    }
}

std::pair<std::uint8_t, cv::Mat> AutoGrayscaleTask::avgBrightness(cv::Mat const& frame,
    base::samples::frame::frame_mode_t mode)
{
    cv::Mat gray;
    switch (mode) {
        case frame_mode_t::MODE_RGB:
            cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY, 1);
            break;
        case frame_mode_t::MODE_BGR:
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY, 1);
            break;
        default:
            throw std::runtime_error(
                "frame mode " + std::to_string(mode) + " not supported");
    }

    return {cv::mean(gray)[0], gray};
}

AutoGrayscaleTask::States AutoGrayscaleTask::evaluate(std::size_t brightness) const
{
    if (brightness > m_off_trigger) {
        return GRAYSCALE_OFF;
    }

    if (brightness < m_on_trigger) {
        return GRAYSCALE_ON;
    }

    return state();
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
