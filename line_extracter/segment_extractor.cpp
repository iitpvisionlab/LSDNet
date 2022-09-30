//
// Created by leonid on 14.02.2021.
//

#include "segment_extractor.h"

segment_extractor::segment_extractor() = default;

std::vector<std::vector<float>> segment_extractor::extract(const std::vector<std::vector<float>> &new_region,
                                                           float region_mean) {
    float min = INT32_MAX;
    float max = INT32_MIN;
    for (int i = 0; i < new_region[0].size(); ++i) {
        float x = new_region[0][i];
        if (x < min) {
            min = x;
        }
        if (x > max) {
            max = x;
        }
    }

    std::vector<std::vector<float>> points_new = {
            {min, max},
            {region_mean, region_mean}
    };
    return points_new;
}