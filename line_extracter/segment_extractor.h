//
// Created by leonid on 14.02.2021.
//

#ifndef LINE_EXTRACTER_SEGMENT_EXTRACTOR_H
#define LINE_EXTRACTER_SEGMENT_EXTRACTOR_H


#include <string>
#include <vector>
#include "algorithm"
class segment_extractor {
public:
    segment_extractor();
    std::vector<std::vector<float>>
    static extract(const std::vector<std::vector<float>> &new_region, float region_mean);
};


#endif //LINE_EXTRACTER_SEGMENT_EXTRACTOR_H
