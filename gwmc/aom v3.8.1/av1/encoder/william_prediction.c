#include <stdio.h>
#include <string.h>
#include "william_prediction.h"
#include "william_model.c"

static FILE *collect_stats_file;
struct gwmc_stats gwmc_stats;

void collect_init() {
  gwmc_stats.src_height = 0;
  gwmc_stats.src_width = 0;
  gwmc_stats.time_compute_global_motion = 0;
  gwmc_stats.time_compute_feature_segmentation_map = 0;
  gwmc_stats.time_segmented_frame_error = 0;
  gwmc_stats.time_fast_warp_error_segmented = 0;
  gwmc_stats.time_refine_integerized_param = 0;
  gwmc_stats.time_prediction = 0;
  gwmc_stats.skipped = 0;
  gwmc_stats.is_enough_erroradvantage = 0;
}

void collect_frame(int src_width, int src_height) {
  gwmc_stats.src_width = src_width;
  gwmc_stats.src_height = src_height;
}

void time_compute_global_motion(double time_compute_global_motion) {
  gwmc_stats.time_compute_global_motion = time_compute_global_motion;
}

void time_compute_feature_segmentation_map(
    double time_compute_feature_segmentation_map) {
  gwmc_stats.time_compute_feature_segmentation_map =
      time_compute_feature_segmentation_map;
}

void time_segmented_frame_error(double time_segmented_frame_error) {
  gwmc_stats.time_segmented_frame_error = time_segmented_frame_error;
}

void time_fast_warp_error_segmented(double time_fast_warp_error_segmented) {
  gwmc_stats.time_fast_warp_error_segmented = time_fast_warp_error_segmented;
}

void time_refine_integerized_param(double time_refine_integerized_param) {
  gwmc_stats.time_refine_integerized_param = time_refine_integerized_param;
}

void time_prediction(double time_prediction) {
  gwmc_stats.time_prediction = time_prediction;
}

void skipped(int skipped) { gwmc_stats.skipped = skipped; }

void is_enough_erroradvantage(int is_enough_erroradvantage) {
  gwmc_stats.is_enough_erroradvantage = is_enough_erroradvantage;
}

void collect_finalize() {
  fprintf(collect_stats_file, "%s, %d, %d, %f, %f, %f, %f, %f, %f, %d, %d\n",
          gwmc_stats.name, gwmc_stats.src_height, gwmc_stats.src_width,
          gwmc_stats.time_compute_global_motion,
          gwmc_stats.time_compute_feature_segmentation_map,
          gwmc_stats.time_segmented_frame_error,
          gwmc_stats.time_fast_warp_error_segmented,
          gwmc_stats.time_refine_integerized_param, gwmc_stats.time_prediction,
          gwmc_stats.is_enough_erroradvantage, gwmc_stats.skipped);
}

void collect_open_file(const char *fn) {
  strcpy(gwmc_stats.name, fn);
  strcat(gwmc_stats.name, ".csv");
  collect_stats_file = fopen(gwmc_stats.name, "w+");
}

void collect_close_file() { fclose(collect_stats_file); }

int predict_should_skip(double input[8]) {
  double output[2] = { 0, 0 };

  score(input, output);

  // Classe 0 (skip) retona na posição output[0]
  return output[0] > output[1];
}