//
// Created by leonid on 14.02.2021.
//

#ifndef LINE_EXTRACTER_REGION_GROWING_H
#define LINE_EXTRACTER_REGION_GROWING_H

#include "opencv2/core/core.hpp"
#include <vector>
#include "map"
#include <numeric>
#include <algorithm>
#include <thread>
#include <set>

class region_growing {
public:
    region_growing(int r, float tau, float deviation_thresh, int min_region_size);

    std::vector<std::vector<std::vector<int>>>
    get_regions(const cv::Mat &cls_map, const cv::Mat &angle_map, const cv::Mat &cls_bin);

    void
    get_regions_one_thread(const cv::Mat &cls_map, const cv::Mat &angle_map, const cv::Mat &cls_bin,
                           std::vector<std::vector<std::vector<int>>> &regions,
                           std::vector<std::vector<float>> &regions_mean_angle, std::vector<std::vector<bool>> &U,
                           int shift, int top_corner_x, int top_corner_y, int bot_corner_x, int bot_corner_y);

private:
//    static void
//    compute_merge_map(const std::map<int, int> &mapping, std::map<int, std::pair<int, int>> &merge_map_target,
//                      std::map<int, std::pair<int, int>> &merge_map_origin);
//
//    static void
//    compute_merge_map_for_second(std::map<int, int> &third_to_first, std::map<int, std::pair<int, int>> &merge_map_2,
//                                 const std::map<int, int> &first_to_second);
//
//    void find_mappings(int i, std::map<int, int> &mapping, int length, const std::vector<std::vector<int>> &line,
//                       const std::vector<std::vector<float>> &regions_first_mean_angle,
//                       const std::vector<std::vector<float>> &regions_second_mean_angle,
//                       bool first_index) const;
    void merge_connectivity_components(
            std::map<int, std::vector<std::set<int>>> &final_merge,
            const std::map<int, std::vector<std::vector<int>>> &comp_with_prev,
            const std::map<int, std::vector<std::vector<int>>> &comp_with_post,
            const std::map<int, int> &map_to_prev_prev, const std::map<int, int> &map_to_post_post,
            std::set<int> &used_componets_prev, std::set<int> &used_componets_post);

    void find_connectivity_components(std::map<int, std::vector<std::vector<int>>> &components,
                                      const std::map<int, std::vector<int>> &first_to_second_map,
                                      const std::map<int, std::vector<int>> &second_to_first_map,
                                      std::map<int, int> &first_to_Comp,
                                      std::map<int, int> &second_to_Comp);

    void
    find_graph(int i, std::map<int, std::vector<int>> &mapping, int length, const std::vector<std::vector<int>> &line,
               const std::vector<std::vector<float>> &regions_first_mean_angle,
               const std::vector<std::vector<float>> &regions_second_mean_angle,
               bool first_index) const;

    void merge_regions(std::vector<std::vector<std::vector<int>>> &regions,
                       std::vector<std::vector<std::vector<int>>> &regions_curr,
                       const std::vector<std::vector<std::vector<int>>> &regions_prev,
                       const std::vector<std::vector<std::vector<int>>> &regions_post,
                       std::vector<bool> &used_prev,
                       std::vector<bool> &used_curr,
                       std::vector<bool> &used_post,
                       const std::map<int, std::vector<std::set<int>>> &final_merge);

    static void
    fill_horiz_and_vert(std::vector<std::vector<std::vector<int>>> const &regions, std::vector<std::vector<int>> &vert,
                        std::vector<std::vector<int>> &horiz, int length_row,
                        int length_col, int x_shift, int y_shift, int horiz_idx,
                        int vert_idx);

    std::pair<std::vector<std::vector<int>>, std::pair<float, float>>
    region_grouping(int root[2], const cv::Mat &cls_map, const cv::Mat &angle_map, const cv::Mat &cls_bin,
                    std::vector<std::vector<bool>> &U, int top_corner_x, int top_corner_y, int bot_corner_x,
                    int bot_corner_y) const;

    int get_r_neighborhood(int x, int y, int **neighborhood, const cv::Mat &cls_bin,
                           std::vector<std::vector<bool>> &U, int top_corner_x, int top_corner_y, int bot_corner_x,
                           int bot_corner_y) const;

    int r;
    float tau;
    float deviation_thresh;
    int min_region_size;
};

#endif //LINE_EXTRACTER_REGION_GROWING_H