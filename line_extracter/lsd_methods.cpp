//
// Created by leonid on 16.03.2021.
//

#include "opencv2/core/core.hpp"
#include <opencv2/highgui.hpp>
#include <string>
#include <vector>

#include <fstream>
#include <ftw.h>
#include <iostream>

#include "compute_eigen_coords.h"
#include "region_growing.h"
#include "region_splitter.h"
#include "roi_extraction.h"
#include "segment_extractor.h"

#include "lsd_methods.h"

void compute_predictions_and_confidences_one_thread(
    std::vector<std::vector<int>> &predictions, std::vector<float> &confidences,
    const std::vector<std::vector<std::vector<int>>> &regions,
    const cv::Mat &cls_map, const region_splitter &rs, int first_region,
    int last_region, bool use_spliter) {
  compute_eigen_coords compute_eigen;
  for (int i = first_region; i < last_region; ++i) {
    const auto &region = regions[i];
    std::vector<std::vector<float>> A =
        compute_eigen.compute_transformation_matrix(region, cls_map);
    std::vector<std::vector<float>> new_coords =
        compute_eigen.apply_trans(A, region, cls_map);

    if (use_spliter) {
      // here we use splitter:
      std::pair<std::vector<std::vector<float>>,
                std::vector<std::vector<std::vector<float>>>>
          splitted_regions_and_conf =
              rs.get_splitted_regions_with_conf_and_weighted_mean(
                  region, new_coords, cls_map);
      std::vector<std::vector<float>> &means_and_conf =
          splitted_regions_and_conf.first;
      std::vector<std::vector<std::vector<float>>> &splitted_regions =
          splitted_regions_and_conf.second;
      for (int j = 0; j < splitted_regions.size(); ++j) {
        float region_mean = means_and_conf[0][j];
        float confidence = means_and_conf[1][j];

        std::vector<std::vector<float>> points_new =
            segment_extractor::extract(splitted_regions[j], region_mean);
        std::vector<std::vector<float>> points =
            compute_eigen.apply_inverse_trans(A, points_new);

        predictions[0].push_back(int(points[1][0]));
        predictions[1].push_back(int(points[0][0]));
        predictions[2].push_back(int(points[1][1]));
        predictions[3].push_back(int(points[0][1]));
        confidences.push_back(confidence);
      }

    } else {
      // here we do not use splitter:
      float region_mean = 0;
      float confidence = 0;
      std::vector<float> &y = new_coords[1];
      for (int j = 0; j < y.size(); ++j) {
        float conf = cls_map.at<float>(region[0][j], region[1][j]);
        region_mean += (y[j] * conf);
        confidence += conf;
      }
      region_mean = region_mean / confidence;
      confidence = confidence / y.size();
      std::vector<std::vector<float>> points_new =
          segment_extractor::extract(new_coords, region_mean);
      std::vector<std::vector<float>> points =
          compute_eigen.apply_inverse_trans(A, points_new);

      predictions[0].push_back(int(points[1][0]));
      predictions[1].push_back(int(points[0][0]));
      predictions[2].push_back(int(points[1][1]));
      predictions[3].push_back(int(points[0][1]));
      confidences.push_back(confidence);
    }
  }
}

void load_data_with_index(int img_index, cv::Mat &cls_map, cv::Mat &angle_map,
                          const std::string &dataset_path) {
  cls_map = imread(dataset_path + R"(\raw_pred\cls_map)" +
                       std::to_string(img_index) + ".tiff",
                   cv::IMREAD_ANYDEPTH);
  angle_map = imread(dataset_path + R"(\raw_pred\angle_map)" +
                         std::to_string(img_index) + ".tiff",
                     cv::IMREAD_ANYDEPTH);
}

void load_data_with_path(const std::string &cls_map_path,
                         const std::string &angle_map_path, cv::Mat &cls_map,
                         cv::Mat &angle_map) {
  cls_map = imread(cls_map_path, cv::IMREAD_ANYDEPTH);
  angle_map = imread(angle_map_path, cv::IMREAD_ANYDEPTH);
}

void compute_predictions_and_confidences(
    const cv::Mat &cls_map, const cv::Mat &angle_map,
    std::vector<std::vector<int>> &predictions, std::vector<float> &confidences,
    std::map<std::string, float> &algorithm_params) {

  roi_extraction roi("local thresh", algorithm_params["global thresh"],
                     algorithm_params["C"],
                     static_cast<int>(algorithm_params["block size"]));
  region_growing rg(1, algorithm_params["tau"],
                    algorithm_params["deviation thresh"],
                    static_cast<int>(algorithm_params["min region size"]));
  region_splitter rs(algorithm_params["bandwidth"],
                     static_cast<int>(algorithm_params["bins size"]),
                     static_cast<int>(algorithm_params["discrete size"]),
                     static_cast<int>(algorithm_params["order"]));

  cv::Mat cls_bin = roi.get_roi(cls_map);

  std::vector<std::vector<std::vector<int>>> regions =
      rg.get_regions(cls_map, angle_map, cls_bin);

  // here we can parallize
  std::vector<std::vector<int>> predictions1(4);
  std::vector<std::vector<int>> predictions2(4);
  std::vector<std::vector<int>> predictions3(4);
  std::vector<std::vector<int>> predictions4(4);

  std::vector<float> confidences1;
  std::vector<float> confidences2;
  std::vector<float> confidences3;
  std::vector<float> confidences4;

  int num_regions = regions.size();

  int chunck_len = num_regions / 4;

  bool use_spliter = (bool)algorithm_params["use spliter"];

  std::thread th1(compute_predictions_and_confidences_one_thread,
                  std::ref(predictions1), std::ref(confidences1),
                  std::ref(regions), std::ref(cls_map), std::ref(rs), 0,
                  chunck_len, use_spliter);

  std::thread th2(compute_predictions_and_confidences_one_thread,
                  std::ref(predictions2), std::ref(confidences2),
                  std::ref(regions), std::ref(cls_map), std::ref(rs),
                  chunck_len, chunck_len * 2, use_spliter);

  std::thread th3(compute_predictions_and_confidences_one_thread,
                  std::ref(predictions3), std::ref(confidences3),
                  std::ref(regions), std::ref(cls_map), std::ref(rs),
                  chunck_len * 2, chunck_len * 3, use_spliter);

  // main_thread
  compute_predictions_and_confidences_one_thread(
      std::ref(predictions4), std::ref(confidences4), std::ref(regions),
      std::ref(cls_map), std::ref(rs), chunck_len * 3, num_regions,
      use_spliter);
  th1.join();
  th2.join();
  th3.join();

  for (int i = 0; i < 4; ++i) {
    predictions[i].insert(predictions[i].end(), predictions1[i].begin(),
                          predictions1[i].end());
    predictions[i].insert(predictions[i].end(), predictions2[i].begin(),
                          predictions2[i].end());
    predictions[i].insert(predictions[i].end(), predictions3[i].begin(),
                          predictions3[i].end());
    predictions[i].insert(predictions[i].end(), predictions4[i].begin(),
                          predictions4[i].end());
  }

  confidences.insert(confidences.end(), confidences1.begin(),
                     confidences1.end());
  confidences.insert(confidences.end(), confidences2.begin(),
                     confidences2.end());
  confidences.insert(confidences.end(), confidences3.begin(),
                     confidences3.end());
  confidences.insert(confidences.end(), confidences4.begin(),
                     confidences4.end());
}

void evaluation(std::vector<float> &threshes, int data_set_size,
                const std::string &dataset_path,
                std::map<std::string, float> &algorithm_params) {
  for (int i = 0; i < data_set_size; ++i) {

    std::vector<std::vector<int>> predictions(4);
    std::vector<float> confidences;
    cv::Mat cls_map;
    cv::Mat angle_map;
    load_data_with_index(i, cls_map, angle_map, dataset_path);
    compute_predictions_and_confidences(cls_map, angle_map, predictions,
                                        confidences, algorithm_params);

    for (auto &thresh : threshes) {
      std::ofstream file;

      std::string last_cut = dataset_path.substr(0, dataset_path.rfind('\\'));
      std::string sub_foulder_name = dataset_path.substr(
          dataset_path.rfind('\\') + 1, dataset_path.size());
      std::string model_name =
          last_cut.substr(last_cut.rfind('\\') + 1, last_cut.size());
      std::string foulder_path = last_cut.substr(0, last_cut.rfind('\\'));

      std::string path1 = foulder_path + R"(\)" + model_name + R"(_cpp\)";
      std::string path2 = path1 + sub_foulder_name;
      std::string path3 = path2 + R"(\cpp_prediction\)";
      std::string path4 = path3 + std::to_string(thresh);

      mkdir(path1.c_str(), 0777);
      mkdir(path2.c_str(), 0777);
      mkdir(path3.c_str(), 0777);
      mkdir(path4.c_str(), 0777);

      // exit(0);

      file.open(path4 + R"(\)" + std::to_string(i) + ".csv",
                std::ofstream::out);
      for (int j = 0; j < predictions[0].size(); ++j) {
        if (confidences[j] > thresh) {
          file << predictions[0][j] << " " << predictions[1][j] << " "
               << predictions[2][j] << " " << predictions[3][j] << "\n";
        }
      }
      file.close();
    }
  }
}

double speed_test(int img_index, const std::string &dataset_path, int N,
                    std::map<std::string, float> &algorithm_params) {
  cv::Mat cls_map;
  cv::Mat angle_map;
  load_data_with_index(img_index, cls_map, angle_map, dataset_path);

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    std::vector<std::vector<int>> predictions(4);
    std::vector<float> confidences;

    compute_predictions_and_confidences(cls_map, angle_map, predictions,
                                        confidences, algorithm_params);
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  return double(
             std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
                 .count()) /
         N;
}

void predict_one_img_and_save(const std::string &cls_map_path,
                              const std::string &angle_map_path,
                              const std::string &out_path,
                              std::map<std::string, float> &algorithm_params,
                              int img_original_height,
                              int img_original_width) {
  std::vector<std::vector<int>> predictions(4);
  std::vector<float> confidences;

  cv::Mat cls_map;
  cv::Mat angle_map;
  load_data_with_path(cls_map_path, angle_map_path, cls_map, angle_map);
  compute_predictions_and_confidences(cls_map, angle_map, predictions,
                                      confidences, algorithm_params);

  std::ofstream file;

  std::string file_name =
      cls_map_path.substr(cls_map_path.rfind('/') + 1, cls_map_path.size());
  std::string img_name = file_name.substr(0, file_name.find("_cls_map"));
  file.open(out_path + img_name + ".csv", std::ofstream::out);

  for (int j = 0; j < predictions[0].size(); ++j) {
    file << int((float(predictions[0][j]) / cls_map.cols) * img_original_width) << " " 
         << int((float(predictions[1][j]) / cls_map.rows) * img_original_height) << " "
         << int((float(predictions[2][j]) / cls_map.cols) * img_original_width) << " " 
         << int((float(predictions[3][j]) / cls_map.rows) * img_original_height) << " "
         << confidences[j] << "\n";
  }
  file.close();
}

void compute_and_draw_image(int img_index, float thresh,
                            const std::string &dataset_path,
                            std::map<std::string, float> &algorithm_params) {
  std::vector<std::vector<int>> predictions(4);
  std::vector<float> confidences;
  cv::Mat cls_map;
  cv::Mat angle_map;
  load_data_with_index(img_index, cls_map, angle_map, dataset_path);
  compute_predictions_and_confidences(cls_map, angle_map, predictions,
                                      confidences, algorithm_params);

  cv::Mat final(cls_map.rows * 2, cls_map.cols * 2, CV_8UC1,
                cv::Scalar(0, 0, 0));
  for (int p = 0; p < predictions[0].size(); ++p) {
    if (confidences[p] > thresh) {
      line(final, cv::Point(predictions[0][p] * 2, predictions[1][p] * 2),
           cv::Point(predictions[2][p] * 2, predictions[3][p] * 2),
           cv::Scalar(255, 255, 255), 1);
    }
  }
  //    line(final, cv::Point(0, img_size - 2 ),
  //         cv::Point(img_size*2, img_size-2),
  //         cv::Scalar(255,255, 255), 1);
  //    line(final, cv::Point(0, img_size + 1 ),
  //         cv::Point(img_size*2, img_size + 1),
  //         cv::Scalar(255,255, 255), 1);
  //
  //    line(final, cv::Point(img_size -2, 0 ),
  //         cv::Point(img_size -2 , img_size*2),
  //         cv::Scalar(255,255, 255), 1);
  //
  //    line(final, cv::Point(img_size + 1, 0 ),
  //         cv::Point(img_size  + 1 , img_size*2),
  //         cv::Scalar(255,255, 255), 1);
  //    final.at<uchar>(cv::Point(img_size, img_size)) = 255;
  //    final.at<uchar>(cv::Point(img_size-1, img_size)) = 255;
  //    final.at<uchar>(cv::Point(img_size, img_size-1)) = 255;
  //    final.at<uchar>(cv::Point(img_size-1, img_size-1)) = 255;
  namedWindow("F", cv::WINDOW_AUTOSIZE);
  imshow("F", final);
  cv::waitKey(0);
}

//    image.convertTo(image_fix, CV_8UC1);

std::string type2str(int type) {
  /*
   * Usage:
   * cout<<type2str(image.type())<<'\n';
   */
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch (depth) {
  case CV_8U:
    r = "8U";
    break;
  case CV_8S:
    r = "8S";
    break;
  case CV_16U:
    r = "16U";
    break;
  case CV_16S:
    r = "16S";
    break;
  case CV_32S:
    r = "32S";
    break;
  case CV_32F:
    r = "32F";
    break;
  case CV_64F:
    r = "64F";
    break;
  default:
    r = "User";
    break;
  }

  r += "C";
  r += (chans + '0');

  return r;
}

void draw_all_regions_of_img(int img_index) {

  cv::Mat cls_map =
      imread("/home/leonid/trunk/line_segment_detection/for_cpp/cls_map" +
                 std::to_string(img_index) + ".tiff",
             cv::IMREAD_ANYDEPTH);
  cv::Mat angle_map =
      imread("/home/leonid/trunk/line_segment_detection/for_cpp/angle_map" +
                 std::to_string(img_index) + ".tiff",
             cv::IMREAD_ANYDEPTH);

  roi_extraction roi("local thresh", 0.5, 2, 11);
  region_growing rg(1, 0.05, 0.35, 25);
  region_splitter rs(1, 15, 15, 1);
  cv::Mat cls_bin = roi.get_roi(cls_map);
  std::vector<std::vector<std::vector<int>>> regions =
      rg.get_regions(cls_map, angle_map, cls_bin);
  cv::Mat canvas(256, 256, CV_8UC1, cv::Scalar(0));
  int n = regions.size();
  for (int i = 0; i < n; ++i) {
    uchar r = uchar((double(std::rand()) / RAND_MAX) * 255 + 100);
    for (int j = 0; j < regions[i][0].size(); ++j) {
      canvas.at<uchar>(regions[i][0][j], regions[i][1][j]) = 255;
    }
  }
  int img_size = canvas.rows / 2;
  line(canvas, cv::Point(0, img_size - 2),
       cv::Point(img_size * 2, img_size - 2), cv::Scalar(0), 1);
  line(canvas, cv::Point(0, img_size + 1),
       cv::Point(img_size * 2, img_size + 1), cv::Scalar(0), 1);

  line(canvas, cv::Point(img_size - 2, 0),
       cv::Point(img_size - 2, img_size * 2), cv::Scalar(0), 1);

  line(canvas, cv::Point(img_size + 1, 0),
       cv::Point(img_size + 1, img_size * 2), cv::Scalar(0), 1);
  std::string name = "F";
  namedWindow(name, cv::WINDOW_AUTOSIZE);
  imshow(name, canvas);
  cv::waitKey(0);
}
