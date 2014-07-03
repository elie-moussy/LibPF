#ifndef HOGPEOPLEDETECTOR_H
#define HOGPEOPLEDETECTOR_H

#include "LibPF/twoDDetector/twoDDetector.h"

class HogPeopleDetector : public twoDDetector
{
public:
    HogPeopleDetector(string file, ImgProducer* imgB=NULL);

    virtual GaussianMixture* process(int offsetX=0, int offsetY=0, double scale=1);

};

#endif // HOGPEOPLEDETECTOR_H
