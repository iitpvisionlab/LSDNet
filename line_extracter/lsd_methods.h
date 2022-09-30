//
// Created by leonid on 16.03.2021.
//
#include <vector>
#include <string>
#include <map>
#ifndef LINE_EXTRACTER_LSD_METHODS_H
#define LINE_EXTRACTER_LSD_METHODS_H

void evaluation(std::vector<float> &threshes, int data_set_size, const std::string &dataset_name, std::map<std::string, float> &algorithm_params);
double speed_test(int img_index, const std::string &dataset_name, int N, std::map<std::string, float> &algorithm_params);
void compute_and_draw_image(int img_index, float thresh, const std::string &dataset_name, std::map<std::string, float> &algorithm_params);
void draw_all_regions_of_img(int img_index);
void predict_one_img_and_save(const std::string &cls_map_path, const std::string &angle_map_path,
                              const std::string &out_path, std::map<std::string, float> &algorithm_params, int img_original_height, int img_original_width);
std::string type2str(int type);

#endif // LINE_EXTRACTER_LSD_METHODS_H
