//
// Created by leonid on 14.02.2021.
//

#include "roi_extraction.h"

roi_extraction::roi_extraction(const std::string &highlight_type1, float global_thresh, float C, int block_size) :
        global_thresh(global_thresh),
        C(C),
        block_size(block_size) {
    std::vector<std::string> highlight_types{"thresh", "local thresh"};
    if (std::find(highlight_types.begin(), highlight_types.end(), highlight_type1) != highlight_types.end()) {
        highlight_type = highlight_type1;
    } else {
        fprintf(stderr, "Incorrect input");
    }

}

cv::Mat roi_extraction::get_roi(const cv::Mat &cls_map) {
    if (highlight_type == "thresh") {
        return cls_map > global_thresh;
    }
    if (highlight_type == "local thresh") {
        cv::Mat src;
        cv::Mat dst;
        cls_map.convertTo(src, CV_8UC1, 255);

        cv::adaptiveThreshold(
                src,
                dst,
                255,
                cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                cv::THRESH_BINARY,
                block_size,
                C
        );

        return (cls_map > global_thresh).mul(dst > 0);
    }

}