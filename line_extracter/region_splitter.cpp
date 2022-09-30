//
// Created by leonid on 14.02.2021.
//


#include <iostream>
#include "region_splitter.h"

region_splitter::region_splitter(float bandwidth,
                                 int bins_size,
                                 int discrete_size,
                                 int order) : bandwidth(bandwidth), bins_size(bins_size), discrete_size(discrete_size),
                                              order(order) {

}

std::pair<std::vector<std::vector<float>>, std::vector<std::vector<std::vector<float>>>>
region_splitter::get_splitted_regions_with_conf_and_weighted_mean(const std::vector<std::vector<int>> &region,
                                                                  const std::vector<std::vector<float>> &new_coords,
                                                                  const cv::Mat &cls_map) const {
    /* Here we return list of splitted regions (std::vector<std::vector<int>> 2 x region_len).
     * first in pair contains confidence and region_mean for each region in the list
     * it has shape : 2 x number_of_regions
     */

    std::vector<double> y(new_coords[1].size());
    std::vector<int> idx(y.size());
    iota(idx.begin(), idx.end(), 0);

    stable_sort(idx.begin(), idx.end(),
                [&new_coords](size_t i1, size_t i2) { return new_coords[1][i1] < new_coords[1][i2]; });
    for (int i = 0; i < y.size(); ++i) {
        y[i] = static_cast<double >(new_coords[1][idx[i]]);
    }


    double min_y = y[0];
    double max_y = y[y.size() - 1];
    std::vector<float> minimums;

    if (max_y - min_y > 0.000001) {
        double y_increment = (max_y - min_y) / bins_size;

        std::vector<int> discret_count(discrete_size + 1);
        double y_increment_discret = (max_y - min_y) / discrete_size + 0.000000000001;
        double r = min_y + y_increment_discret;

        int j = 0;
        for (int i = 0; i < y.size(); ++i) {
            if (y[i] > r) {
                r += y_increment_discret;
                j++;
            }
            discret_count[j]++;
        }


        std::vector<double> hist_y;
        if (y_increment == 0) {
            exit(2);
        }

        double sum = 0;
        for (double point = min_y; point <= max_y; point += y_increment) {
            double d = 0.0;

            for (int i = 0; i < discret_count.size(); ++i) {

                double z = (point - (min_y + i * y_increment_discret + y_increment_discret / 2)) / bandwidth;

                d += (exp(-0.5 * z * z) / (sqrt(2.0 * M_PI))) * discret_count[i];
            }
            d = d / (y.size() * bandwidth);
            sum += d;
            hist_y.push_back(d);
        }
        for (int i = 0; i < hist_y.size(); ++i) {
            hist_y[i] /= sum;
        }



        //std::vector<float> minimums_i;

        double tol = 0;
        for (int i = order; i < hist_y.size() - order; ++i) {
            bool is_i_min = true;
            double i_value = hist_y[i];
            for (j = -order; j <= order; ++j) {
                if (j == 0)continue;
                if (i_value > hist_y[i + j] - tol) {
                    is_i_min = false;
                }
            }
            if (is_i_min) {
                minimums.push_back(min_y + y_increment * i + y_increment / 2);
                //minimums_i.push_back(i);
            }
        }
    }


//    int scale = 10000;
//    double y_hist_max = -1;
//    for (int i = 0; i <hist_y.size(); ++i) {
//        if(hist_y[i]>y_hist_max)y_hist_max = hist_y[i];
//    }
//    cv::Mat plot(400, hist_y.size() * 20, CV_8UC1, cv::Scalar(0));
//    for (int i = 0; i < hist_y.size(); ++i) {
//        std::cout<<int((1-hist_y[i])*scale)<<std::endl;
//        plot.at<uchar>(int((1-hist_y[i])*scale) - 9200, i * 20) = 255;
//    }
//    for (int i = 0; i < minimums_i.size(); ++i) {
//        plot.at<uchar>(3 * scale / 4, minimums_i[i] * 20) = 255;
//    }
//    namedWindow("F", cv::WINDOW_AUTOSIZE);
//    imshow("F", plot);
//    cv::waitKey(0);
//
//    exit(0);

//    if (!minimums.empty()) {
//        int x = 1;
//    }

    std::vector<std::vector<std::vector<float>>> splitted_regions(minimums.size() + 1,
                                                                  std::vector<std::vector<float>>(2));

    std::vector<std::vector<float>> y_means_and_conf(2, std::vector<float>(minimums.size() + 1));

    minimums.push_back(INFINITY);


    float rm = minimums[0];
    int j = 0;
    for (int i = 0; i < y.size(); ++i) {
        if (y[i] > rm) {
            j++;
            rm = minimums[j];
        }
        int index = idx[i];
        splitted_regions[j][0].push_back(new_coords[0][index]);
        splitted_regions[j][1].push_back(new_coords[1][index]);
        float conf = cls_map.at<float>(region[0][index], region[1][index]);
        y_means_and_conf[0][j] += (y[i] * conf);
        y_means_and_conf[1][j] += conf;
    }

    for (int i = 0; i < y_means_and_conf[0].size(); ++i) {
        y_means_and_conf[0][i] /= y_means_and_conf[1][i];
        y_means_and_conf[1][i] /= splitted_regions[i][0].size();
    }

    std::pair<std::vector<std::vector<float>>, std::vector<std::vector<std::vector<float>>>> ret(y_means_and_conf,
                                                                                                 splitted_regions);
    return ret;
}