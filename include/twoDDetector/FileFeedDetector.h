#ifndef FILEFEEDDETECTOR_H
#define FILEFEEDDETECTOR_H

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include "twoDDetector/twoDDetector.h"
#include "../HOGPeopleDetectionExtractor/HOGPeopleDetectionExtractor/Detection.h"

using std::string;
using std::vector;

/**
 * @brief The FileFeedDetector class
 * This class specializes twoDDetector class. It basically does no detection at all
 * but takes it from a formatted file.
 * The class keeps track of the current position in the file in the curDetec variable.
 */
class FileFeedDetector : public twoDDetector
{
public:
    FileFeedDetector(string file,ImgProducer* imgB, string filePath = NULL);
    GaussianMixture* process(int offsetX=0, int offsetY=0, double scale=1);
    inline void setCurrentDetecIterator(int pos) {this->curDetec = pos;}


protected:
    vector<Detection> detecs;
    int curDetec = 0 ;
    string filePath;

    int getInputFromFile();
};

#endif // FILEFEEDDETECTOR_H
