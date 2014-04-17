/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "HSVSegmentationAndBlur.hpp"
#include <opencv/cv.h>
#include <frame_helper/FrameHelper.h>



using namespace image_preprocessing;

HSVSegmentationAndBlur::HSVSegmentationAndBlur(std::string const& name, TaskCore::TaskState initial_state)
    : HSVSegmentationAndBlurBase(name, initial_state)
{
}

HSVSegmentationAndBlur::HSVSegmentationAndBlur(std::string const& name, RTT::ExecutionEngine* engine, TaskCore::TaskState initial_state)
    : HSVSegmentationAndBlurBase(name, engine, initial_state)
{
}

HSVSegmentationAndBlur::~HSVSegmentationAndBlur()
{
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
    base::samples::frame::Frame* pout_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* ph_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* ps_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* pv_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* phsv_frame = new base::samples::frame::Frame();
    base::samples::frame::Frame* org = new base::samples::frame::Frame();
    h_frame.reset(ph_frame);
    s_frame.reset(ps_frame);
    v_frame.reset(pv_frame);
    hsv_frame.reset(phsv_frame);
    out_frame.reset(org);
    
    
    return true;
}
void HSVSegmentationAndBlur::updateHook()
{
    HSVSegmentationAndBlurBase::updateHook();
    frame_helper::FrameHelper frame_helper;
    
     while(_frame.read(in_frame) == RTT::NewData)
     {
         base::samples::frame::Frame* pout_frame = out_frame.write_access();
         base::samples::frame::Frame* ph_frame = h_frame.write_access();
         base::samples::frame::Frame* ps_frame = s_frame.write_access();
         base::samples::frame::Frame* pv_frame = v_frame.write_access();
         base::samples::frame::Frame* phsv_frame = hsv_frame.write_access();
         if(!pout_frame){
             std::cerr << "Warning could not acquire write access" << std::endl;
             continue;
        }
        pout_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_RGB,false);
         ph_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
         ps_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
         pv_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_GRAYSCALE,false);
         
         //Todo check weher this is neede
         phsv_frame->init(in_frame->getWidth(), in_frame->getHeight(),in_frame->getDataDepth(),base::samples::frame::MODE_BGR,false);
         frame_helper.convertColor(*in_frame,*phsv_frame);
         //TODO save copieing twice
         //frame_helper.convertColor(*in_frame,*pout_frame);
         IplImage hsv(frame_helper::FrameHelper::convertToCvMat(*phsv_frame));
         //IplImage org(frame_helper::FrameHelper::convertToCvMat(*pout_frame));
         IplImage *org = cvCreateImage(cvGetSize(&hsv),hsv.depth,hsv.nChannels);
         cvCopy(&hsv,org,0);
         
         
         cvCvtColor(&hsv,&hsv,CV_RGB2HSV);
         //cvCvtColor(&org,&org,CV_RGB2HSV);
      
         IplImage *h_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         IplImage *s_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         IplImage *v_plane = cvCreateImage(cvGetSize(&hsv), 8, 1);
         cvCvtPixToPlane(&hsv, h_plane, s_plane, v_plane, 0);
         
         cvThreshold(h_plane, h_plane, _hMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(h_plane, h_plane, _hMin, 255, CV_THRESH_BINARY);
         cvThreshold(s_plane, s_plane, _sMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(s_plane, s_plane, _sMin, 255, CV_THRESH_BINARY);
         cvThreshold(v_plane, v_plane, _vMax, 255, CV_THRESH_TOZERO_INV);
         cvThreshold(v_plane, v_plane, _vMin, 255, CV_THRESH_BINARY);
         
         frame_helper::FrameHelper::copyMatToFrame(h_plane,*ph_frame);
         h_frame.reset(ph_frame);
         _hDebug.write(h_frame);
         
         frame_helper::FrameHelper::copyMatToFrame(s_plane,*ps_frame);
         s_frame.reset(ps_frame);
         _sDebug.write(s_frame);
         
         frame_helper::FrameHelper::copyMatToFrame(v_plane,*pv_frame);
         v_frame.reset(pv_frame);
         _vDebug.write(v_frame);
         
         
         for(size_t i= 0; i < v_plane->imageSize;i++){
             bool isset = h_plane->imageData[i] && s_plane->imageData[i] && v_plane->imageData[i];
             if(!isset){
                org->imageData[(i*3)] = _unsetValue;
                org->imageData[(i*3)+1] =  _unsetValue;
                org->imageData[(i*3)+2] = _unsetValue;
             }
         }
         if(_blur > 1 && _blur < org->width/2){
            cvSmooth(org,org,CV_BLUR,_blur,_blur);
         }
         
         frame_helper::FrameHelper::copyMatToFrame(org,*pout_frame);
         out_frame.reset(pout_frame);
         _oframe.write(out_frame);
         
         
         hsv_frame.reset(phsv_frame);
         
         cvReleaseImage(&h_plane);
         cvReleaseImage(&s_plane);
         cvReleaseImage(&v_plane);
         cvReleaseImage(&org);

         
         
         
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
