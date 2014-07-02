#include "HogPeopleDetector.h"
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/gpu/gpu.hpp"



using cv::gpu::HOGDescriptor;
using cv::Size;
using cv::Point;
using cv::Scalar;

HogPeopleDetector::HogPeopleDetector(string file, ImgProducer* imgB=NULL)
    : twoDDetector(file, imgB) {
    this->gm = new GaussianMixture(NBMAXFACES,2);


}

GaussianMixture* HogPeopleDetector::process(int offsetX=0, int offsetY=0, double scale=1) {

    detectedROI.clear();
    detectedCENTER.clear();
    detectedSCALES.clear();
    detectedIDS.clear();

    //gets the detections
    vector<cv::Rect> rois;
    vector<cv::Point> points;
    HOGDescriptor hogDesc;//the detector
    hogDesc.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());//we use a people detector

    //cv::Mat m (this->imgBank->imgSRC);
    cv::gpu::GpuMat gpuMat(this->imgBank->imgSRC);

    hogDesc.detectMultiScale(gpuMat, rois, 0, Size(8, 8),
                             Size(32,32), 1.05, 2.5);

    //fills the gaussian mixture with detections
    for(int i = 0 ; i < rois.size() ; i++) {
        cv::Rect r = rois[i];
        detectedROI.push_back(r);
        cv::Point p;
        p.x = r.x + r.width*0.5;
        p.y = r.y + r.height*0.5;
        detectedCENTER.push_back(p);
        this->gm->glist[i]->mean[0] = p.x;
        this->gm->glist[i]->mean[1] = p.y;
    }
    this->nbdetected = rois.size();

    gm->curnb = rois.size();
    return gm;

}
