#include <stdio.h>
#include <string.h>
#include "william_features.h"

static struct gwmc_features f;
static FILE *collect_ft_file;

void collect_init() {
  fprintf(stderr, "collect_init\n");

  f.distance = -1;
  f.num_ref_frames = -1;
  f.cq_level = -1;
  f.cur_ref_count = -1;
  f.cur_order_hint = -1;
  f.cur_showable_frame = -1;
  f.ref_ref_count = -1;
  f.ref_order_hint = -1;
  f.ref_showable_frame = -1;
  f.src_width = -1;
  f.src_height = -1;
  f.src_stride = -1;
  f.bit_depth = -1;
  f.frame = -1;
  f.segment_map_w = -1;
  f.segment_map_h = -1;
  f.downsample_level = -1;
  f.num_refinements = -1;
  f.src_max_levels = -1;
  f.src_filled_levels = -1;
  f.src_pyr_width = -1;
  f.src_pyr_height = -1;
  f.src_pyr_stride = -1;
  f.ref_max_levels = -1;
  f.ref_filled_levels = -1;
  f.ref_pyr_width = -1;
  f.ref_pyr_height = -1;
  f.ref_pyr_stride = -1;
  f.src_layers = -1;
  f.ref_layers = -1;
  f.num_corners = -1;
  f.num_correspondences = -1;
  f.num_inliers = -1;
  f.params_0 = -1;
  f.params_1 = -1;
  f.params_2 = -1;
  f.params_3 = -1;
  f.params_4 = -1;
  f.params_5 = -1;
  f.alpha = -1;
  f.beta = -1;
  f.gamma = -1;
  f.delta = -1;
  f.seg_count = -1;
  f.ref_frame_error = -1;
  f.fast_error = -1;
  f.warp_error = -1;
  f.pre_ref_frame_error = -1;
  // custom
  f.selected = 0;  // não deve ser usado como feature no treinamento!!!
  f.inliers_rate = -1;
}

void collect_compute_global_motion_for_references(int distance,
                                                  int num_ref_frames,
                                                  int frame) {
  fprintf(stderr, "collect_compute_global_motion_for_references\n");

  f.distance = distance;
  f.num_ref_frames = num_ref_frames;
  f.frame = frame;
}

void collect_cpi(int cq_level) {
  fprintf(stderr, "collect_cpi\n");
  f.cq_level = cq_level;
}

void collect_cm(int cur_ref_count, int cur_order_hint, int cur_showable_frame,
                int ref_ref_count, int ref_order_hint, int ref_showable_frame) {
  fprintf(stderr, "collect_cm\n");

  f.cur_ref_count = cur_ref_count;
  f.cur_order_hint = cur_order_hint;
  f.cur_showable_frame = cur_showable_frame;
  f.ref_ref_count = ref_ref_count;
  f.ref_order_hint = ref_order_hint;
  f.ref_showable_frame = ref_showable_frame;
}

void collect_compute_global_motion_for_ref_frame(int src_width, int src_height,
                                                 int src_stride, int bit_depth,
                                                 int segment_map_w,
                                                 int segment_map_h,
                                                 int downsample_level,
                                                 int num_refinements) {
  fprintf(stderr, "collect_compute_global_motion_for_ref_frame\n");

  f.src_width = src_width;
  f.src_height = src_height;
  f.src_stride = src_stride;
  f.bit_depth = bit_depth;
  f.segment_map_w = segment_map_w;
  f.segment_map_h = segment_map_h;
  f.downsample_level = downsample_level;
  f.num_refinements = num_refinements;
}

void collect_av1_compute_global_motion_disflow(
    int src_max_levels, int src_filled_levels, int src_pyr_width,
    int src_pyr_height, int src_pyr_stride, int ref_max_levels,
    int ref_filled_levels, int ref_pyr_width, int ref_pyr_height,
    int ref_pyr_stride, int src_layers, int ref_layers, int num_corners,
    int num_correspondences, int num_inliers) {
  fprintf(stderr, "collect_av1_compute_global_motion_disflow\n");

  f.src_max_levels = src_max_levels;
  f.src_filled_levels = src_filled_levels;
  f.src_pyr_width = src_pyr_width;
  f.src_pyr_height = src_pyr_height;
  f.src_pyr_stride = src_pyr_stride;
  f.ref_max_levels = ref_max_levels;
  f.ref_filled_levels = ref_filled_levels;
  f.ref_pyr_width = ref_pyr_width;
  f.ref_pyr_height = ref_pyr_height;
  f.ref_pyr_stride = ref_pyr_stride;
  f.src_layers = src_layers;
  f.ref_layers = ref_layers;
  f.num_corners = num_corners;
  f.num_correspondences = num_correspondences;
  f.num_inliers = num_inliers;

  if (num_correspondences > 0) {
    f.inliers_rate = (double)num_inliers / (double)num_correspondences;
  } else {
    f.inliers_rate = 0;
  }
}

void collect_affine_params(double params_0, double params_1, double params_2,
                           double params_3, double params_4, double params_5,
                           double alpha, double beta, double gamma,
                           double delta) {
  fprintf(stderr, "collect_affine_params\n");
  f.params_0 = params_0;
  f.params_1 = params_1;
  f.params_2 = params_2;
  f.params_3 = params_3;
  f.params_4 = params_4;
  f.params_5 = params_5;
  f.alpha = alpha;
  f.beta = beta;
  f.gamma = gamma;
  f.delta = delta;
}

void collect_av1_compute_feature_segmentation_map(int seg_count) {
  fprintf(stderr, "collect_av1_compute_feature_segmentation_map\n");
  f.seg_count = seg_count;
}

void collect_error(long ref_frame_error, long fast_error, long pre_ref_frame_error) {
  fprintf(stderr, "collect_error\n");
  f.ref_frame_error = ref_frame_error;
  f.fast_error = fast_error;
  f.pre_ref_frame_error = pre_ref_frame_error;
}

void collect_warp_error(long warp_error) {
  fprintf(stderr, "collect_warp_error\n");
  f.warp_error = warp_error;
}

void collect_selected(int selected) {
  fprintf(stderr, "collect_selected\n");
  f.selected = selected;
}

void collect_finalize() {
  fprintf(stderr, "collect_finalize\n");
  // Se não coletou erro, não adiciona ao csv
  if (f.ref_frame_error < 0 || f.warp_error < 0)
    return;

  fprintf(collect_ft_file,
          "%s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
          "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %f, "
          "%f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %ld, %ld, %ld, %ld, %d, %f\n",
          f.name, f.distance, f.num_ref_frames, f.cq_level, f.cur_ref_count,
          f.cur_order_hint, f.cur_showable_frame, f.ref_ref_count,
          f.ref_order_hint, f.ref_showable_frame, f.src_width, f.src_height,
          f.src_stride, f.bit_depth, f.frame, f.segment_map_w, f.segment_map_h,
          f.downsample_level, f.num_refinements, f.src_max_levels,
          f.src_filled_levels, f.src_pyr_width, f.src_pyr_height,
          f.src_pyr_stride, f.ref_max_levels, f.ref_filled_levels,
          f.ref_pyr_width, f.ref_pyr_height, f.ref_pyr_stride, f.src_layers,
          f.ref_layers, f.num_corners, f.num_correspondences, f.num_inliers,
          f.params_0, f.params_1, f.params_2, f.params_3, f.params_4,
          f.params_5, f.alpha, f.beta, f.gamma, f.delta, f.seg_count,
          f.ref_frame_error, f.fast_error, f.warp_error, f.pre_ref_frame_error,
          // custom
          f.selected,  // não deve ser usado como feature no treinamento!!!
          f.inliers_rate);
}

void collect_open_file(const char *fn) {
  fprintf(stderr, "collect_open_file\n");
  strcpy(f.name, fn);
  strcat(f.name,".csv");

  collect_ft_file = fopen(f.name, "w+");

//  fprintf(
//      collect_ft_file,
//      "name, distance, num_ref_frames, cq_level, cur_ref_count, cur_order_hint, "
//      "cur_showable_frame, ref_ref_count, ref_order_hint, ref_showable_frame, "
//      "src_width, src_height, src_stride, bit_depth, frame, segment_map_w, "
//      "segment_map_h, downsample_level, num_refinements, src_max_levels, "
//      "src_filled_levels, src_pyr_width, src_pyr_height, src_pyr_stride, "
//      "ref_max_levels, ref_filled_levels, ref_pyr_width, ref_pyr_height, "
//      "ref_pyr_stride, src_layers, ref_layers, num_corners, "
//      "num_correspondences, num_inliers, params_0, params_1, params_2, "
//      "params_3, params_4, params_5, alpha, beta, gamma, delta, seg_count, "
//      "ref_frame_error, fast_error, warp_error, pre_ref_frame_error, selected, inliers_rate\n");
}

void collect_close_file() {
  fprintf(stderr, "collect_close_file\n");
  fclose(collect_ft_file);
}