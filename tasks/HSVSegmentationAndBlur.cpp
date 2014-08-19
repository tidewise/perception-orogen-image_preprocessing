/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "HSVSegmentationAndBlur.hpp"
#include <opencv/cv.h>
#include <frame_helper/FrameHelper.h>



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
    base::samples::frame::Frame* phsv_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* phsv_v_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* org = new base::samples::frame::Frame();
    base::samples::frame::Frame* bin = new base::samples::frame::Frame();
    h_frame.reset(ph_frame);
    s_frame.reset(ps_frame);
    v_frame.reset(pv_frame);
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
         binary->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false); //We have no binary image so far
         
         //Todo check weher this is neede
         phsv_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_BGR,false);
         phsv_v_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
         frame_helper.convertColor(*in_frame,*phsv_frame);
         //TODO save copieing twice
         //frame_helper.convertColor(*in_frame,*pout_frame);
         IplImage hsv(frame_helper::FrameHelper::convertToCvMat(*phsv_frame));
         //IplImage org(frame_helper::FrameHelper::convertToCvMat(*pout_frame));
         IplImage *org = cvCreateImage(cvGetSize(&hsv),hsv.depth,hsv.nChannels);
         IplImage *bin = cvCreateImage(cvGetSize(&hsv), IPL_DEPTH_8U,1);
         cvCopy(&hsv,org,0);
         
         
         cvCvtColor(&hsv,&hsv,CV_RGB2HSV);
         //cvCvtColor(&org,&org,CV_RGB2HSV);
      
         IplImage *h_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         IplImage *s_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         IplImage *v_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         cvCvtPixToPlane(&hsv, h_plane, s_plane, v_plane, 0);
         
         //Calculate current v-lighting's disturbtion
         double upper_lighting=0;
         double lower_lighting=0;
         uint64_t num_pixel=0;
         for(size_t x= 0; x < v_plane->width;x++){
            assert(v_plane->height > 30);
            for(size_t y= 0; y < 30 ;y++){
                size_t pos = x+(y*v_plane->width);
                upper_lighting += v_plane->imageData[pos];
                num_pixel++;
            }
            for(size_t y= v_plane->height-30; y < v_plane->height;y++){
                size_t pos = x+(y*v_plane->width);
                lower_lighting += v_plane->imageData[pos];
            }
         }
         double correction = (double)upper_lighting-lower_lighting;
         correction /= num_pixel*v_plane->height;
         printf("Correctoin factor: %f (%lu,%lu)\n",correction,upper_lighting,lower_lighting);
         
         for(size_t x= 0; x < v_plane->width;x++){
            for(size_t y= 0; y < 30 ;y++){
                size_t pos = x+(y*v_plane->width);
                v_plane->imageData[pos]+=correction*y;
            }
         }
         
         frame_helper::FrameHelper::copyMatToFrame(v_plane,*phsv_v_frame);
         hsv_v_frame.reset(phsv_v_frame);
         _hsv_v_frame.write(hsv_v_frame);
         
         
         cvThreshold(h_plane, h_plane, _hMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(h_plane, h_plane, _hMin, 255, CV_THRESH_BINARY);
         cvThreshold(s_plane, s_plane, _sMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(s_plane, s_plane, _sMin, 255, CV_THRESH_BINARY);
         cvThreshold(v_plane, v_plane, _vMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(v_plane, v_plane, _vMin, 255, CV_THRESH_BINARY);
         
         pout_frame->time = in_frame->time;
         phsv_frame->time = in_frame->time;
         phsv_v_frame->time = in_frame->time;
         ps_frame->time = in_frame->time;
         
         frame_helper::FrameHelper::copyMatToFrame(h_plane,*ph_frame);
         ph_frame->time = in_frame->time;
         h_frame.reset(ph_frame);
         _hDebug.write(h_frame);
         
         frame_helper::FrameHelper::copyMatToFrame(s_plane,*ps_frame);
         pv_frame->time = in_frame->time;
         s_frame.reset(ps_frame);
         _sDebug.write(s_frame);
         
         frame_helper::FrameHelper::copyMatToFrame(v_plane,*pv_frame);
         v_frame.reset(pv_frame);
         _vDebug.write(v_frame);
         
         
         size_t v_pixel_count = 0;
         for(size_t i= 0; i < v_plane->imageSize;i++){
             if(v_plane->imageData[i])
             {
                v_pixel_count++;
             }
             bool isset = h_plane->imageData[i] && s_plane->imageData[i] && v_plane->imageData[i];
             bin->imageData[i] = isset?0:255.0;
             if(!isset){
                org->imageData[(i*3)] = _unsetValue;
                org->imageData[(i*3)+1] =  _unsetValue;
                org->imageData[(i*3)+2] = _unsetValue;
             }
         }
         if(_blur > 1 && _blur < org->width/2){
            cvSmooth(org,org,CV_BLUR,_blur,_blur);
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
         
         frame_helper::FrameHelper::copyMatToFrame(org,*pout_frame);
         frame_helper::FrameHelper::copyMatToFrame(bin,*binary);
         
         binary->time = in_frame->time;
         this->binary.reset(binary);
         out_frame.reset(pout_frame);
         _oframe.write(out_frame);
         _binary_result.write(this->binary);
         
         
         hsv_frame.reset(phsv_frame);
         
         cvReleaseImage(&h_plane);
         cvReleaseImage(&s_plane);
         cvReleaseImage(&v_plane);
         cvReleaseImage(&org);
         cvReleaseImage(&bin);

         
         
         
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
