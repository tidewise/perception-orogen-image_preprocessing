/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "HSVSegmentationAndBlur.hpp"

#include <opencv2/imgproc.hpp>

using namespace image_preprocessing;

HSVSegmentationAndBlur::HSVSegmentationAndBlur(std::string const& name, TaskCore::TaskState initial_state)
    : HSVSegmentationAndBlurBase(name, initial_state)
{
    init();
}

HSVSegmentationAndBlur::HSVSegmentationAndBlur(std::string const& name, RTT::ExecutionEngine* engine, TaskCore::TaskState initial_state)
    : HSVSegmentationAndBlurBase(name, engine, initial_state)
{
    init();
}

HSVSegmentationAndBlur::~HSVSegmentationAndBlur()
{
}


void HSVSegmentationAndBlur::init(){
    base::samples::frame::Frame* pout_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* ph_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* ps_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* pv_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* phd_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* psd_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* pvd_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* phsv_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* phsv_v_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* org = new base::samples::frame::Frame();
    base::samples::frame::Frame* bin = new base::samples::frame::Frame();
    h_frame.reset(ph_frame);
    s_frame.reset(ps_frame);
    v_frame.reset(pv_frame);
    h_frame_debug.reset(phd_frame);
    s_frame_debug.reset(psd_frame);
    v_frame_debug.reset(pvd_frame);
    hsv_frame.reset(phsv_frame);
    hsv_v_frame.reset(phsv_v_frame);
    out_frame.reset(org);
    binary.reset(bin);
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See HSVSegmentationAndBlur.hpp for more detailed
// documentation about them.

bool HSVSegmentationAndBlur::configureHook()
{
    if (! HSVSegmentationAndBlurBase::configureHook())
        return false;
    return true;
}
bool HSVSegmentationAndBlur::startHook()
{
    if (! HSVSegmentationAndBlurBase::startHook())
        return false;
    
    
    return true;
}
void HSVSegmentationAndBlur::updateHook()
{
    HSVSegmentationAndBlurBase::updateHook();
    frame_helper::FrameHelper frame_helper;
    
     if(_frame.read(in_frame,true) == RTT::NewData)
     {
         
        base::samples::frame::Frame* pout_frame = out_frame.write_access();
        base::samples::frame::Frame* ph_frame = h_frame.write_access();
        base::samples::frame::Frame* ps_frame = s_frame.write_access();
        base::samples::frame::Frame* pv_frame = v_frame.write_access();
        base::samples::frame::Frame* phd_frame = h_frame_debug.write_access();
        base::samples::frame::Frame* psd_frame = s_frame_debug.write_access();
        base::samples::frame::Frame* pvd_frame = v_frame_debug.write_access();
        base::samples::frame::Frame* phsv_frame = hsv_frame.write_access();
        base::samples::frame::Frame* phsv_v_frame = hsv_v_frame.write_access();
        base::samples::frame::Frame* binary = this->binary.write_access();
        
        if(!pout_frame){
            std::cerr << "Warning could not acquire write access" << std::endl;
            return;
        }
        pout_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_RGB,false);
        ph_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        ps_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        pv_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        phd_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        psd_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        pvd_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        binary->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false); //We have no binary image so far
        
        //Todo check weher this is neede
        phsv_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_BGR,false);
        phsv_v_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
        frame_helper.convertColor(*in_frame,*phsv_frame);
        //TODO save copieing twice
        //frame_helper.convertColor(*in_frame,*pout_frame);
        cv::Mat hsv(frame_helper::FrameHelper::convertToCvMat(*phsv_frame));
        //IplImage org(frame_helper::FrameHelper::convertToCvMat(*pout_frame));
        cv::Mat org(hsv.size() ,hsv.depth() ,hsv.channels());
        cv::Mat bin(hsv.size(), CV_8U, 1);

        hsv.copyTo(org);

        cvtColor(hsv, hsv, cv::COLOR_RGB2HSV);

        std::vector<cv::Mat> hsv_channels;
        split(hsv, hsv_channels);

        cv::Mat *h_plane = &(hsv_channels.at(0));
        cv::Mat *s_plane = &(hsv_channels.at(1));
        cv::Mat *v_plane = &(hsv_channels.at(2));
         
        int blur = _blur;
        if(!(blur % 2)){
            blur += 1;
        }
        if(blur > 1 && _blur < (org.size().width / 2)) {
            for (uint i = 0; i < hsv_channels.size(); ++i) {
                cv::medianBlur(hsv_channels.at(i), hsv_channels.at(i), blur);
            }
        }
        
        //Calculate current v-lighting's disturbtion
        double upper_lighting=0;
        double lower_lighting=0;
        uint64_t num_pixel=0;
        for(size_t x= 0; x < v_plane->size().width;x++){
            assert(v_plane->height > 30);
            for(size_t y= 0; y < 30 ;y++){
                size_t pos = x+(y*v_plane->size().width);
                upper_lighting += v_plane->data[pos];
                num_pixel++;
            }
            for(size_t y= v_plane->size().height-30; y < v_plane->size().height;y++){
                size_t pos = x+(y*v_plane->size().width);
                lower_lighting += v_plane->data[pos];
            }
        }
        double correction = (double)upper_lighting-lower_lighting;
        correction /= num_pixel*v_plane->size().height;
        printf("Correctoin factor: %f (%lu,%lu)\n",correction,upper_lighting,lower_lighting);
        
        for(size_t x= 0; x < v_plane->size().width;x++) {
            for(size_t y= 0; y < 30 ;y++){
                size_t pos = x+(y*v_plane->size().width);
                double v = v_plane->data[pos] + correction*y;
                if(v<0)v=0;
                if(v>2550)v=255;
                v_plane->data[pos]=v;
                
            }
        }

        frame_helper::FrameHelper::copyMatToFrame(*v_plane,*phsv_v_frame);
        hsv_v_frame.reset(phsv_v_frame);
        _hsv_v_frame.write(hsv_v_frame);

        // ----- DEBUG OUTPUT -----

        frame_helper::FrameHelper::copyMatToFrame(*h_plane,*phd_frame);
        phd_frame->time = in_frame->time;
        h_frame_debug.reset(phd_frame);
        _hDebugGray.write(h_frame_debug);
        
        frame_helper::FrameHelper::copyMatToFrame(*s_plane,*psd_frame);
        psd_frame->time = in_frame->time;
        s_frame_debug.reset(psd_frame);
        _sDebugGray.write(s_frame_debug);
        
        frame_helper::FrameHelper::copyMatToFrame(*v_plane,*pvd_frame);
        pvd_frame->time = in_frame->time;
        v_frame_debug.reset(pvd_frame);
        _vDebugGray.write(v_frame_debug);     

        // --------------------------

        threshold(*h_plane, _hMin, _hMax);
        threshold(*s_plane, _sMin, _sMax);
        threshold(*v_plane, _vMin, _vMax);

        pout_frame->time = in_frame->time;
        phsv_frame->time = in_frame->time;
        phsv_v_frame->time = in_frame->time;
        ps_frame->time = in_frame->time;
        
        frame_helper::FrameHelper::copyMatToFrame(*h_plane,*ph_frame);
        ph_frame->time = in_frame->time;
        h_frame.reset(ph_frame);
        _hDebug.write(h_frame);
        
        frame_helper::FrameHelper::copyMatToFrame(*s_plane,*ps_frame);
        pv_frame->time = in_frame->time;
        s_frame.reset(ps_frame);
        _sDebug.write(s_frame);
        
        frame_helper::FrameHelper::copyMatToFrame(*v_plane,*pv_frame);
        v_frame.reset(pv_frame);
        _vDebug.write(v_frame);
         
         
        size_t v_pixel_count = 0;
        for(size_t i= 0; i < v_plane->size(); i++) {
            if(v_plane->data[i]) {
                v_pixel_count++;
            }
            bool isset = h_plane->data[i] && s_plane->data[i] && v_plane->data[i];
            bool invert = _invert.get();
            bin.data[i] = isset?(invert?0:255):(invert?255:0);
            if(!isset){
            org.data[(i*3)] = _unsetValue;
            org.data[(i*3)+1] =  _unsetValue;
            org.data[(i*3)+2] = _unsetValue;
            }
        }
        if(_target_pixel_s.get() > 0)
        {
            if(v_pixel_count < _target_pixel_s){
                _vMax.set(_vMax.get()+_steps_per_frame.get());
            }
            if(v_pixel_count > _target_pixel_s){
                _vMax.set(_vMax.get()-_steps_per_frame.get());
            }
            if(_maxVadapt.get() < _vMax.get()){
                _vMax.set(_maxVadapt.get());
            }
            if(_minVadapt.get() > _vMax.get()){
                _vMax.set(_minVadapt.get());
            }
        }
         
        frame_helper::FrameHelper::copyMatToFrame(org, *pout_frame);
        frame_helper::FrameHelper::copyMatToFrame(bin, *binary);
        
        binary->time = in_frame->time;
        this->binary.reset(binary);
        out_frame.reset(pout_frame);
        _oframe.write(out_frame);
        _binary_result.write(this->binary);
        
        
        hsv_frame.reset(phsv_frame);
    }
}
void HSVSegmentationAndBlur::errorHook()
{
    HSVSegmentationAndBlurBase::errorHook();
}
void HSVSegmentationAndBlur::stopHook()
{
    HSVSegmentationAndBlurBase::stopHook();
}
void HSVSegmentationAndBlur::cleanupHook()
{
    HSVSegmentationAndBlurBase::cleanupHook();
}

void HSVSegmentationAndBlur::threshold(cv::Mat &img, int min, int max) 
{
    cv::threshold(img, img, max, 255, cv::THRESH_TOZERO_INV);
    cv::threshold(img, img, min, 255, cv::THRESH_BINARY);    
}
