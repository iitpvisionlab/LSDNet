
#include "lsd_methods.h"
#include "map"
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

std::map<std::string, float> algorithm_params{{"global thresh", 0.4},
                                              {"C", 2},
                                              {"block size", 11},
                                              {"tau", 0.05},
                                              {"deviation thresh", 0.3},
                                              {"min region size", 10},
                                              {"use spliter", 1},
                                              {"bandwidth", 1},
                                              {"bins size", 15},
                                              {"discrete size", 15},
                                              {"order", 1}};

int main(int argc, char **argv)
{

  if (argc != 6)
  {
    std::cout << "invalid arguments";
    exit(1);
  }

  std::string cls_map_path = argv[1];
  std::string angle_map_path = argv[2];
  std::string out_path = argv[3];
  int img_original_height = (int)atoi(argv[4]);
  int img_original_width = (int)atoi(argv[5]);

  predict_one_img_and_save(cls_map_path, angle_map_path, out_path, algorithm_params, img_original_height, img_original_width);
  return 0;
}
