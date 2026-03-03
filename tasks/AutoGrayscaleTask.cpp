/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AutoGrayscaleTask.hpp"
#include <frame_helper/FrameHelper.h>
#include <opencv2/imgproc.hpp>

using namespace base::samples::frame;
using namespace frame_helper;
using namespace image_preprocessing;

using PixelRGB = cv::Point3_<uint8_t>;

static void sumGrayscale(cv::Mat const& rgb, cv::Mat& gray);

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

    m_frame_gray = std::make_unique<Frame>();

    m_replicate_input_mode = _replicate_input_mode.get();
    m_method = _grayscale_method.get();

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

    RTT::extras::ReadOnlyPointer<base::samples::frame::Frame> frame;
    if (_frame.read(frame, false) != RTT::NewData) {
        return;
    }

    if (frame->isGrayscale()) {
        _oframe.write(frame);
        updateState(States::NO_OP);
        return;
    }

    if (FrameHelper::getOpenCvType(*frame) != CV_8UC3) {
        throw std::runtime_error("only 3-one-byte channel images are supported");
    }

    cv::Mat cv_gray = getGrayFrame(*frame);
    std::uint8_t brightness = cv::mean(cv_gray)[0];

    States next_state = evaluate(brightness);

    if (next_state != GRAYSCALE_ON) {
        _oframe.write(frame);
        updateState(next_state);
        return;
    }

    if (m_method != GrayscaleMethod::OPENCV) {
        cv::Mat cv_frame = FrameHelper::convertToCvMat(*frame);
        convertToGrayscale(cv_frame, cv_gray, m_method);
    }
    writeOFrame(next_state, cv_gray, *frame);
    updateState(next_state);
}

cv::Mat AutoGrayscaleTask::getGrayFrame(Frame const& input_frame)
{
    if (!m_frame_gray) {
        m_frame_gray = std::make_unique<Frame>();
    }

    auto size = input_frame.getSize();
    if (!m_frame_gray->isGrayscale() || m_frame_gray->getSize() != size) {
        m_frame_gray->setFrameMode(frame_mode_t::MODE_GRAYSCALE);
        m_frame_gray->init(size.width, size.height, 8, frame_mode_t::MODE_GRAYSCALE, -1);
    }
    cv::Mat gray = FrameHelper::convertToCvMat(*m_frame_gray);
    cv::Mat input_cv = FrameHelper::convertToCvMat(input_frame);

    auto mode = input_frame.getFrameMode();
    switch (mode) {
        case frame_mode_t::MODE_RGB:
            cv::cvtColor(input_cv, gray, cv::COLOR_RGB2GRAY, 1);
            break;
        case frame_mode_t::MODE_BGR:
            cv::cvtColor(input_cv, gray, cv::COLOR_BGR2GRAY, 1);
            break;
        default:
            throw std::runtime_error(
                "frame mode " + std::to_string(mode) + " not supported");
    }

    return gray;
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

void AutoGrayscaleTask::convertToGrayscale(cv::Mat const& src,
    cv::Mat& dst,
    GrayscaleMethod method)
{
    if (src.size() != dst.size()) {
        throw std::runtime_error("src and dst images have mismatching sizes");
    }

    // TODO: evaluate grayscale conversions from
    // https://cadik.posvete.cz/color_to_gray_evaluation/cadik08perceptualEvaluation.pdf
    switch (method) {
        case GrayscaleMethod::SUM:
            sumGrayscale(src, dst);
            break;
        default:
            throw std::runtime_error(
                "grayscale conversion " + std::to_string(method) + " is not supported");
    }
}

void sumGrayscale(cv::Mat const& rgb, cv::Mat& gray)
{
    for (int i = 0; i < rgb.rows; i++) {
        for (int j = 0; j < rgb.cols; j++) {
            PixelRGB const& pixel = rgb.at<PixelRGB>(i, j);
            gray.at<std::uint8_t>(i, j) =
                std::min<std::uint16_t>(255, pixel.x + pixel.y + pixel.z);
        }
    }
}

void AutoGrayscaleTask::writeOFrame(States state,
    cv::Mat const& cv_gray,
    Frame const& input_frame)
{
    if (!m_replicate_input_mode) {
        m_frame_gray->time = input_frame.time;
        m_frame_gray->received_time = input_frame.time;
        _oframe.write(m_frame_gray.release());
        return;
    }

    std::unique_ptr<Frame> out_frame = std::make_unique<Frame>();
    out_frame->init(input_frame, false);
    cv::Mat cv_out = FrameHelper::convertToCvMat(*out_frame);

    cv::cvtColor(cv_gray, cv_out, cv::COLOR_GRAY2BGR);
    _oframe.write(out_frame.release());
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
