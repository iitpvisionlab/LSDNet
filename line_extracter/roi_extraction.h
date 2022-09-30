//
// Created by leonid on 14.02.2021.
//

#ifndef LINE_EXTRACTER_ROI_EXTRACTION_H
#define LINE_EXTRACTER_ROI_EXTRACTION_H

#include "opencv2/core/core.hpp"
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iomanip>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/cuda.hpp>

#include "string"
class roi_extraction {
public:
    roi_extraction(const std::string&, float, float, int );
    cv:: Mat get_roi(const cv::Mat&);
private:
    std::string highlight_type;
    float global_thresh;
    float C;
    int block_size;
};


#endif //LINE_EXTRACTER_ROI_EXTRACTION_H
