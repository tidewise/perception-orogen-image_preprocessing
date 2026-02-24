/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AutoGrayscaleTask.hpp"
#include <frame_helper/FrameHelper.h>
#include <opencv2/imgproc.hpp>

using namespace base::samples::frame;
using namespace frame_helper;
using namespace image_preprocessing;

// value channel
constexpr std::size_t V = 2;
constexpr uint8_t DEPTH = 3;

using MatrixXu8 =
    Eigen::Matrix<std::uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Stride = Eigen::Stride<Eigen::Dynamic, 3>;
using MapU8 = Eigen::Map<MatrixXu8, Eigen::Unaligned, Stride>;

// Maps a cv::Mat channel. It only works for CV_8UC3 types!
MapU8 map(cv::Mat const& frame, std::size_t channel)
{
    return MapU8(frame.data + channel,
        frame.rows,
        frame.cols,
        Stride(frame.cols * DEPTH, DEPTH));
}

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
    auto [brightness, hsv_frame] = avgBrightness(cv_frame, frame->getFrameMode());
    States next_state = evaluate(brightness);
    if (next_state == GRAYSCALE_ON) {
        // TODO: evaluate grayscale conversions from
        // https://cadik.posvete.cz/color_to_gray_evaluation/cadik08perceptualEvaluation.pdf
        grayscaleFromHSV(hsv_frame, cv_frame);
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
    cv::Mat frame_hsv;
    switch (mode) {
        case frame_mode_t::MODE_RGB:
            cv::cvtColor(frame, frame_hsv, cv::COLOR_RGB2HSV);
            break;
        case frame_mode_t::MODE_BGR:
            cv::cvtColor(frame, frame_hsv, cv::COLOR_BGR2HSV);
            break;
        default:
            throw std::runtime_error(
                "frame mode " + std::to_string(mode) + " not supported");
    }

    MapU8 value = map(frame_hsv, V);
    double avg = value.cast<double>().mean();

    return {static_cast<std::uint8_t>(avg), frame_hsv};
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

void AutoGrayscaleTask::grayscaleFromHSV(cv::Mat const& hsv, cv::Mat& dst)
{
    map(dst, 0) = map(hsv, V);
    map(dst, 1) = map(hsv, V);
    map(dst, 2) = map(hsv, V);
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
