// Copyright (c) 2022-2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#ifndef DALI_OPERATORS_IMGCODEC_UTIL_OUTPUT_SHAPE_H_
#define DALI_OPERATORS_IMGCODEC_UTIL_OUTPUT_SHAPE_H_

#include "dali/core/error_handling.h"
#include "dali/operators/imgcodec/imgcodec.h"

namespace dali {
namespace imgcodec {

template <typename OutShape>
void OutputShape(OutShape &&out_shape, const ImageInfo &info, DALIImageType format, bool planar,
                 bool use_orientation, const ROI &roi) {
  int ndim = info.shape.sample_dim();
  resize_if_possible(out_shape, ndim);
  assert(size(out_shape) == ndim);  // check the size, in case we couldn't resize

  int spatial_ndim = ndim - 1;
  int in_channel_dim = ndim - 1;
  int num_channels = NumberOfChannels(format, info.shape[in_channel_dim]);
  int out_channel_dim = planar ? 0 : ndim - 1;

  out_shape[out_channel_dim] = num_channels;

  bool rotate = use_orientation && ((info.orientation.rotated / 90) & 1);
  if (rotate && spatial_ndim != 2) {
    throw std::logic_error("Orientation only applies to 2D images.");
  }

  for (int d = 0; d < spatial_ndim; d++) {
    int in_d = d;
    if (rotate) {
      in_d = 1 - d;
    }

    int extent = info.shape[in_d];
    if (d < roi.end.size()) {
      DALI_ENFORCE(0 <= roi.end[d] && roi.end[d] <= info.shape[in_d],
                   "ROI end must fit within the image bounds");
      extent = roi.end[d];
    }
    if (d < roi.begin.size()) {
      DALI_ENFORCE(0 <= roi.begin[d] && roi.begin[d] <= info.shape[in_d],
                   "ROI begin must fit within the image bounds");
      extent -= roi.begin[d];
    }

    out_shape[static_cast<int>(planar) + d] = extent;
  }
}

}  // namespace imgcodec
}  // namespace dali

#endif  // DALI_OPERATORS_IMGCODEC_UTIL_OUTPUT_SHAPE_H_
