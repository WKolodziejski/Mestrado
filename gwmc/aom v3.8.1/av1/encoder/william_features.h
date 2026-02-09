#ifndef AOM_WILLIAM_FEATURES_H
#define AOM_WILLIAM_FEATURES_H

// features in order they appear in code
struct gwmc_features {
  // compute_global_motion_for_references
  int distance;
  int num_ref_frames;
  int frame;

  // cpi->oxcf->rc_cfg
  int cq_level;

  // cm->cur_frame
  int cur_ref_count;
  int cur_order_hint;
  int cur_showable_frame;

  // cm->ref_frame_map[frame]
  int ref_ref_count;
  int ref_order_hint;
  int ref_showable_frame;

  // compute_global_motion_for_ref_frame
  int src_width;
  int src_height;
  int src_stride;
  int bit_depth;
  int segment_map_w;
  int segment_map_h;
  int downsample_level;
  int num_refinements;

  // av1_compute_global_motion_disflow
  int src_max_levels;
  int src_filled_levels;
  int src_pyr_width;
  int src_pyr_height;
  int src_pyr_stride;
  int ref_max_levels;
  int ref_filled_levels;
  int ref_pyr_width;
  int ref_pyr_height;
  int ref_pyr_stride;
  int src_layers;
  int ref_layers;
  int num_corners;
  int num_correspondences;
  int num_inliers;

  // compute_global_motion_for_ref_frame
  double params_0;
  double params_1;
  double params_2;
  double params_3;
  double params_4;
  double params_5;
  double alpha;
  double beta;
  double gamma;
  double delta;

  // av1_compute_feature_segmentation_map
  int seg_count;

  // compute_global_motion_for_ref_frame
  long ref_frame_error;
  long fast_error;
  long pre_ref_frame_error;
  long warp_error;  // <- valor a ser predito

  // custom features (não deve ser usado como feature no treinamento!!!)
  int selected;
  double inliers_rate;
  char name[200];
};

void collect_init();

void collect_compute_global_motion_for_references(int distance,
                                                  int num_ref_frames,
                                                  int frame);

void collect_cpi(int cq_level);

void collect_cm(int cur_ref_count, int cur_order_hint, int cur_showable_frame,
                int ref_ref_count, int ref_order_hint, int ref_showable_frame);

void collect_compute_global_motion_for_ref_frame(int src_width, int src_height,
                                                 int src_stride, int bit_depth,
                                                 int segment_map_w,
                                                 int segment_map_h,
                                                 int downsample_level,
                                                 int num_refinements);

void collect_av1_compute_global_motion_disflow(
    int src_max_levels, int src_filled_levels, int src_pyr_width,
    int src_pyr_height, int src_pyr_stride, int ref_max_levels,
    int ref_filled_levels, int ref_pyr_width, int ref_pyr_height,
    int ref_pyr_stride, int src_layers, int ref_layers, int num_corners,
    int num_correspondences, int num_inliers);

void collect_affine_params(double params_0, double params_1, double params_2,
                           double params_3, double params_4, double params_5,
                           double alpha, double beta, double gamma,
                           double delta);

void collect_av1_compute_feature_segmentation_map(int seg_count);

void collect_error(long ref_frame_error, long fast_error, long pre_ref_frame_error);

void collect_warp_error(long warp_error);

void collect_selected(int selected);

void collect_finalize();

void collect_open_file(const char *fn);

void collect_close_file();

#endif
