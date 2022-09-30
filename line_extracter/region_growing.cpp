//
// Created by leonid on 14.02.2021.
//

#include "region_growing.h"
#include <cmath>
#include <iostream>
#include "queue"
#include <set>

#define SAW 1

region_growing::region_growing(int r, float tau, float deviation_thresh, int min_region_size) :
        r(r),
        tau(tau),
        deviation_thresh(deviation_thresh),
        min_region_size(min_region_size) {
}

void region_growing::fill_horiz_and_vert(std::vector<std::vector<std::vector<int>>> const &regions,
                                         std::vector<std::vector<int>> &vert,
                                         std::vector<std::vector<int>> &horiz,
                                         int length_row,
                                         int length_col,
                                         int x_shift,
                                         int y_shift,
                                         int horiz_idx, int vert_idx) {
    for (int i = 0; i < regions.size(); ++i) {
        int n = regions[i][0].size();
        for (int j = 0; j < n; ++j) {
            int x = regions[i][0][j];
            int y = regions[i][1][j];
            if (x == length_row - x_shift) {
                horiz[horiz_idx][y] = i + 1;
            }
            if (y == length_col - y_shift) {
                vert[vert_idx][x] = i + 1;
            }
        }
    }
}

void region_growing::find_graph(int i, std::map<int, std::vector<int>> &mapping, int length,
                                const std::vector<std::vector<int>> &line,
                                const std::vector<std::vector<float>> &regions_first_mean_angle,
                                const std::vector<std::vector<float>> &regions_second_mean_angle,
                                bool first_index) const {
    int min_len = i;
    int j;
    int k;
    int l;
    while (i < length) {
        while (i != length and !line[first_index][i]) {
            i++;
        }
        if (i == length)break;
        int i_index = line[first_index][i];
        j = i + 1;

        while (j != length and line[first_index][j] == i_index) {
            j++;
        }
        k = i - 1;
        if (k < min_len)k++;

        while (k != length and !line[!first_index][k]) {
            k++;

        }

        if (k == length)break;
        int k_index = line[!first_index][k];

        l = k + 1;
        while (l != length and line[!first_index][l] == k_index) {
            l++;
        }
        while (j - k >= 0) {
            if (l - i >= 0) {
                int first_region_index = line[first_index][i] - 1;
                int second_region_index = line[!first_index][k] - 1;
                float V_mean_x_first = regions_first_mean_angle[0][first_region_index];
                float V_mean_y_first = regions_first_mean_angle[1][first_region_index];
                float V_mean_x_second = regions_second_mean_angle[0][second_region_index];
                float V_mean_y_second = regions_second_mean_angle[1][second_region_index];

                if ((V_mean_x_first - V_mean_x_second) * (V_mean_x_first - V_mean_x_second) +
                    (V_mean_y_first - V_mean_y_second) * (V_mean_y_first - V_mean_y_second)
                    < tau) {
                    mapping[first_region_index].push_back(second_region_index);
                }
            }
            k = l;
            while (k != length and !line[!first_index][k] and j - k >= 0) {
                k++;
            }
            if (j - k < 0)break;
            if (k == length)break;
            k_index = line[!first_index][k];
            l = k + 1;
            while (l != length and line[!first_index][l] == k_index) {
                l++;
            }
        }
        i = j;
    }

}

void region_growing::find_connectivity_components(std::map<int, std::vector<std::vector<int>>> &components,
                                                  const std::map<int, std::vector<int>> &first_to_second_map,
                                                  const std::map<int, std::vector<int>> &second_to_first_map,
                                                  std::map<int, int> &first_to_Comp,
                                                  std::map<int, int> &second_to_Comp) {
    std::set<int> has_been_visited_first;
    std::set<int> has_been_visited_second;
    int i = 0; // индекс очередной компоненты
    for (auto const &x:first_to_second_map) {
        if (has_been_visited_first.find(x.first) != has_been_visited_first.end())
            continue;
        // для ускорения процесса проверим сразу случай компоненты связности из двух вершин
        if (x.second.size() == 1 and second_to_first_map.at(x.second[0]).size() == 1) {
            std::vector<int> first = {x.first};
            std::vector<int> second = {x.second[0]};
            components[i].push_back(first);
            components[i].push_back(second);
            first_to_Comp[x.first] = i;
            second_to_Comp[x.second[0]] = i;
            ++i;
            continue;
        }
        std::queue<std::pair<bool, int>> bfs_queue;

        std::vector<int> first;
        std::vector<int> second;
        components[i].push_back(first);
        components[i].push_back(second);

        bfs_queue.push(std::pair<bool, int>(0, x.first));
        while (!bfs_queue.empty()) {
            bool part = bfs_queue.front().first;
            int region = bfs_queue.front().second;
            bfs_queue.pop();

            if (part == 0) {
                components[i][0].push_back(region);
                first_to_Comp[region] = i;
                has_been_visited_first.insert(region);
                for (int j : first_to_second_map.at(region)) {
                    if (has_been_visited_second.find(j) == has_been_visited_second.end())
                        bfs_queue.push(std::pair<bool, int>(1, j));
                }
            } else {
                components[i][1].push_back(region);
                second_to_Comp[region] = i;
                has_been_visited_second.insert(region);
                for (int j : second_to_first_map.at(region)) {
                    if (has_been_visited_first.find(j) == has_been_visited_first.end())
                        bfs_queue.push(std::pair<bool, int>(0, j));
                }
            }
        }
        ++i;

    }
}

void region_growing::merge_connectivity_components(
        std::map<int, std::vector<std::set<int>>> &final_merge,
        const std::map<int, std::vector<std::vector<int>>> &comp_with_prev,
        const std::map<int, std::vector<std::vector<int>>> &comp_with_post,
        const std::map<int, int> &map_to_prev_prev, const std::map<int, int> &map_to_post_post,
        std::set<int> &used_componets_prev, std::set<int> &used_componets_post) {
    // проверим на пересечение все пары компонент

    int merged_regions = 0;
    std::set<int> prev_used_in_intersection;
    for (auto &post:comp_with_post) {
        if (used_componets_post.find(post.first) != used_componets_post.end())continue;
        std::vector<int> intersected_components;
        for (auto &prev:comp_with_prev) {
            if (used_componets_prev.find(prev.first) != used_componets_prev.end())continue;
            //проверим пересечение множеств
            bool intersect = false;
            for (int i = 0; i < post.second[0].size(); ++i) {
                for (int j = 0; j < prev.second[1].size(); ++j) {
                    if (post.second[0][i] == prev.second[1][j])intersect = true;
                }
            }

            if (intersect) {
                prev_used_in_intersection.insert(prev.first);
                intersected_components.push_back(prev.first);

                used_componets_prev.insert(prev.first);
                used_componets_post.insert(post.first);
            }
        }
        if (!intersected_components.empty()) {
            //если с кем-то пересеклись

            std::set<int> current_regions(post.second[0].begin(), post.second[0].end());
            std::set<int> prev_regions;
            for (int i = 0; i < intersected_components.size(); ++i) {
                int comp_index = intersected_components[i];

                for (int j = 0; j < comp_with_prev.at(comp_index)[0].size(); ++j) {
                    prev_regions.insert(comp_with_prev.at(comp_index)[0][j]);
                }
                for (int j = 0; j < comp_with_prev.at(comp_index)[1].size(); ++j) {
                    current_regions.insert(comp_with_prev.at(comp_index)[1][j]);
                }
            }

            final_merge[merged_regions].emplace_back(prev_regions);
            final_merge[merged_regions].emplace_back(current_regions);
            final_merge[merged_regions].emplace_back(post.second[1].begin(), post.second[1].end());
            ++merged_regions;
        } else {
            // ни с кем не пересеклись нужно проверить, что нет связей с post_post
            bool connected_with_post_post = false;
            for (int i = 0; i < post.second[1].size(); ++i) {
                if (map_to_post_post.find(post.second[1][i]) != map_to_post_post.end()) {
                    connected_with_post_post = true;
                    break;
                }
            }
            if (connected_with_post_post)continue;

            final_merge[merged_regions].emplace_back();
            final_merge[merged_regions].emplace_back(post.second[0].begin(), post.second[0].end());
            final_merge[merged_regions].emplace_back(post.second[1].begin(), post.second[1].end());
            used_componets_post.insert(post.first);

            ++merged_regions;
        }
    }
    // теперь разберёмся с той частью второй доли, которая не слилась ни с какой компонентой
    for (auto &prev:comp_with_prev) {
        if (prev_used_in_intersection.find(prev.first) != prev_used_in_intersection.end())continue;
        if (used_componets_prev.find(prev.first) != used_componets_prev.end())continue;
        // ни с кем не пересеклись нужно проверить, что нет связей с prev_prev
        bool connected_with_prev_prev = false;
        for (int i = 0; i < prev.second[0].size(); ++i) {
            if (map_to_prev_prev.find(prev.second[0][i]) != map_to_prev_prev.end()) {
                connected_with_prev_prev = true;
                break;
            }
        }
        if (connected_with_prev_prev)continue;

        final_merge[merged_regions].emplace_back(prev.second[0].begin(), prev.second[0].end());
        final_merge[merged_regions].emplace_back(prev.second[1].begin(), prev.second[1].end());
        final_merge[merged_regions].emplace_back();
        used_componets_prev.insert(prev.first);
        ++merged_regions;
    }

}

void region_growing::merge_regions(std::vector<std::vector<std::vector<int>>> &regions,
                                   std::vector<std::vector<std::vector<int>>> &regions_curr,
                                   const std::vector<std::vector<std::vector<int>>> &regions_prev,
                                   const std::vector<std::vector<std::vector<int>>> &regions_post,
                                   std::vector<bool> &used_prev,
                                   std::vector<bool> &used_curr,
                                   std::vector<bool> &used_post,
                                   const std::map<int, std::vector<std::set<int>>> &final_merge) {

    //добавляем слитые регионы
    for (auto &x:final_merge) {
        int merge_region_index = *x.second[1].begin();
        std::vector<std::vector<int>> &merge_region = regions_curr[merge_region_index];
        used_curr[merge_region_index] = true;
        std::set<int>::iterator it_curr;
        for (it_curr = ++x.second[1].begin(); it_curr != x.second[1].end(); ++it_curr) {
            merge_region[0].insert(merge_region[0].begin(), regions_curr[*it_curr][0].begin(),
                                   regions_curr[*it_curr][0].end());
            merge_region[1].insert(merge_region[1].begin(), regions_curr[*it_curr][1].begin(),
                                   regions_curr[*it_curr][1].end());
            used_curr[*it_curr] = true;
        }

        std::set<int>::iterator it_prev;
        for (it_prev = x.second[0].begin(); it_prev != x.second[0].end(); ++it_prev) {
            merge_region[0].insert(merge_region[0].begin(), regions_prev[*it_prev][0].begin(),
                                   regions_prev[*it_prev][0].end());
            merge_region[1].insert(merge_region[1].begin(), regions_prev[*it_prev][1].begin(),
                                   regions_prev[*it_prev][1].end());
            used_prev[*it_prev] = true;
        }

        std::set<int>::iterator it_post;
        for (it_post = x.second[2].begin(); it_post != x.second[2].end(); ++it_post) {
            merge_region[0].insert(merge_region[0].begin(), regions_post[*it_post][0].begin(),
                                   regions_post[*it_post][0].end());
            merge_region[1].insert(merge_region[1].begin(), regions_post[*it_post][1].begin(),
                                   regions_post[*it_post][1].end());
            used_post[*it_post] = true;
        }

        regions.insert(regions.begin(), regions_curr.begin() + merge_region_index,
                       regions_curr.begin() + merge_region_index + 1);
    }
    //добавляем остальные регионы
    for (int i = 0; i < regions_curr.size(); ++i) {
        if (!used_curr[i]) {
            regions.insert(regions.begin(), regions_curr.begin() + i,
                           regions_curr.begin() + i + 1);
        }
    }
}

std::vector<std::vector<std::vector<int>>>
region_growing::get_regions(const cv::Mat &cls_map, const cv::Mat &angle_map, const cv::Mat &cls_bin) {
    std::vector<std::vector<std::vector<int>>> regions1;
    std::vector<std::vector<std::vector<int>>> regions2;
    std::vector<std::vector<std::vector<int>>> regions3;
    std::vector<std::vector<std::vector<int>>> regions4;

    std::vector<std::vector<float>> regions1_mean_angle(2);
    std::vector<std::vector<float>> regions2_mean_angle(2);
    std::vector<std::vector<float>> regions3_mean_angle(2);
    std::vector<std::vector<float>> regions4_mean_angle(2);

    std::vector<std::vector<bool>> U(angle_map.rows, std::vector<bool>(angle_map.cols));

    std::thread th1(&region_growing::get_regions_one_thread, this, std::ref(cls_map), std::ref(angle_map),
                    std::ref(cls_bin), std::ref(regions1), std::ref(regions1_mean_angle), std::ref(U), 0, 0, 0,
                    angle_map.rows / 2,
                    angle_map.cols / 2);
    std::thread th2(&region_growing::get_regions_one_thread, this, std::ref(cls_map), std::ref(angle_map),
                    std::ref(cls_bin), std::ref(regions2), std::ref(regions2_mean_angle), std::ref(U), 0, 0,
                    angle_map.cols / 2, angle_map.rows / 2,
                    angle_map.cols);
    std::thread th3(&region_growing::get_regions_one_thread, this, std::ref(cls_map), std::ref(angle_map),
                    std::ref(cls_bin), std::ref(regions3), std::ref(regions3_mean_angle), std::ref(U), 0,
                    angle_map.rows / 2, 0, angle_map.rows,
                    angle_map.cols / 2);

    //main_thread
    region_growing::get_regions_one_thread(cls_map, angle_map, cls_bin,
                                           regions4, std::ref(regions4_mean_angle),
                                           U, 0, angle_map.rows / 2, angle_map.cols / 2, angle_map.rows,
                                           angle_map.cols);
    th1.join();
    th2.join();
    th3.join();

    std::vector<std::vector<std::vector<int>>> regions;
    regions.reserve(regions1.size() + regions2.size() + regions3.size() + regions4.size());

#if SAW
    std::vector<std::vector<int>> vert(2, std::vector<int>(cls_map.rows));
    std::vector<std::vector<int>> horiz(2, std::vector<int>(cls_map.cols));
    int length_row = cls_map.rows / 2;
    int double_length_row = cls_map.rows;
    int length_col = cls_map.cols / 2;
    int double_length_cols = cls_map.cols;

    //fill values
    fill_horiz_and_vert(regions1, vert, horiz, length_row, length_col, 1, 1, 0, 0);
    fill_horiz_and_vert(regions2, vert, horiz, length_row, length_col, 1, 0, 0, 1);
    fill_horiz_and_vert(regions3, vert, horiz, length_row, length_col, 0, 1, 1, 0);
    fill_horiz_and_vert(regions4, vert, horiz, length_row, length_col, 0, 0, 1, 1);

//    for (int i = length; i < length * 2; ++i) {
//        std::cout << i << " : " << horiz[0][i] << " " << horiz[1][i] << '\n';
//    }
    //find graphs
    std::map<int, std::vector<int>> first_second_map;
    std::map<int, std::vector<int>> second_first_map;

    std::map<int, std::vector<int>> second_forth_map;
    std::map<int, std::vector<int>> forth_second_map;

    std::map<int, std::vector<int>> forth_third_map;
    std::map<int, std::vector<int>> third_forth_map;

    std::map<int, std::vector<int>> third_first_map;
    std::map<int, std::vector<int>> first_third_map;


    find_graph(0, first_second_map, length_row, vert, regions1_mean_angle, regions2_mean_angle, false);
    find_graph(0, second_first_map, length_row, vert, regions2_mean_angle, regions1_mean_angle, true);

    find_graph(length_col, second_forth_map, double_length_cols, horiz, regions2_mean_angle, regions4_mean_angle,
               false);
    find_graph(length_col, forth_second_map, double_length_cols, horiz, regions4_mean_angle, regions2_mean_angle, true);

    find_graph(length_row, forth_third_map, double_length_row, vert, regions4_mean_angle, regions3_mean_angle, true);
    find_graph(length_row, third_forth_map, double_length_row, vert, regions3_mean_angle, regions4_mean_angle, false);

    find_graph(0, third_first_map, length_col, horiz, regions3_mean_angle, regions1_mean_angle, true);
    find_graph(0, first_third_map, length_col, horiz, regions1_mean_angle, regions3_mean_angle, false);

    //find connectivity components
    std::map<int, std::vector<std::vector<int>>> first_second_components;
    std::map<int, std::vector<std::vector<int>>> second_forth_components;
    std::map<int, std::vector<std::vector<int>>> forth_third_components;
    std::map<int, std::vector<std::vector<int>>> third_first_components;

    std::map<int, int> first_second_RegtoComp;
    std::map<int, int> second_first_RegtoComp;

    std::map<int, int> second_forth_RegtoComp;
    std::map<int, int> forth_second_RegtoComp;

    std::map<int, int> forth_third_RegtoComp;
    std::map<int, int> third_forth_RegtoComp;

    std::map<int, int> third_first_RegtoComp;
    std::map<int, int> first_third_RegtoComp;


    find_connectivity_components(first_second_components, first_second_map, second_first_map, first_second_RegtoComp,
                                 second_first_RegtoComp);
    find_connectivity_components(second_forth_components, second_forth_map, forth_second_map, second_forth_RegtoComp,
                                 forth_second_RegtoComp);
    find_connectivity_components(forth_third_components, forth_third_map, third_forth_map, forth_third_RegtoComp,
                                 third_forth_RegtoComp);
    find_connectivity_components(third_first_components, third_first_map, first_third_map, third_first_RegtoComp,
                                 first_third_RegtoComp);

    //merge connectivity components
    std::map<int, std::vector<std::set<int>>> final_merge1;
    std::map<int, std::vector<std::set<int>>> final_merge2;
    std::map<int, std::vector<std::set<int>>> final_merge3;
    std::map<int, std::vector<std::set<int>>> final_merge4;

    std::set<int> used_componets_first_second;
    std::set<int> used_componets_second_forth;
    std::set<int> used_componets_forth_third;
    std::set<int> used_componets_third_first;

    merge_connectivity_components(final_merge1, third_first_components, first_second_components, third_forth_RegtoComp,
                                  second_forth_RegtoComp, used_componets_third_first, used_componets_first_second);
    merge_connectivity_components(final_merge2, first_second_components, second_forth_components, first_third_RegtoComp,
                                  forth_third_RegtoComp, used_componets_first_second, used_componets_second_forth);
    merge_connectivity_components(final_merge4, second_forth_components, forth_third_components, second_first_RegtoComp,
                                  third_first_RegtoComp, used_componets_second_forth, used_componets_forth_third);
    merge_connectivity_components(final_merge3, forth_third_components, third_first_components, forth_second_RegtoComp,
                                  first_second_RegtoComp, used_componets_forth_third, used_componets_third_first);
    // actual merging

    std::vector<bool> used_regions1(regions1.size());
    std::vector<bool> used_regions2(regions2.size());
    std::vector<bool> used_regions3(regions3.size());
    std::vector<bool> used_regions4(regions4.size());

    merge_regions(regions, regions1, regions3, regions2, used_regions3, used_regions1, used_regions2, final_merge1);
    merge_regions(regions, regions2, regions1, regions4, used_regions1, used_regions2, used_regions4, final_merge2);
    merge_regions(regions, regions4, regions2, regions3, used_regions2, used_regions4, used_regions3, final_merge4);
    merge_regions(regions, regions3, regions4, regions1, used_regions4, used_regions3, used_regions1, final_merge3);
    return regions;


#else
    regions.insert(regions.end(), regions1.begin(), regions1.end());
regions.insert(regions.end(), regions2.begin(), regions2.end());
regions.insert(regions.end(), regions3.begin(), regions3.end());
regions.insert(regions.end(), regions4.begin(), regions4.end());


#endif
    return regions;
}

void region_growing::get_regions_one_thread(const cv::Mat &cls_map, const cv::Mat &angle_map, const cv::Mat &cls_bin,
                                            std::vector<std::vector<std::vector<int>>> &regions,
                                            std::vector<std::vector<float>> &regions_mean_angle,
                                            std::vector<std::vector<bool>> &U, int shift, int top_corner_x,
                                            int top_corner_y, int bot_corner_x, int bot_corner_y) {
    std::vector<std::vector<int>> G(2);
    std::vector<float> D;


    for (int i = top_corner_x; i < bot_corner_x; ++i) {
        for (int j = top_corner_y; j < bot_corner_y; ++j) {
            if (cls_bin.at<uchar>(i, j)) {
                G[0].push_back(i); // TO OPTIMISE . maybe we can preallocate G
                G[1].push_back(j);
                D.push_back(cls_map.at<float>(i, j));
            }
        }
    }
    // here we need to do argsort of D

    // initialize original index locations
    std::vector<size_t> idx(D.size());
    iota(idx.begin(), idx.end(), 0);
    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    stable_sort(idx.begin(), idx.end(),
                [&D](size_t i1, size_t i2) { return D[i1] < D[i2]; });


    std::vector<std::vector<int>> S(2, std::vector<int>(D.size()));

    for (size_t i = 0; i < idx.size(); i++) {
        int idx_i = idx[i];
        S[0][i] = G[0][idx_i];
        S[1][i] = G[1][idx_i];
    }
    int i = 0;
    int S_index = S[0].size() - 1;

    while (S_index >= 0) {
        i++;
        int root[2];
        root[0] = S[0][S_index];
        root[1] = S[1][S_index];
        S_index--;
        while (U[root[0]][root[1]]) {
            if (S_index < 0) break;

            root[0] = S[0][S_index];
            root[1] = S[1][S_index];
            S_index--;
        }
        if (S_index < 0) break;
        std::pair<std::vector<std::vector<int>>, std::pair<float, float>> region_and_angle = region_grouping(root,
                                                                                                             cls_map,
                                                                                                             angle_map,
                                                                                                             cls_bin, U,
                                                                                                             top_corner_x,
                                                                                                             top_corner_y,
                                                                                                             bot_corner_x,
                                                                                                             bot_corner_y);

        if (region_and_angle.first[0].size() > min_region_size) {
            regions.push_back(region_and_angle.first);
            regions_mean_angle[0].push_back(region_and_angle.second.first);
            regions_mean_angle[1].push_back(region_and_angle.second.second);
        } else i--;
    }

}

std::pair<std::vector<std::vector<int>>, std::pair<float, float>>
region_growing::region_grouping(int root[2], const cv::Mat &cls_map, const cv::Mat &angle_map,
                                const cv::Mat &cls_bin, std::vector<std::vector<bool>> &U, int top_corner_x,
                                int top_corner_y, int bot_corner_x, int bot_corner_y) const {
    std::vector<std::vector<int>> region(2);

    float region_mean = cls_map.at<float>(root[0], root[1]);
    float ang = angle_map.at<float>(root[0], root[1]) * M_PI;
    float cos0 = std::cos(ang);
    float sin0 = std::sin(ang);
    float V_mean_x = cos0 * cos0 - sin0 * sin0;
    float V_mean_y = 2 * sin0 * cos0;
    float Norm_region = 1;

    region[0].push_back(root[0]);
    region[1].push_back(root[1]);

    int neighborhood_max_size = (2 * r + 1) * (2 * r + 1);
    std::vector<std::queue<int>> newly_added(2);
    newly_added[0].push(root[0]);
    newly_added[1].push(root[1]);
    U[root[0]][root[1]] = true;
    int region_len = 1;

    int **neighborhood;
    neighborhood = new int *[2];
    int neighborhood_size = 0;
    for (int i = 0; i < 2; ++i) {
        neighborhood[i] = new int[neighborhood_max_size];
    }
    while (!newly_added[0].empty()) {

        neighborhood_size = get_r_neighborhood(newly_added[0].front(), newly_added[1].front(), neighborhood, cls_bin,
                                               U, top_corner_x, top_corner_y, bot_corner_x, bot_corner_y);
        newly_added[0].pop();
        newly_added[1].pop();

        for (int j = 0; j < neighborhood_size; ++j) {
            int x = neighborhood[0][j];
            int y = neighborhood[1][j];
            ang = angle_map.at<float>(x, y) * M_PI;
            cos0 = std::cos(ang);
            sin0 = std::sin(ang);
            float V_x = cos0 * cos0 - sin0 * sin0;
            float V_y = 2 * sin0 * cos0;

            float probability = cls_map.at<float>(x, y);
            if ((V_mean_x - V_x) * (V_mean_x - V_x) + (V_mean_y - V_y) * (V_mean_y - V_y)
                < tau and
                abs(region_mean - probability) < deviation_thresh) {
                region[0].push_back(x);
                region[1].push_back(y);
                newly_added[0].push(x);
                newly_added[1].push(y);

                U[x][y] = true;

                region_len++;
                auto new_Norm_region = static_cast<float > (std::sqrt(
                        1 + Norm_region * Norm_region + 2 * Norm_region * (V_mean_x * V_x + V_mean_y * V_y)));

                V_mean_x = (V_mean_x * Norm_region + V_x) / new_Norm_region;
                V_mean_y = (V_mean_y * Norm_region + V_y) / new_Norm_region;
                Norm_region = new_Norm_region;

                region_mean = (region_mean * (region_len - 1) + probability) / region_len;
            }
        }
    }
    for (int i = 0; i < 2; ++i) {
        delete[] neighborhood[i];
    }
    delete[] neighborhood;

    std::pair<std::vector<std::vector<int>>, std::pair<float, float>> ret_value(region,
                                                                                std::pair<float, float>(V_mean_x,
                                                                                                        V_mean_y));
    return ret_value;
}

inline int region_growing::get_r_neighborhood(int x, int y, int **neighborhood, const cv::Mat &cls_bin,
                                              std::vector<std::vector<bool>> &U, int top_corner_x, int top_corner_y,
                                              int bot_corner_x, int bot_corner_y) const {
    assert(r == 1);
    //??? TO OPTIMISE . x - 1 >= 0 and so on is always true, due to the construction of cls_bin(no active borders)?
    int neighborhood_size = 0;
    int max_size = cls_bin.cols;
    if (x - 1 >= top_corner_x) {
        if (y - 1 >= top_corner_y and cls_bin.at<uchar>(x - 1, y - 1) and !U[x - 1][y - 1]) {
            neighborhood[0][neighborhood_size] = x - 1;
            neighborhood[1][neighborhood_size] = y - 1;
            neighborhood_size++;
        }
        if (cls_bin.at<uchar>(x - 1, y) and !U[x - 1][y]) {
            neighborhood[0][neighborhood_size] = x - 1;
            neighborhood[1][neighborhood_size] = y;
            neighborhood_size++;
        }
        if (y + 1 < bot_corner_y and cls_bin.at<uchar>(x - 1, y + 1) and !U[x - 1][y + 1]) {
            neighborhood[0][neighborhood_size] = x - 1;
            neighborhood[1][neighborhood_size] = y + 1;
            neighborhood_size++;
        }
    }
    if (y - 1 >= top_corner_y and cls_bin.at<uchar>(x, y - 1) and !U[x][y - 1]) {
        neighborhood[0][neighborhood_size] = x;
        neighborhood[1][neighborhood_size] = y - 1;
        neighborhood_size++;
    }
    if (y + 1 < bot_corner_y and cls_bin.at<uchar>(x, y + 1) and !U[x][y + 1]) {
        neighborhood[0][neighborhood_size] = x;
        neighborhood[1][neighborhood_size] = y + 1;
        neighborhood_size++;
    }
    if (x + 1 < bot_corner_x) {
        if (y - 1 >= top_corner_y and cls_bin.at<uchar>(x + 1, y - 1) and !U[x + 1][y - 1]) {
            neighborhood[0][neighborhood_size] = x + 1;
            neighborhood[1][neighborhood_size] = y - 1;
            neighborhood_size++;
        }
        if (cls_bin.at<uchar>(x + 1, y) and !U[x + 1][y]) {
            neighborhood[0][neighborhood_size] = x + 1;
            neighborhood[1][neighborhood_size] = y;
            neighborhood_size++;
        }
        if (y + 1 < bot_corner_y and cls_bin.at<uchar>(x + 1, y + 1) and !U[x + 1][y + 1]) {
            neighborhood[0][neighborhood_size] = x + 1;
            neighborhood[1][neighborhood_size] = y + 1;
            neighborhood_size++;
        }
    }
    return neighborhood_size;
}