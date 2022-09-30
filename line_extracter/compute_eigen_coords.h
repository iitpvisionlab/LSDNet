//
// Created by leonid on 14.02.2021.
//

#ifndef LINE_EXTRACTER_COMPUTE_EIGEN_COORDS_H
#define LINE_EXTRACTER_COMPUTE_EIGEN_COORDS_H

#include "opencv2/core/core.hpp"
#include <vector>
#include <numeric>

class compute_eigen_coords {
public:
    compute_eigen_coords();

    std::vector<std::vector<float>>
    compute_transformation_matrix(const std::vector<std::vector<int>> &region, const cv::Mat &cls_map = cv::Mat());

    std::vector<std::vector<float>>
    apply_trans(const std::vector<std::vector<float>> &transformation, const std::vector<std::vector<int>> &region,
                const cv::Mat &cls_map = cv::Mat());

    std::vector<std::vector<float>>
    apply_inverse_trans(const std::vector<std::vector<float>> &transformation,
                        const std::vector<std::vector<float>> &points_new) const;
private:

    float x_mean = 0;
    float y_mean = 0;
};


#endif //LINE_EXTRACTER_COMPUTE_EIGEN_COORDS_H
