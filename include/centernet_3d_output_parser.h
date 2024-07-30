// Copyright (c) 2024，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DNN_NODE_3D_OUTPUT_PARSER_H
#define DNN_NODE_3D_OUTPUT_PARSER_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "dnn_node/dnn_node_data.h"

using hobot::dnn_node::DNNTensor;
using hobot::dnn_node::Model;

struct BBox {
  inline BBox() {}
  inline BBox(float x1_, float y1_, float x2_, float y2_, float score_ = 0.0f,
              int32_t id_ = -1, const std::string &category_name_ = "") {
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
    id = id_;
    score = score_;
    category_name = category_name_;
  }
  inline float Width() const { return (x2 - x1); }
  inline float Height() const { return (y2 - y1); }
  inline float CenterX() const { return (x1 + (x2 - x1) / 2); }
  inline float CenterY() const { return (y1 + (y2 - y1) / 2); }

  inline friend std::ostream &operator<<(std::ostream &out, BBox &bbox) {
    out << "( x1: " << bbox.x1 << " y1: " << bbox.y1 << " x2: " << bbox.x2
        << " y2: " << bbox.y2 << " score: " << bbox.score << " )";
    return out;
  }

  inline friend std::ostream &operator<<(std::ostream &out, const BBox &bbox) {
    out << "( x1: " << bbox.x1 << " y1: " << bbox.y1 << " x2: " << bbox.x2
        << " y2: " << bbox.y2 << " score: " << bbox.score << " )";
    return out;
  }

  float x1 = 0;
  float y1 = 0;
  float x2 = 0;
  float y2 = 0;
  float score = 0.0;
  int32_t id = 0;
  float rotation_angle = 0.0;
  std::string category_name = "";
};

struct BBox2D {
  float x1, y1, x2, y2;
  int32_t score;
  uint32_t cls;
  uint32_t idx;
  inline static bool greater(const BBox2D &a, const BBox2D &b) {
    return a.score > b.score;
  }
};

struct BBox3D {
  enum CLASS_LABEL_TYPE {
     CHARGING_BASE = 0,
     TRASH_CAN,
     SLIPPER
  };
  uint32_t grid_idx, grid_x, grid_y;
  float x, y, z;
  float w, l, h;
  float d, r;
  std::vector<std::vector<float>> corners2d;          // undistort, img ord
  std::vector<std::vector<float>> corners2d_upscale;  // undistort, img ord
  std::vector<std::vector<float>> corners3d;          // undistort, img ord
  float score;
  uint16_t class_label;
  BBox3D() {
//    corners2d.resize(2, std::vector<float>(8));
//    corners3d.resize(3, std::vector<float>(8));
//    corners2d_upscale.resize(2, std::vector<float>(8));
     #define BBOX3D_CORNER_SIZE 8
     corners2d.resize(BBOX3D_CORNER_SIZE, std::vector<float>(2));
     corners2d_upscale.resize(BBOX3D_CORNER_SIZE, std::vector<float>(2));
     corners3d.resize(BBOX3D_CORNER_SIZE, std::vector<float>(3));
  }
};

class CenterNet3DDetResult {
 public:
  std::vector<BBox3D> boxes;
  void Reset() { boxes.clear(); }
};

class CenterNet3DOutputParser {
 public:
  CenterNet3DOutputParser(const std::string &config_file) {
    yaml_file_ = config_file + "/centernet.yaml";
  }

  int PostProcess(std::vector<std::shared_ptr<DNNTensor>> &tensors,
                  std::vector<BBox3D> &det_result);

 private:
 
  int Parse2DBox(std::shared_ptr<DNNTensor> &output_tensor,
                 std::vector<BBox2D> &dets);

 private:
  std::vector<std::vector<uint8_t>> output_shifts_;
  std::vector<int> channel_size_;
  std::vector<int> aligned_channel_size_;
  std::vector<int> aligned_width_size_;
  // std::vector<std::vector<float>> kCalibMatrix_{
  //     {1551.085693359375f, 0.0f, 960.1524658203125f, 0.0f},
  //     {0.0f, 1551.085693359375f, 550.1450805664063f, 0.0f},
  //     {0.0f, 0.0f, 1.0f, 0.0f}};
  std::vector<std::vector<float>> kCalibMatrix_{
      {746.2463540682126, 0.0, 971.6589299894808, 0.0},
      {0.0, 750.2202098997767, 514.5994408429885, 0.0},
      {0.0, 0.0, 1.0, 0.0}};

  void Get3DBboxCorners(std::vector<float> &loc3d, BBox3D &bbox);
  void ConvertCornerToStandupBox(std::vector<BBox3D> &bbox,
                                 std::vector<BBox> &bbox2d);
  float GetSimpleRotY(float alpha, std::vector<float> &loc);
  int MaxPoolingRefine(std::vector<BBox3D> &boxes, void *heat_map, int layer);
  void Create3DBBox(std::vector<BBox3D> &boxes, uint32_t grid_x,
                    uint32_t grid_y, float score, uint16_t class_label,
                    uint32_t grid_idx);
  void ParseBBoxes(std::vector<BBox3D> &bbox, std::vector<BBox> &bbox2d,
                   std::vector<std::vector<float>> &feature_map);
  void ProjectToImage(BBox3D &bbox);
  void ProjLocTo3D(float cx, float cy, float depth, std::vector<float> &loc3d);
  float GetMultiBinRotY(float alpha, float cx);
  float GetSimpleAlpha(float *rot, uint32_t idx, uint32_t rot_dim = 2);
  float GetMultiBinAlpha(float *rot, uint32_t idx, uint32_t rot_dim = 8);
  int Nms2D(std::vector<BBox3D> &bboxes3d, std::vector<BBox> &bboxes,
            bool suppress);
  int NmsBev(std::vector<BBox3D> &bboxes3d, std::vector<BBox> &bboxes2d_bev,
             bool suppress);
  int GetSpecFeatureMap(void *virAddr, const BBox3D &box, void **data,
                        int layer);

 private:
  std::string yaml_file_;
  bool is_parameter_init_ = false;
  int model_output_height_, model_output_width_;
  int model_input_height_ = 512, model_input_width_ = 960;
  bool use_multibin_ = false;
  uint16_t pool_height_, pool_width_, pool_stride_ = 1, pool_kernel_ = 3;
  uint16_t pool_pad_ = (pool_kernel_ - 1) / 2;

  uint16_t nms_size_ = 100;
  float model_focal_length_ = 740.38,
        cam_focal_length_ = model_focal_length_, focal_length_scale_;
  float down_ratio_, output_scale_, image_shift_ = 24.f;

  std::vector<float> box_score_init_;
  std::vector<float> box_score_thres_;
  float box_score_th_ = 0.5, box_log_score_th_;
  void ParameterInit(std::vector<std::shared_ptr<DNNTensor>> &tensors);

  float iou_th_ = 0.5, beviou_th_ = 0.5;

};

#endif  // DNN_NODE_3D_OUTPUT_PARSER_H
