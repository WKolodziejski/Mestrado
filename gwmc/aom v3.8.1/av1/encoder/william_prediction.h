#ifndef AOM_WILLIAM_PREDICTION_H
#define AOM_WILLIAM_PREDICTION_H

struct gwmc_stats {
  char name[200];
  int src_width;
  int src_height;
  double time_compute_global_motion;
  double time_compute_feature_segmentation_map;
  double time_segmented_frame_error;
  double time_fast_warp_error_segmented;
  double time_refine_integerized_param;
  double time_prediction;
  int skipped;
  int is_enough_erroradvantage;
};

void collect_init();

void collect_frame(int src_width, int src_height);

void time_compute_global_motion(double time_compute_global_motion);

void time_compute_feature_segmentation_map(
    double time_compute_feature_segmentation_map);

void time_segmented_frame_error(double time_segmented_frame_error);

void time_fast_warp_error_segmented(double time_fast_warp_error_segmented);

void time_refine_integerized_param(double time_refine_integerized_param);

void time_prediction(double time_prediction);

void skipped(int skipped);

void is_enough_erroradvantage(int is_enough_erroradvantage);

void collect_finalize();

void collect_open_file(const char *fn);

void collect_close_file();

int predict_should_skip(double input[8]);

#endif  // AOM_WILLIAM_PREDICTION_H
