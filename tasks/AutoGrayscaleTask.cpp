/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "AutoGrayscaleTask.hpp"
#include <frame_helper/FrameHelper.h>
#include <opencv2/imgproc.hpp>

using namespace base::samples::frame;
using namespace frame_helper;
using namespace image_preprocessing;

using PixelRGB8 = cv::Point3_<uint8_t>;
using PixelRGB16 = cv::Point3_<uint16_t>;

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

    m_color_pass_band = _color_pass_band.get();
    bool valid_color_pass_band = std::all_of(m_color_pass_band.begin(),
        m_color_pass_band.end(),
        [](Range3U8 const& range) { return range.valid(); });

    if (!valid_color_pass_band) {
        throw std::invalid_argument("invalid color pass band");
    }

    if (!m_color_pass_band.empty() && !m_replicate_input_mode) {
        throw std::invalid_argument("color pass band is only valid when "
                                    "replicate_input_mode is true");
    }

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
        return updateState(next_state);
    }

    if (m_method != GrayscaleMethod::OPENCV) {
        cv::Mat cv_frame = FrameHelper::convertToCvMat(*frame);
        computeGrayscale(cv_frame, cv_gray, m_method);
    }

    if (!m_replicate_input_mode) {
        m_frame_gray->received_time = m_frame_gray->time = frame->time;
        _oframe.write(m_frame_gray.release());
        return updateState(next_state);
    }

    std::unique_ptr<Frame> gray3 = augmentChannels(cv_gray, *frame);
    cv::Mat cv_gray3{FrameHelper::convertToCvMat(*gray3)};
    overrideWithColors(*frame, cv_gray3);
    _oframe.write(gray3.release());
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

void AutoGrayscaleTask::computeGrayscale(cv::Mat const& src,
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
            PixelRGB8 const& p = rgb.at<PixelRGB8>(i, j);
            PixelRGB16 pixel = p;
            gray.at<std::uint8_t>(i, j) =
                std::min<std::uint16_t>(255, pixel.x + pixel.y + pixel.z);
        }
    }
}

std::unique_ptr<Frame> AutoGrayscaleTask::augmentChannels(cv::Mat const& gray,
    Frame const& example)
{
    std::unique_ptr<Frame> augmented = std::make_unique<Frame>();
    augmented->init(example, false);
    cv::Mat cv_augmented = FrameHelper::convertToCvMat(*augmented);
    cv::cvtColor(gray, cv_augmented, cv::COLOR_GRAY2BGR);

    return augmented;
}

void AutoGrayscaleTask::overrideWithColors(Frame const& source, cv::Mat& gray) const
{
    if (m_color_pass_band.empty()) {
        return;
    }

    const cv::Mat cv_source = FrameHelper::convertToCvMat(source);
    const cv::Mat cv_hsv = toHSV(cv_source, source.frame_mode);

    cv::Mat mask(cv::Mat::zeros(cv_hsv.size(), CV_8UC1));
    cv::Mat scratchpad_mask;
    for (auto const& band : m_color_pass_band) {
        cv::inRange(cv_hsv, band.min, band.max, scratchpad_mask);
        cv::bitwise_or(mask, scratchpad_mask, mask);
    }

    std::vector<cv::Point> idx;
    cv::findNonZero(mask, idx);
    for (auto const& id : idx) {
        gray.at<cv::Vec3b>(id) =
            computeColoredPixel(cv_source, source.frame_mode, cv_hsv, id);
    }
}

cv::Mat AutoGrayscaleTask::toHSV(cv::Mat const& image, frame_mode_t mode)
{
    cv::Mat cv_hsv;
    switch (mode) {
        case frame_mode_t::MODE_RGB:
            cv::cvtColor(image, cv_hsv, cv::COLOR_RGB2HSV);
            break;
        case frame_mode_t::MODE_BGR:
            cv::cvtColor(image, cv_hsv, cv::COLOR_BGR2HSV);
            break;
        default:
            throw std::runtime_error(
                "hsv conversion " + std::to_string(mode) + " is not supported");
    }

    return cv_hsv;
}

cv::Vec3b AutoGrayscaleTask::computeColoredPixel(cv::Mat const& original,
    base::samples::frame::frame_mode_t original_mode,
    cv::Mat const& hsv,
    cv::Point const& pixel_pos) const
{
    cv::Mat pixel_hsv(1, 1, CV_8UC3, hsv.at<cv::Vec3b>(pixel_pos));
    cv::Mat pixel_rgb(1, 1, CV_8UC3);

    switch (m_method) {
        case (SUM):
            // Uses gray pixel "brightness" as hsv value. If the plain values were taken,
            // while in SUM mode the lights would might become less bright than the
            // surrounding pixels
            pixel_hsv.at<cv::Vec3b>(0, 0)[2] =
                m_frame_gray->at<uint8_t>(pixel_pos.x, pixel_pos.y);
            return fromHSV(pixel_hsv, original_mode).at<cv::Vec3b>(0, 0);
        default:
            return original.at<cv::Vec3b>(pixel_pos);
    }
}

cv::Mat AutoGrayscaleTask::fromHSV(cv::Mat const& image,
    base::samples::frame::frame_mode_t mode)
{
    cv::Mat rgb;
    switch (mode) {
        case (frame_mode_t::MODE_RGB):
            cv::cvtColor(image, rgb, cv::COLOR_HSV2RGB);
            break;
        case (frame_mode_t::MODE_BGR):
            cv::cvtColor(image, rgb, cv::COLOR_HSV2BGR);
            break;
        default:
            throw std::runtime_error(
                std::to_string(mode) + " conversion to hsv is not supported");
    }

    return rgb;
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
