#include "features_collect.h"
#include <stdio.h>
#include <string.h>

static char filename[200];
static unsigned int seed = 21743;

static char *bsizes[] = { "4X4",     "4X8",   "8X4",   "8X8",    "8X16",
                          "16X8",    "16X16", "16X32", "32X16",  "32X32",
                          "32X64",   "64X32", "64X64", "64X128", "128X64",
                          "128X128", "4X16",  "16X4",  "8X32",   "32X8",
                          "16X64",   "64X16", "ALL" };

static char *plane_types[] = { "Y", "UV", "T" };

static char *partition_types[] = { "NONE",   "HORZ",   "VERT",   "SPLIT",
                                   "HORZ_A", "HORZ_B", "VERT_A", "VERT_B",
                                   "HORZ_4", "VERT_4", "T" };

static char *prediction_modes[] = { "DC",
                                    "V",
                                    "H",
                                    "D45",
                                    "D135",
                                    "D113",
                                    "D157",
                                    "D203",
                                    "D67",
                                    "SMOOTH",
                                    "SMOOTH_V",
                                    "SMOOTH_H",
                                    "PAETH",
                                    "NEARESTMV",
                                    "NEARMV",
                                    "GLOBALMV",
                                    "NEWMV",
                                    "NEAREST_NEARESTMV",
                                    "NEAR_NEARMV",
                                    "NEAREST_NEWMV",
                                    "NEW_NEARESTMV",
                                    "NEAR_NEWMV",
                                    "NEW_NEARMV",
                                    "GLOBAL_GLOBALMV",
                                    "NEW_NEWMV",
                                    "M" };

static char *uv_prediction_modes[] = {
  "DC",  "V",      "H",        "D45",      "D135",  "D113", "D157",  "D203",
  "D67", "SMOOTH", "SMOOTH_V", "SMOOTH_H", "PAETH", "CFL",  "INTRA", "INVALID",
};

static char *motion_modes[] = { "TRANS", "OBMC", "WARP", "M" };

static char *interintra_modes[] = { "II_DC", "II_V", "II_H", "II_SMOOTH",
                                    "INTERINTRA" };

static char *frame_types[] = { "KEY", "INTER", "INTRA", "S", "T" };

static char *superres_modes[] = { "NONE", "FIXED", "RANDOM", "QTHRESH",
                                  "AUTO" };

static char *screen_content_tools[] = { "OFF", "ON", "ADPT" };

static char *force_integer_mvs[] = { "DONT", "INT", "ADPT" };

static char *bitstream_profiles[] = { "0", "1", "2", "P" };

static char *color_primaries[] = { "R_0",       "BT_709",    "U",
                                   "R_3",       "BT_470_M",  "BT_470_B_G",
                                   "BT_601",    "SMPTE_240", "GENERIC_FILM",
                                   "BT_2020",   "XYZ",       "SMPTE_431",
                                   "SMPTE_432", "R_13" };

static char *transfer_characteristics[] = { "R_0",
                                            "BT_709",
                                            "U",
                                            "R_3",
                                            "BT_470_M",
                                            "BT_470_B_G",
                                            "BT_601",
                                            "SMPTE_240",
                                            "LINEAR",
                                            "LOG_100",
                                            "LOG_100_SQRT10",
                                            "IEC_61966",
                                            "BT_1361",
                                            "SRGB",
                                            "BT_2020_10_BIT",
                                            "BT_2020_12_BIT",
                                            "SMPTE_2084",
                                            "SMPTE_428",
                                            "HLG",
                                            "R_19" };

static char *matrix_coefficients[] = {
  "ID",          "BT_709",      "U",          "R_3",
  "FCC",         "BT_470_B_G",  "BT_601",     "SMPTE_240",
  "SMPTE_YCGCO", "BT_2020_NCL", "BT_2020_CL", "SMPTE_2085",
  "CHR_NCL",     "CHR_CL",      "ICTCP",      "R_15"
};

static char *chroma_sample_positions[] = { "U", "V", "C", "R" };

static char *reference_modes[] = { "SINGLE", "COMP", "SELECT", "M" };

static char *tx_modes[] = { "4X4", "L", "S", "M" };

static char *interp_filters[] = { "REG", "SMO", "SHA", "BIL", "SH2" };

static char *refresh_frame_context_modes[] = { "D", "B" };

static char *enc_passes[] = { "O", "F", "S", "T" };

static char *modes[] = { "G", "R", "A" };

static char *disable_trellis_quants[] = { "D", "E", "RD", "YRD" };

static char *cdf_update_modes[] = { "NO", "EVERY", "SELECTIVELY" };

static char *loopfilter_controls[] = { "NONE", "ALL", "REFERENCE",
                                       "SELECTIVELY" };

static char *rc_modes[] = { "VBR", "CBR", "CQ", "Q" };

static char *aq_modes[] = { "NO", "VARIANCE", "COMPLEXITY", "CYCLIC", "M" };

static char *deltaq_modes[] = {
  "NO", "OBJECTIVE", "PERCEPTUAL", "PERCEPTUAL_AI", "USER", "HDR", "M"
};

static char *frame_content_types[] = { "NORMAL", "GRAPHICS_ANIMATION", "T" };

static char *filter_pair[] = { "REG_REG",    "REG_SMOOTH",    "REG_SHARP",
                               "SMOOTH_REG", "SMOOTH_SMOOTH", "SMOOTH_SHARP",
                               "SHARP_REG",  "SHARP_SMOOTH",  "SHARP_SHARP" };

static char *frame_update_types[] = { "KF",       "LF",      "GF",
                                      "ARF",      "OVERLAY", "INTNL_OVERLAY",
                                      "INTNL_ARF" };

static int write_filter_prob_true = 10;

static int write_filter_prob_false = 5;

char *block_size(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return bsizes[i];
}

char *plane_type(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return plane_types[i];
}

char *partition_type(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return partition_types[i];
}

char *prediction_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return prediction_modes[i];
}

char *uv_prediction_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return uv_prediction_modes[i];
}

char *motion_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return motion_modes[i];
}

char *interintra_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return interintra_modes[i];
}

char *frame_type(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return frame_types[i];
}

char *superres_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return superres_modes[i];
}

char *screen_content_tool(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return screen_content_tools[i];
}

char *force_integer_mv(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return force_integer_mvs[i];
}

char *bitstream_profile(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return bitstream_profiles[i];
}

char *color_primary(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  if (i == 22) return "EBU_3213";
  if (i == 23) return "R_23";
  return color_primaries[i];
}

char *transfer_characteristic(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return transfer_characteristics[i];
}

char *matrix_coefficient(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return matrix_coefficients[i];
}

char *chroma_sample_position(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return chroma_sample_positions[i];
}

char *reference_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return reference_modes[i];
}

char *tx_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return tx_modes[i];
}

char *interp_filter(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return interp_filters[i];
}

char *refresh_frame_context_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return refresh_frame_context_modes[i];
}

char *enc_pass(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return enc_passes[i];
}

char *mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return modes[i];
}

char *disable_trellis_quant(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return disable_trellis_quants[i];
}

char *cdf_update_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return cdf_update_modes[i];
}

char *loopfilter_control(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return loopfilter_controls[i];
}

char *rc_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return rc_modes[i];
}

char *aq_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return aq_modes[i];
}

char *deltaq_mode(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return deltaq_modes[i];
}

char *frame_content_type(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return frame_content_types[i];
}

char *frame_update_type(int i) {
  if (i < 0) return "INVALID";
  if (i == 255) return "INVALID";
  return frame_update_types[i];
}

void collect_init(struct features_interp_filters *features) {
  features->bsize = "";
  features->ref_best_rd = -1;
  features->ref_skip_rd = -1;
  features->best_est_rd = -1;
  features->do_tx_search = -1;
  features->rate_mv = -1;
  features->num_planes = -1;
  features->is_comp_pred = -1;
  features->rate2_nocoeff = -1;
  features->interintra_allowed = -1;
  features->txfm_rd_gate_level = -1;
  features->update_type = "";
  features->x__qindex = -1;
  features->x__delta_qindex = -1;
  features->x__rdmult_delta_qindex = -1;
  features->x__rdmult_cur_qindex = -1;
  features->x__rdmult = -1;
  features->x__intra_sb_rdmult_modifier = -1;
  features->x__rb = -1;
  features->x__mb_energy = -1;
  features->x__sb_energy_level = -1;
  features->x__errorperbit = -1;
  features->x__sadperbit = -1;
  features->x__seg_skip_block = -1;
  features->x__actual_num_seg1_blocks = -1;
  features->x__actual_num_seg2_blocks = -1;
  features->x__cnt_zeromv = -1;
  features->x__force_zeromv_skip_for_sb = -1;
  features->x__force_zeromv_skip_for_blk = -1;
  features->x__nonrd_prune_ref_frame_search = -1;
  features->x__must_find_valid_partition = -1;
  features->x__skip_mode = -1;
  features->x__winner_mode_count = -1;
  features->x__recalc_luma_mc_data = -1;
  features->x__reuse_inter_pred = -1;
  features->x__source_variance = -1;
  features->x__block_is_zero_sad = -1;
  features->x__sb_me_partition = -1;
  features->x__sb_me_block = -1;
  features->x__sb_force_fixed_part = -1;
  features->x__try_merge_partition = -1;
  features->x__palette_pixels = -1;
  features->x__plane__0__src_diff = -1;
  features->x__plane__0__dqcoeff = -1;
  features->x__plane__0__qcoeff = -1;
  features->x__plane__0__coeff = -1;
  features->x__plane__0__eobs = -1;
  features->x__plane__0__quant_fp_QTX = -1;
  features->x__plane__0__round_fp_QTX = -1;
  features->x__plane__0__quant_QTX = -1;
  features->x__plane__0__round_QTX = -1;
  features->x__plane__0__quant_shift_QTX = -1;
  features->x__plane__0__zbin_QTX = -1;
  features->x__plane__0__dequant_QTX = -1;
  features->xd__mi_row = -1;
  features->xd__mi_col = -1;
  features->xd__mi_stride = -1;
  features->xd__is_chroma_ref = -1;
  features->xd__up_available = -1;
  features->xd__left_available = -1;
  features->xd__chroma_up_available = -1;
  features->xd__chroma_left_available = -1;
  features->xd__tx_type_map_stride = -1;
  features->xd__mb_to_left_edge = -1;
  features->xd__mb_to_right_edge = -1;
  features->xd__mb_to_top_edge = -1;
  features->xd__mb_to_bottom_edge = -1;
  features->xd__width = -1;
  features->xd__height = -1;
  features->xd__is_last_vertical_rect = -1;
  features->xd__is_first_horizontal_rect = -1;
  features->xd__bd = -1;
  features->xd__current_base_qindex = -1;
  features->xd__cur_frame_force_integer_mv = -1;
  features->xd__delta_lf_from_base = -1;
  features->xd__block_ref_scale_factors__0__x_scale_fp = -1;
  features->xd__block_ref_scale_factors__0__y_scale_fp = -1;
  features->xd__block_ref_scale_factors__0__x_step_q4 = -1;
  features->xd__block_ref_scale_factors__0__y_step_q4 = -1;
  features->xd__block_ref_scale_factors__1__x_scale_fp = -1;
  features->xd__block_ref_scale_factors__1__y_scale_fp = -1;
  features->xd__block_ref_scale_factors__1__x_step_q4 = -1;
  features->xd__block_ref_scale_factors__1__y_step_q4 = -1;
  features->xd__plane__0__plane_type = "";
  features->xd__plane__0__subsampling_x = -1;
  features->xd__plane__0__width = -1;
  features->xd__plane__0__height = -1;
  features->xd__plane__1__plane_type = "";
  features->xd__plane__1__subsampling_x = -1;
  features->xd__plane__1__width = -1;
  features->xd__plane__1__height = -1;
  features->xd__plane__2__plane_type = "";
  features->xd__plane__2__subsampling_x = -1;
  features->xd__plane__2__width = -1;
  features->xd__plane__2__height = -1;
  features->xd__tile__mi_row_start = -1;
  features->xd__tile__mi_row_end = -1;
  features->xd__tile__mi_col_start = -1;
  features->xd__tile__mi_col_end = -1;
  features->xd__tile__tile_row = -1;
  features->xd__tile__tile_col = -1;
  features->mbmi__bsize = "";
  features->mbmi__partition = "";
  features->mbmi__mode = "";
  features->mbmi__uv_mode = "";
  features->mbmi__current_qindex = -1;
  features->mbmi__motion_mode = "";
  features->mbmi__num_proj_ref = -1;
  features->mbmi__overlappable_neighbors = -1;
  features->mbmi__interintra_mode = "";
  features->mbmi__interintra_wedge_index = -1;
  features->mbmi__cfl_alpha_signs = -1;
  features->mbmi__cfl_alpha_idx = -1;
  features->mbmi__skip_txfm = -1;
  features->mbmi__tx_size = -1;
  features->mbmi__ref_mv_idx = -1;
  features->mbmi__skip_mode = -1;
  features->mbmi__use_intrabc = -1;
  features->mbmi__comp_group_idx = -1;
  features->mbmi__compound_idx = -1;
  features->mbmi__use_wedge_interintra = -1;
  features->mbmi__cdef_strength = -1;
  features->mbmi__interp_filters__as_filters__y_filter = "";
  features->mbmi__interp_filters__as_filters__x_filter = "";
  features->mbmi__mv__0__as_mv__row = -1;
  features->mbmi__mv__1__as_mv__col = -1;
  features->xd__left_mbmi__bsize = "";
  features->xd__left_mbmi__partition = "";
  features->xd__left_mbmi__mode = "";
  features->xd__left_mbmi__uv_mode = "";
  features->xd__left_mbmi__current_qindex = -1;
  features->xd__left_mbmi__motion_mode = "";
  features->xd__left_mbmi__num_proj_ref = -1;
  features->xd__left_mbmi__overlappable_neighbors = -1;
  features->xd__left_mbmi__interintra_mode = "";
  features->xd__left_mbmi__interintra_wedge_index = -1;
  features->xd__left_mbmi__cfl_alpha_signs = -1;
  features->xd__left_mbmi__cfl_alpha_idx = -1;
  features->xd__left_mbmi__skip_txfm = -1;
  features->xd__left_mbmi__tx_size = -1;
  features->xd__left_mbmi__ref_mv_idx = -1;
  features->xd__left_mbmi__skip_mode = -1;
  features->xd__left_mbmi__use_intrabc = -1;
  features->xd__left_mbmi__comp_group_idx = -1;
  features->xd__left_mbmi__compound_idx = -1;
  features->xd__left_mbmi__use_wedge_interintra = -1;
  features->xd__left_mbmi__cdef_strength = -1;
  features->xd__left_mbmi__interp_filters__as_filters__y_filter = "";
  features->xd__left_mbmi__interp_filters__as_filters__x_filter = "";
  features->xd__left_mbmi__mv__0__as_mv__row = -1;
  features->xd__left_mbmi__mv__1__as_mv__col = -1;
  features->xd__above_mbmi__bsize = "";
  features->xd__above_mbmi__partition = "";
  features->xd__above_mbmi__mode = "";
  features->xd__above_mbmi__uv_mode = "";
  features->xd__above_mbmi__current_qindex = -1;
  features->xd__above_mbmi__motion_mode = "";
  features->xd__above_mbmi__num_proj_ref = -1;
  features->xd__above_mbmi__overlappable_neighbors = -1;
  features->xd__above_mbmi__interintra_mode = "";
  features->xd__above_mbmi__interintra_wedge_index = -1;
  features->xd__above_mbmi__cfl_alpha_signs = -1;
  features->xd__above_mbmi__cfl_alpha_idx = -1;
  features->xd__above_mbmi__skip_txfm = -1;
  features->xd__above_mbmi__tx_size = -1;
  features->xd__above_mbmi__ref_mv_idx = -1;
  features->xd__above_mbmi__skip_mode = -1;
  features->xd__above_mbmi__use_intrabc = -1;
  features->xd__above_mbmi__comp_group_idx = -1;
  features->xd__above_mbmi__compound_idx = -1;
  features->xd__above_mbmi__use_wedge_interintra = -1;
  features->xd__above_mbmi__cdef_strength = -1;
  features->xd__above_mbmi__interp_filters__as_filters__y_filter = "";
  features->xd__above_mbmi__interp_filters__as_filters__x_filter = "";
  features->xd__above_mbmi__mv__0__as_mv__row = -1;
  features->xd__above_mbmi__mv__1__as_mv__col = -1;
  features->rd_stats__rate = -1;
  features->rd_stats__zero_rate = -1;
  features->rd_stats__dist = -1;
  features->rd_stats__rdcost = -1;
  features->rd_stats__sse = -1;
  features->rd_stats__skip_txfm = -1;
  features->rd_stats_y__rate = -1;
  features->rd_stats_y__zero_rate = -1;
  features->rd_stats_y__dist = -1;
  features->rd_stats_y__rdcost = -1;
  features->rd_stats_y__sse = -1;
  features->rd_stats_y__skip_txfm = -1;
  features->rd_stats_uv__rate = -1;
  features->rd_stats_uv__zero_rate = -1;
  features->rd_stats_uv__dist = -1;
  features->rd_stats_uv__rdcost = -1;
  features->rd_stats_uv__sse = -1;
  features->rd_stats_uv__skip_txfm = -1;
  features->cpi__skip_tpl_setup_stats = -1;
  features->cpi__tpl_rdmult_scaling_factors = -1;
  features->cpi__rt_reduce_num_ref_buffers = -1;
  features->cpi__framerate = -1;
  features->cpi__ref_frame_flags = -1;
  features->cpi__speed = -1;
  features->cpi__all_one_sided_refs = -1;
  features->cpi__gf_frame_index = -1;
  features->cpi__droppable = -1;
  features->cpi__data_alloc_width = -1;
  features->cpi__data_alloc_height = -1;
  features->cpi__initial_mbs = -1;
  features->cpi__frame_size_related_setup_done = -1;
  features->cpi__last_coded_width = -1;
  features->cpi__last_coded_height = -1;
  features->cpi__num_frame_recode = -1;
  features->cpi__intrabc_used = -1;
  features->cpi__prune_ref_frame_mask = -1;
  features->cpi__use_screen_content_tools = -1;
  features->cpi__is_screen_content_type = -1;
  features->cpi__frame_header_count = -1;
  features->cpi__deltaq_used = -1;
  features->cpi__last_frame_type = "";
  features->cpi__num_tg = -1;
  features->cpi__superres_mode = "";
  features->cpi__fp_block_size = "";
  features->cpi__sb_counter = -1;
  features->cpi__ref_refresh_index = -1;
  features->cpi__refresh_idx_available = -1;
  features->cpi__ref_idx_to_skip = -1;
  features->cpi__do_frame_data_update = -1;
  features->cpi__ext_rate_scale = -1;
  features->cpi__weber_bsize = "";
  features->cpi__norm_wiener_variance = -1;
  features->cpi__is_dropped_frame = -1;
  features->cpi__rec_sse = -1;
  features->cpi__frames_since_last_update = -1;
  features->cpi__palette_pixel_num = -1;
  features->cpi__scaled_last_source_available = -1;
  features->cpi__ppi__ts_start_last_show_frame = -1;
  features->cpi__ppi__ts_end_last_show_frame = -1;
  features->cpi__ppi__num_fp_contexts = -1;
  features->cpi__ppi__filter_level__0 = -1;
  features->cpi__ppi__filter_level__1 = -1;
  features->cpi__ppi__filter_level_u = -1;
  features->cpi__ppi__filter_level_v = -1;
  features->cpi__ppi__seq_params_locked = -1;
  features->cpi__ppi__internal_altref_allowed = -1;
  features->cpi__ppi__show_existing_alt_ref = -1;
  features->cpi__ppi__lap_enabled = -1;
  features->cpi__ppi__b_calculate_psnr = -1;
  features->cpi__ppi__frames_left = -1;
  features->cpi__ppi__use_svc = -1;
  features->cpi__ppi__buffer_removal_time_present = -1;
  features->cpi__ppi__number_temporal_layers = -1;
  features->cpi__ppi__number_spatial_layers = -1;
  features->cpi__ppi__tpl_sb_rdmult_scaling_factors = -1;
  features->cpi__ppi__gf_group__max_layer_depth = -1;
  features->cpi__ppi__gf_group__max_layer_depth_allowed = -1;
  features->cpi__ppi__gf_state__arf_gf_boost_lst = -1;
  features->cpi__ppi__twopass__section_intra_rating = -1;
  features->cpi__ppi__twopass__first_pass_done = -1;
  features->cpi__ppi__twopass__bits_left = -1;
  features->cpi__ppi__twopass__modified_error_min = -1;
  features->cpi__ppi__twopass__modified_error_max = -1;
  features->cpi__ppi__twopass__modified_error_left = -1;
  features->cpi__ppi__twopass__kf_group_bits = -1;
  features->cpi__ppi__twopass__kf_group_error_left = -1;
  features->cpi__ppi__twopass__bpm_factor = -1;
  features->cpi__ppi__twopass__rolling_arf_group_target_bits = -1;
  features->cpi__ppi__twopass__rolling_arf_group_actual_bits = -1;
  features->cpi__ppi__twopass__sr_update_lag = -1;
  features->cpi__ppi__twopass__kf_zeromotion_pct = -1;
  features->cpi__ppi__twopass__last_kfgroup_zeromotion_pct = -1;
  features->cpi__ppi__twopass__extend_minq = -1;
  features->cpi__ppi__twopass__extend_maxq = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__frame = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__weight = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__intra_error = -1;
  features
      ->cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__coded_error = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_cols =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVr = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVc = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVrv = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVcv = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__duration = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__count = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__raw_error_stdev =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__is_flash = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__noise_var = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__cor_coeff = -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error =
      -1;
  features->cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error =
      -1;
  features->cpi__ppi__p_rc__gf_group_bits = -1;
  features->cpi__ppi__p_rc__kf_boost = -1;
  features->cpi__ppi__p_rc__gfu_boost = -1;
  features->cpi__ppi__p_rc__cur_gf_index = -1;
  features->cpi__ppi__p_rc__num_regions = -1;
  features->cpi__ppi__p_rc__regions_offset = -1;
  features->cpi__ppi__p_rc__frames_till_regions_update = -1;
  features->cpi__ppi__p_rc__baseline_gf_interval = -1;
  features->cpi__ppi__p_rc__constrained_gf_group = -1;
  features->cpi__ppi__p_rc__this_key_frame_forced = -1;
  features->cpi__ppi__p_rc__next_key_frame_forced = -1;
  features->cpi__ppi__p_rc__starting_buffer_level = -1;
  features->cpi__ppi__p_rc__optimal_buffer_level = -1;
  features->cpi__ppi__p_rc__maximum_buffer_size = -1;
  features->cpi__ppi__p_rc__arf_q = -1;
  features->cpi__ppi__p_rc__arf_boost_factor = -1;
  features->cpi__ppi__p_rc__base_layer_qp = -1;
  features->cpi__ppi__p_rc__num_stats_used_for_kf_boost = -1;
  features->cpi__ppi__p_rc__num_stats_used_for_gfu_boost = -1;
  features->cpi__ppi__p_rc__num_stats_required_for_gfu_boost = -1;
  features->cpi__ppi__p_rc__enable_scenecut_detection = -1;
  features->cpi__ppi__p_rc__use_arf_in_this_kf_group = -1;
  features->cpi__ppi__p_rc__ni_frames = -1;
  features->cpi__ppi__p_rc__tot_q = -1;
  features->cpi__ppi__p_rc__last_kf_qindex = -1;
  features->cpi__ppi__p_rc__last_boosted_qindex = -1;
  features->cpi__ppi__p_rc__avg_q = -1;
  features->cpi__ppi__p_rc__total_actual_bits = -1;
  features->cpi__ppi__p_rc__total_target_bits = -1;
  features->cpi__ppi__p_rc__buffer_level = -1;
  features->cpi__ppi__p_rc__rate_error_estimate = -1;
  features->cpi__ppi__p_rc__vbr_bits_off_target = -1;
  features->cpi__ppi__p_rc__vbr_bits_off_target_fast = -1;
  features->cpi__ppi__p_rc__bits_off_target = -1;
  features->cpi__ppi__p_rc__rolling_target_bits = -1;
  features->cpi__ppi__p_rc__rolling_actual_bits = -1;
  features->cpi__ppi__tf_info__is_temporal_filter_on = -1;
  features->cpi__ppi__tf_info__frame_diff__0__sum = -1;
  features->cpi__ppi__tf_info__frame_diff__0__sse = -1;
  features->cpi__ppi__tf_info__frame_diff__1__sum = -1;
  features->cpi__ppi__tf_info__frame_diff__1__sse = -1;
  features->cpi__ppi__tf_info__tf_buf_gf_index__0 = -1;
  features->cpi__ppi__tf_info__tf_buf_gf_index__1 = -1;
  features->cpi__ppi__tf_info__tf_buf_display_index_offset__0 = -1;
  features->cpi__ppi__tf_info__tf_buf_display_index_offset__1 = -1;
  features->cpi__ppi__tf_info__tf_buf_valid__0 = -1;
  features->cpi__ppi__tf_info__tf_buf_valid__1 = -1;
  features->cpi__ppi__seq_params__num_bits_width = -1;
  features->cpi__ppi__seq_params__num_bits_height = -1;
  features->cpi__ppi__seq_params__max_frame_width = -1;
  features->cpi__ppi__seq_params__max_frame_height = -1;
  features->cpi__ppi__seq_params__frame_id_numbers_present_flag = -1;
  features->cpi__ppi__seq_params__frame_id_length = -1;
  features->cpi__ppi__seq_params__delta_frame_id_length = -1;
  features->cpi__ppi__seq_params__sb_size = "";
  features->cpi__ppi__seq_params__mib_size = -1;
  features->cpi__ppi__seq_params__mib_size_log2 = -1;
  features->cpi__ppi__seq_params__force_screen_content_tools = "";
  features->cpi__ppi__seq_params__still_picture = -1;
  features->cpi__ppi__seq_params__reduced_still_picture_hdr = -1;
  features->cpi__ppi__seq_params__force_integer_mv = "";
  features->cpi__ppi__seq_params__enable_filter_intra = -1;
  features->cpi__ppi__seq_params__enable_intra_edge_filter = -1;
  features->cpi__ppi__seq_params__enable_interintra_compound = -1;
  features->cpi__ppi__seq_params__enable_masked_compound = -1;
  features->cpi__ppi__seq_params__enable_dual_filter = -1;
  features->cpi__ppi__seq_params__enable_warped_motion = -1;
  features->cpi__ppi__seq_params__enable_superres = -1;
  features->cpi__ppi__seq_params__enable_cdef = -1;
  features->cpi__ppi__seq_params__enable_restoration = -1;
  features->cpi__ppi__seq_params__profile = "";
  features->cpi__ppi__seq_params__bit_depth = -1;
  features->cpi__ppi__seq_params__use_highbitdepth = -1;
  features->cpi__ppi__seq_params__monochrome = -1;
  features->cpi__ppi__seq_params__color_primaries = "";
  features->cpi__ppi__seq_params__transfer_characteristics = "";
  features->cpi__ppi__seq_params__matrix_coefficients = "";
  features->cpi__ppi__seq_params__color_range = -1;
  features->cpi__ppi__seq_params__subsampling_x = -1;
  features->cpi__ppi__seq_params__subsampling_y = -1;
  features->cpi__ppi__seq_params__chroma_sample_position = "";
  features->cpi__ppi__seq_params__separate_uv_delta_q = -1;
  features->cpi__ppi__seq_params__film_grain_params_present = -1;
  features->cpi__ppi__seq_params__operating_points_cnt_minus_1 = -1;
  features->cpi__ppi__seq_params__has_nonzero_operating_point_idc = -1;
  features->cpi__ppi__tpl_data__ready = -1;
  features->cpi__ppi__tpl_data__tpl_stats_block_mis_log2 = -1;
  features->cpi__ppi__tpl_data__tpl_bsize_1d = -1;
  features->cpi__ppi__tpl_data__frame_idx = -1;
  features->cpi__ppi__tpl_data__border_in_pixels = -1;
  features->cpi__ppi__tpl_data__r0_adjust_factor = -1;
  features->cpi__ppi__tpl_data__tpl_frame__is_valid = -1;
  features->cpi__ppi__tpl_data__tpl_frame__stride = -1;
  features->cpi__ppi__tpl_data__tpl_frame__width = -1;
  features->cpi__ppi__tpl_data__tpl_frame__height = -1;
  features->cpi__ppi__tpl_data__tpl_frame__mi_rows = -1;
  features->cpi__ppi__tpl_data__tpl_frame__mi_cols = -1;
  features->cpi__ppi__tpl_data__tpl_frame__base_rdmult = -1;
  features->cpi__ppi__tpl_data__tpl_frame__frame_display_index = -1;
  features->cpi__ppi__tpl_data__tpl_frame__use_pred_sad = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_sse = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_dist = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate = -1;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_rate = -1;
  features->cpi__ppi__tpl_data__sf__x_scale_fp = -1;
  features->cpi__ppi__tpl_data__sf__y_scale_fp = -1;
  features->cpi__ppi__tpl_data__sf__x_step_q4 = -1;
  features->cpi__ppi__tpl_data__sf__y_step_q4 = -1;
  features->cpi__common__width = -1;
  features->cpi__common__height = -1;
  features->cpi__common__render_width = -1;
  features->cpi__common__render_height = -1;
  features->cpi__common__superres_upscaled_width = -1;
  features->cpi__common__superres_upscaled_height = -1;
  features->cpi__common__superres_scale_denominator = -1;
  features->cpi__common__frame_presentation_time = -1;
  features->cpi__common__show_frame = -1;
  features->cpi__common__showable_frame = -1;
  features->cpi__common__show_existing_frame = -1;
  features->cpi__common__current_frame__frame_type = "";
  features->cpi__common__current_frame__reference_mode = "";
  features->cpi__common__current_frame__order_hint = -1;
  features->cpi__common__current_frame__display_order_hint = -1;
  features->cpi__common__current_frame__pyramid_level = -1;
  features->cpi__common__current_frame__frame_number = -1;
  features->cpi__common__current_frame__refresh_frame_flags = -1;
  features->cpi__common__current_frame__frame_refs_short_signaling = -1;
  features->cpi__common__sf_identity__x_scale_fp = -1;
  features->cpi__common__sf_identity__y_scale_fp = -1;
  features->cpi__common__sf_identity__x_step_q4 = -1;
  features->cpi__common__sf_identity__y_step_q4 = -1;
  features->cpi__common__features__disable_cdf_update = -1;
  features->cpi__common__features__allow_high_precision_mv = -1;
  features->cpi__common__features__cur_frame_force_integer_mv = -1;
  features->cpi__common__features__allow_screen_content_tools = -1;
  features->cpi__common__features__allow_intrabc = -1;
  features->cpi__common__features__allow_warped_motion = -1;
  features->cpi__common__features__allow_ref_frame_mvs = -1;
  features->cpi__common__features__coded_lossless = -1;
  features->cpi__common__features__all_lossless = -1;
  features->cpi__common__features__reduced_tx_set_used = -1;
  features->cpi__common__features__error_resilient_mode = -1;
  features->cpi__common__features__switchable_motion_mode = -1;
  features->cpi__common__features__tx_mode = "";
  features->cpi__common__features__interp_filter = "";
  features->cpi__common__features__primary_ref_frame = -1;
  features->cpi__common__features__byte_alignment = -1;
  features->cpi__common__features__refresh_frame_context = "";
  features->cpi__common__mi_params__mb_rows = -1;
  features->cpi__common__mi_params__mb_cols = -1;
  features->cpi__common__mi_params__MBs = -1;
  features->cpi__common__mi_params__mi_rows = -1;
  features->cpi__common__mi_params__mi_cols = -1;
  features->cpi__common__mi_params__mi_stride = -1;
  features->cpi__oxcf__noise_level = -1;
  features->cpi__oxcf__noise_block_size = -1;
  features->cpi__oxcf__enable_dnl_denoising = -1;
  features->cpi__oxcf__tier_mask = -1;
  features->cpi__oxcf__border_in_pixels = -1;
  features->cpi__oxcf__max_threads = -1;
  features->cpi__oxcf__speed = -1;
  features->cpi__oxcf__profile = "";
  features->cpi__oxcf__pass = "";
  features->cpi__oxcf__passes = -1;
  features->cpi__oxcf__mode = "";
  features->cpi__oxcf__use_highbitdepth = -1;
  features->cpi__oxcf__save_as_annexb = -1;
  features->cpi__oxcf__input_cfg__init_framerate = -1;
  features->cpi__oxcf__input_cfg__input_bit_depth = -1;
  features->cpi__oxcf__input_cfg__limit = -1;
  features->cpi__oxcf__input_cfg__chroma_subsampling_x = -1;
  features->cpi__oxcf__input_cfg__chroma_subsampling_y = -1;
  features->cpi__oxcf__algo_cfg__sharpness = -1;
  features->cpi__oxcf__algo_cfg__disable_trellis_quant = "";
  features->cpi__oxcf__algo_cfg__arnr_max_frames = -1;
  features->cpi__oxcf__algo_cfg__arnr_strength = -1;
  features->cpi__oxcf__algo_cfg__cdf_update_mode = "";
  features->cpi__oxcf__algo_cfg__enable_tpl_model = -1;
  features->cpi__oxcf__algo_cfg__enable_overlay = -1;
  features->cpi__oxcf__algo_cfg__loopfilter_control = "";
  features->cpi__oxcf__algo_cfg__skip_postproc_filtering = -1;
  features->cpi__oxcf__rc_cfg__starting_buffer_level_ms = -1;
  features->cpi__oxcf__rc_cfg__optimal_buffer_level_ms = -1;
  features->cpi__oxcf__rc_cfg__maximum_buffer_size_ms = -1;
  features->cpi__oxcf__rc_cfg__target_bandwidth = -1;
  features->cpi__oxcf__rc_cfg__vbr_corpus_complexity_lap = -1;
  features->cpi__oxcf__rc_cfg__max_intra_bitrate_pct = -1;
  features->cpi__oxcf__rc_cfg__max_inter_bitrate_pct = -1;
  features->cpi__oxcf__rc_cfg__gf_cbr_boost_pct = -1;
  features->cpi__oxcf__rc_cfg__min_cr = -1;
  features->cpi__oxcf__rc_cfg__drop_frames_water_mark = -1;
  features->cpi__oxcf__rc_cfg__under_shoot_pct = -1;
  features->cpi__oxcf__rc_cfg__over_shoot_pct = -1;
  features->cpi__oxcf__rc_cfg__worst_allowed_q = -1;
  features->cpi__oxcf__rc_cfg__best_allowed_q = -1;
  features->cpi__oxcf__rc_cfg__cq_level = -1;
  features->cpi__oxcf__rc_cfg__mode = "";
  features->cpi__oxcf__rc_cfg__vbrbias = -1;
  features->cpi__oxcf__rc_cfg__vbrmin_section = -1;
  features->cpi__oxcf__rc_cfg__vbrmax_section = -1;
  features->cpi__oxcf__q_cfg__use_fixed_qp_offsets = -1;
  features->cpi__oxcf__q_cfg__qm_minlevel = -1;
  features->cpi__oxcf__q_cfg__qm_maxlevel = -1;
  features->cpi__oxcf__q_cfg__quant_b_adapt = -1;
  features->cpi__oxcf__q_cfg__aq_mode = "";
  features->cpi__oxcf__q_cfg__deltaq_mode = "";
  features->cpi__oxcf__q_cfg__deltaq_strength = -1;
  features->cpi__oxcf__q_cfg__enable_chroma_deltaq = -1;
  features->cpi__oxcf__q_cfg__enable_hdr_deltaq = -1;
  features->cpi__oxcf__q_cfg__using_qm = -1;
  features->cpi__tf_ctx__num_frames = -1;
  features->cpi__tf_ctx__noise_levels__0 = -1;
  features->cpi__tf_ctx__noise_levels__1 = -1;
  features->cpi__tf_ctx__noise_levels__2 = -1;
  features->cpi__tf_ctx__num_pels = -1;
  features->cpi__tf_ctx__mb_rows = -1;
  features->cpi__tf_ctx__mb_cols = -1;
  features->cpi__tf_ctx__q_factor = -1;
  features->cpi__rd__rdmult = -1;
  features->cpi__rd__r0 = -1;
  features->cpi__mv_search_params__max_mv_magnitude = -1;
  features->cpi__mv_search_params__mv_step_param = -1;
  features->cpi__frame_info__frame_width = -1;
  features->cpi__frame_info__frame_height = -1;
  features->cpi__frame_info__mi_rows = -1;
  features->cpi__frame_info__mi_cols = -1;
  features->cpi__frame_info__mb_rows = -1;
  features->cpi__frame_info__mb_cols = -1;
  features->cpi__frame_info__num_mbs = -1;
  features->cpi__frame_info__bit_depth = -1;
  features->cpi__frame_info__subsampling_x = -1;
  features->cpi__frame_info__subsampling_y = -1;
  features->cpi__interp_search_flags__default_interp_skip_flags = -1;
  features->cpi__interp_search_flags__interp_filter_search_mask = -1;
  features->cpi__twopass_frame__mb_av_energy = -1;
  features->cpi__twopass_frame__fr_content_type = "";
  features->cpi__twopass_frame__frame_avg_haar_energy = -1;
  features->cpi__twopass_frame__stats_in__frame = -1;
  features->cpi__twopass_frame__stats_in__weight = -1;
  features->cpi__twopass_frame__stats_in__intra_error = -1;
  features->cpi__twopass_frame__stats_in__frame_avg_wavelet_energy = -1;
  features->cpi__twopass_frame__stats_in__coded_error = -1;
  features->cpi__twopass_frame__stats_in__sr_coded_error = -1;
  features->cpi__twopass_frame__stats_in__pcnt_inter = -1;
  features->cpi__twopass_frame__stats_in__pcnt_motion = -1;
  features->cpi__twopass_frame__stats_in__pcnt_second_ref = -1;
  features->cpi__twopass_frame__stats_in__pcnt_neutral = -1;
  features->cpi__twopass_frame__stats_in__intra_skip_pct = -1;
  features->cpi__twopass_frame__stats_in__inactive_zone_rows = -1;
  features->cpi__twopass_frame__stats_in__inactive_zone_cols = -1;
  features->cpi__twopass_frame__stats_in__MVr = -1;
  features->cpi__twopass_frame__stats_in__mvr_abs = -1;
  features->cpi__twopass_frame__stats_in__MVc = -1;
  features->cpi__twopass_frame__stats_in__mvc_abs = -1;
  features->cpi__twopass_frame__stats_in__MVrv = -1;
  features->cpi__twopass_frame__stats_in__MVcv = -1;
  features->cpi__twopass_frame__stats_in__mv_in_out_count = -1;
  features->cpi__twopass_frame__stats_in__new_mv_count = -1;
  features->cpi__twopass_frame__stats_in__duration = -1;
  features->cpi__twopass_frame__stats_in__count = -1;
  features->cpi__twopass_frame__stats_in__raw_error_stdev = -1;
  features->cpi__twopass_frame__stats_in__is_flash = -1;
  features->cpi__twopass_frame__stats_in__noise_var = -1;
  features->cpi__twopass_frame__stats_in__cor_coeff = -1;
  features->cpi__twopass_frame__stats_in__log_intra_error = -1;
  features->cpi__twopass_frame__stats_in__log_coded_error = -1;
  features->tile_data__abs_sum_level = -1;
  features->tile_data__allow_update_cdf = -1;
  features->tile_data__tile_info__mi_row_start = -1;
  features->tile_data__tile_info__mi_row_end = -1;
  features->tile_data__tile_info__mi_col_start = -1;
  features->tile_data__tile_info__mi_col_end = -1;
  features->tile_data__tile_info__tile_row = -1;
  features->tile_data__tile_info__tile_col = -1;
  features->tile_data__firstpass_top_mv__row = -1;
  features->tile_data__firstpass_top_mv__col = -1;
  features->args__ref_frame_cost = -1;
  features->args__single_comp_cost = -1;
  features->args__skip_ifs = -1;
  features->args__start_mv_cnt = -1;
  features->args__interp_filter_stats_idx = -1;
  features->args__wedge_index = -1;
  features->args__wedge_sign = -1;
  features->args__diffwtd_index = -1;
  features->args__best_pred_sse = -1;
  features->discard = -1;
}

static unsigned int lcg_next(unsigned int *state) {
  *state = (unsigned int)(*state * 1103515245ULL + 12345);
  return *state;
}

static unsigned int lcg_rand16(unsigned int *state) {
  return (lcg_next(state) / 65536) % 32768;
}

/* clang-format off */
void collect_finalize(struct features_interp_filters *features) {
  if (features->discard == -1) return;

  if (strcmp("WARP", features->mbmi__motion_mode) != 0) return;

  if (features->discard == 1) {
    if (lcg_rand16(&seed) % write_filter_prob_true > 0) return;
  } else {
    if (lcg_rand16(&seed) % write_filter_prob_false > 0) return;
  }

  FILE *file = fopen(filename, "a");

fprintf(file, "%s,", features->bsize);
fprintf(file, "%lld,", features->ref_best_rd);
//fprintf(file, "%lld,", features->ref_skip_rd);
fprintf(file, "%lld,", features->best_est_rd);
fprintf(file, "%d,", features->do_tx_search);
fprintf(file, "%d,", features->rate_mv);
//fprintf(file, "%d,", features->num_planes);
fprintf(file, "%d,", features->is_comp_pred);
fprintf(file, "%d,", features->rate2_nocoeff);
fprintf(file, "%d,", features->interintra_allowed);
//fprintf(file, "%d,", features->txfm_rd_gate_level);
fprintf(file, "%s,", features->update_type);
fprintf(file, "%d,", features->x__qindex);
fprintf(file, "%d,", features->x__delta_qindex);
fprintf(file, "%d,", features->x__rdmult_delta_qindex);
fprintf(file, "%d,", features->x__rdmult_cur_qindex);
fprintf(file, "%d,", features->x__rdmult);
//fprintf(file, "%d,", features->x__intra_sb_rdmult_modifier);
fprintf(file, "%f,", features->x__rb);
//fprintf(file, "%d,", features->x__mb_energy);
//fprintf(file, "%d,", features->x__sb_energy_level);
fprintf(file, "%d,", features->x__errorperbit);
fprintf(file, "%d,", features->x__sadperbit);
//fprintf(file, "%d,", features->x__seg_skip_block);
//fprintf(file, "%d,", features->x__actual_num_seg1_blocks);
//fprintf(file, "%d,", features->x__actual_num_seg2_blocks);
fprintf(file, "%d,", features->x__cnt_zeromv);
//fprintf(file, "%d,", features->x__force_zeromv_skip_for_sb);
//fprintf(file, "%d,", features->x__force_zeromv_skip_for_blk);
//fprintf(file, "%d,", features->x__nonrd_prune_ref_frame_search);
//fprintf(file, "%d,", features->x__must_find_valid_partition);
//fprintf(file, "%d,", features->x__skip_mode);
//fprintf(file, "%d,", features->x__winner_mode_count);
fprintf(file, "%d,", features->x__recalc_luma_mc_data);
//fprintf(file, "%d,", features->x__reuse_inter_pred);
fprintf(file, "%d,", features->x__source_variance);
//fprintf(file, "%d,", features->x__block_is_zero_sad);
//fprintf(file, "%d,", features->x__sb_me_partition);
//fprintf(file, "%d,", features->x__sb_me_block);
//fprintf(file, "%d,", features->x__sb_force_fixed_part);
//fprintf(file, "%d,", features->x__try_merge_partition);
fprintf(file, "%d,", features->x__palette_pixels);
fprintf(file, "%d,", features->x__plane__0__src_diff);
fprintf(file, "%d,", features->x__plane__0__dqcoeff);
fprintf(file, "%d,", features->x__plane__0__qcoeff);
fprintf(file, "%d,", features->x__plane__0__coeff);
fprintf(file, "%d,", features->x__plane__0__eobs);
fprintf(file, "%d,", features->x__plane__0__quant_fp_QTX);
fprintf(file, "%d,", features->x__plane__0__round_fp_QTX);
fprintf(file, "%d,", features->x__plane__0__quant_QTX);
fprintf(file, "%d,", features->x__plane__0__round_QTX);
fprintf(file, "%d,", features->x__plane__0__quant_shift_QTX);
fprintf(file, "%d,", features->x__plane__0__zbin_QTX);
fprintf(file, "%d,", features->x__plane__0__dequant_QTX);
fprintf(file, "%d,", features->xd__mi_row);
fprintf(file, "%d,", features->xd__mi_col);
fprintf(file, "%d,", features->xd__mi_stride);
fprintf(file, "%d,", features->xd__is_chroma_ref);
fprintf(file, "%d,", features->xd__up_available);
fprintf(file, "%d,", features->xd__left_available);
fprintf(file, "%d,", features->xd__chroma_up_available);
fprintf(file, "%d,", features->xd__chroma_left_available);
fprintf(file, "%d,", features->xd__tx_type_map_stride);
fprintf(file, "%d,", features->xd__mb_to_left_edge);
fprintf(file, "%d,", features->xd__mb_to_right_edge);
fprintf(file, "%d,", features->xd__mb_to_top_edge);
fprintf(file, "%d,", features->xd__mb_to_bottom_edge);
fprintf(file, "%d,", features->xd__width);
fprintf(file, "%d,", features->xd__height);
fprintf(file, "%d,", features->xd__is_last_vertical_rect);
fprintf(file, "%d,", features->xd__is_first_horizontal_rect);
fprintf(file, "%d,", features->xd__bd);
fprintf(file, "%d,", features->xd__current_base_qindex);
//fprintf(file, "%d,", features->xd__cur_frame_force_integer_mv);
//fprintf(file, "%d,", features->xd__delta_lf_from_base);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__0__x_scale_fp);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__0__y_scale_fp);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__0__x_step_q4);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__0__y_step_q4);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__1__x_scale_fp);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__1__y_scale_fp);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__1__x_step_q4);
//fprintf(file, "%d,", features->xd__block_ref_scale_factors__1__y_step_q4);
//fprintf(file, "%s,", features->xd__plane__0__plane_type);
//fprintf(file, "%d,", features->xd__plane__0__subsampling_x);
fprintf(file, "%d,", features->xd__plane__0__width);
fprintf(file, "%d,", features->xd__plane__0__height);
//fprintf(file, "%s,", features->xd__plane__1__plane_type);
//fprintf(file, "%d,", features->xd__plane__1__subsampling_x);
fprintf(file, "%d,", features->xd__plane__1__width);
fprintf(file, "%d,", features->xd__plane__1__height);
//fprintf(file, "%s,", features->xd__plane__2__plane_type);
//fprintf(file, "%d,", features->xd__plane__2__subsampling_x);
fprintf(file, "%d,", features->xd__plane__2__width);
fprintf(file, "%d,", features->xd__plane__2__height);
//fprintf(file, "%d,", features->xd__tile__mi_row_start);
fprintf(file, "%d,", features->xd__tile__mi_row_end);
//fprintf(file, "%d,", features->xd__tile__mi_col_start);
fprintf(file, "%d,", features->xd__tile__mi_col_end);
//fprintf(file, "%d,", features->xd__tile__tile_row);
//fprintf(file, "%d,", features->xd__tile__tile_col);
fprintf(file, "%s,", features->mbmi__bsize);
fprintf(file, "%s,", features->mbmi__partition);
fprintf(file, "%s,", features->mbmi__mode);
//fprintf(file, "%s,", features->mbmi__uv_mode);
fprintf(file, "%d,", features->mbmi__current_qindex);
//fprintf(file, "%s,", features->mbmi__motion_mode);
fprintf(file, "%d,", features->mbmi__num_proj_ref);
fprintf(file, "%d,", features->mbmi__overlappable_neighbors);
fprintf(file, "%s,", features->mbmi__interintra_mode);
fprintf(file, "%d,", features->mbmi__interintra_wedge_index);
fprintf(file, "%d,", features->mbmi__cfl_alpha_signs);
fprintf(file, "%d,", features->mbmi__cfl_alpha_idx);
fprintf(file, "%d,", features->mbmi__skip_txfm);
fprintf(file, "%d,", features->mbmi__tx_size);
fprintf(file, "%d,", features->mbmi__ref_mv_idx);
//fprintf(file, "%d,", features->mbmi__skip_mode);
//fprintf(file, "%d,", features->mbmi__use_intrabc);
fprintf(file, "%d,", features->mbmi__comp_group_idx);
fprintf(file, "%d,", features->mbmi__compound_idx);
fprintf(file, "%d,", features->mbmi__use_wedge_interintra);
//fprintf(file, "%d,", features->mbmi__cdef_strength);
fprintf(file, "%s,", features->mbmi__interp_filters__as_filters__y_filter);
fprintf(file, "%s,", features->mbmi__interp_filters__as_filters__x_filter);
fprintf(file, "%d,", features->mbmi__mv__0__as_mv__row);
fprintf(file, "%d,", features->mbmi__mv__1__as_mv__col);
fprintf(file, "%s,", features->xd__left_mbmi__bsize);
fprintf(file, "%s,", features->xd__left_mbmi__partition);
fprintf(file, "%s,", features->xd__left_mbmi__mode);
fprintf(file, "%s,", features->xd__left_mbmi__uv_mode);
fprintf(file, "%d,", features->xd__left_mbmi__current_qindex);
fprintf(file, "%s,", features->xd__left_mbmi__motion_mode);
fprintf(file, "%d,", features->xd__left_mbmi__num_proj_ref);
fprintf(file, "%d,", features->xd__left_mbmi__overlappable_neighbors);
fprintf(file, "%s,", features->xd__left_mbmi__interintra_mode);
fprintf(file, "%d,", features->xd__left_mbmi__interintra_wedge_index);
fprintf(file, "%d,", features->xd__left_mbmi__cfl_alpha_signs);
fprintf(file, "%d,", features->xd__left_mbmi__cfl_alpha_idx);
fprintf(file, "%d,", features->xd__left_mbmi__skip_txfm);
fprintf(file, "%d,", features->xd__left_mbmi__tx_size);
fprintf(file, "%d,", features->xd__left_mbmi__ref_mv_idx);
fprintf(file, "%d,", features->xd__left_mbmi__skip_mode);
fprintf(file, "%d,", features->xd__left_mbmi__use_intrabc);
fprintf(file, "%d,", features->xd__left_mbmi__comp_group_idx);
fprintf(file, "%d,", features->xd__left_mbmi__compound_idx);
fprintf(file, "%d,", features->xd__left_mbmi__use_wedge_interintra);
fprintf(file, "%d,", features->xd__left_mbmi__cdef_strength);
fprintf(file, "%s,", features->xd__left_mbmi__interp_filters__as_filters__y_filter);
fprintf(file, "%s,", features->xd__left_mbmi__interp_filters__as_filters__x_filter);
fprintf(file, "%d,", features->xd__left_mbmi__mv__0__as_mv__row);
fprintf(file, "%d,", features->xd__left_mbmi__mv__1__as_mv__col);
fprintf(file, "%s,", features->xd__above_mbmi__bsize);
fprintf(file, "%s,", features->xd__above_mbmi__partition);
fprintf(file, "%s,", features->xd__above_mbmi__mode);
fprintf(file, "%s,", features->xd__above_mbmi__uv_mode);
fprintf(file, "%d,", features->xd__above_mbmi__current_qindex);
fprintf(file, "%s,", features->xd__above_mbmi__motion_mode);
fprintf(file, "%d,", features->xd__above_mbmi__num_proj_ref);
fprintf(file, "%d,", features->xd__above_mbmi__overlappable_neighbors);
fprintf(file, "%s,", features->xd__above_mbmi__interintra_mode);
fprintf(file, "%d,", features->xd__above_mbmi__interintra_wedge_index);
fprintf(file, "%d,", features->xd__above_mbmi__cfl_alpha_signs);
fprintf(file, "%d,", features->xd__above_mbmi__cfl_alpha_idx);
fprintf(file, "%d,", features->xd__above_mbmi__skip_txfm);
fprintf(file, "%d,", features->xd__above_mbmi__tx_size);
fprintf(file, "%d,", features->xd__above_mbmi__ref_mv_idx);
fprintf(file, "%d,", features->xd__above_mbmi__skip_mode);
fprintf(file, "%d,", features->xd__above_mbmi__use_intrabc);
fprintf(file, "%d,", features->xd__above_mbmi__comp_group_idx);
fprintf(file, "%d,", features->xd__above_mbmi__compound_idx);
fprintf(file, "%d,", features->xd__above_mbmi__use_wedge_interintra);
fprintf(file, "%d,", features->xd__above_mbmi__cdef_strength);
fprintf(file, "%s,", features->xd__above_mbmi__interp_filters__as_filters__y_filter);
fprintf(file, "%s,", features->xd__above_mbmi__interp_filters__as_filters__x_filter);
fprintf(file, "%d,", features->xd__above_mbmi__mv__0__as_mv__row);
fprintf(file, "%d,", features->xd__above_mbmi__mv__1__as_mv__col);
fprintf(file, "%d,", features->rd_stats__rate);
fprintf(file, "%d,", features->rd_stats__zero_rate);
fprintf(file, "%lld,", features->rd_stats__dist);
fprintf(file, "%lld,", features->rd_stats__rdcost);
fprintf(file, "%lld,", features->rd_stats__sse);
fprintf(file, "%d,", features->rd_stats__skip_txfm);
fprintf(file, "%d,", features->rd_stats_y__rate);
fprintf(file, "%d,", features->rd_stats_y__zero_rate);
fprintf(file, "%lld,", features->rd_stats_y__dist);
fprintf(file, "%lld,", features->rd_stats_y__rdcost);
fprintf(file, "%lld,", features->rd_stats_y__sse);
fprintf(file, "%d,", features->rd_stats_y__skip_txfm);
fprintf(file, "%d,", features->rd_stats_uv__zero_rate);
fprintf(file, "%lld,", features->rd_stats_uv__dist);
fprintf(file, "%lld,", features->rd_stats_uv__rdcost);
fprintf(file, "%lld,", features->rd_stats_uv__sse);
fprintf(file, "%d,", features->rd_stats_uv__skip_txfm);
//fprintf(file, "%d,", features->cpi__skip_tpl_setup_stats);
fprintf(file, "%f,", features->cpi__tpl_rdmult_scaling_factors);
//fprintf(file, "%d,", features->cpi__rt_reduce_num_ref_buffers);
fprintf(file, "%f,", features->cpi__framerate);
fprintf(file, "%d,", features->cpi__ref_frame_flags);
//fprintf(file, "%d,", features->cpi__speed);
fprintf(file, "%d,", features->cpi__all_one_sided_refs);
fprintf(file, "%d,", features->cpi__gf_frame_index);
//fprintf(file, "%d,", features->cpi__droppable);
fprintf(file, "%d,", features->cpi__data_alloc_width);
fprintf(file, "%d,", features->cpi__data_alloc_height);
fprintf(file, "%d,", features->cpi__initial_mbs);
//fprintf(file, "%d,", features->cpi__frame_size_related_setup_done);
fprintf(file, "%d,", features->cpi__last_coded_width);
fprintf(file, "%d,", features->cpi__last_coded_height);
//fprintf(file, "%d,", features->cpi__num_frame_recode);
//fprintf(file, "%d,", features->cpi__intrabc_used);
//fprintf(file, "%d,", features->cpi__prune_ref_frame_mask);
fprintf(file, "%d,", features->cpi__use_screen_content_tools);
fprintf(file, "%d,", features->cpi__is_screen_content_type);
//fprintf(file, "%d,", features->cpi__frame_header_count);
//fprintf(file, "%d,", features->cpi__deltaq_used);
//fprintf(file, "%s,", features->cpi__last_frame_type);
//fprintf(file, "%d,", features->cpi__num_tg);
//fprintf(file, "%s,", features->cpi__superres_mode);
//fprintf(file, "%s,", features->cpi__fp_block_size);
//fprintf(file, "%d,", features->cpi__sb_counter);
//fprintf(file, "%d,", features->cpi__ref_refresh_index);
//fprintf(file, "%d,", features->cpi__refresh_idx_available);
//fprintf(file, "%d,", features->cpi__ref_idx_to_skip);
//fprintf(file, "%d,", features->cpi__do_frame_data_update);
//fprintf(file, "%f,", features->cpi__ext_rate_scale);
//fprintf(file, "%s,", features->cpi__weber_bsize);
//fprintf(file, "%lld,", features->cpi__norm_wiener_variance);
//fprintf(file, "%d,", features->cpi__is_dropped_frame);
//fprintf(file, "%llu,", features->cpi__rec_sse);
//fprintf(file, "%d,", features->cpi__frames_since_last_update);
//fprintf(file, "%d,", features->cpi__palette_pixel_num);
//fprintf(file, "%d,", features->cpi__scaled_last_source_available);
fprintf(file, "%lld,", features->cpi__ppi__ts_start_last_show_frame);
fprintf(file, "%lld,", features->cpi__ppi__ts_end_last_show_frame);
//fprintf(file, "%d,", features->cpi__ppi__num_fp_contexts);
fprintf(file, "%d,", features->cpi__ppi__filter_level__0);
fprintf(file, "%d,", features->cpi__ppi__filter_level__1);
fprintf(file, "%d,", features->cpi__ppi__filter_level_u);
fprintf(file, "%d,", features->cpi__ppi__filter_level_v);
//fprintf(file, "%d,", features->cpi__ppi__seq_params_locked);
fprintf(file, "%d,", features->cpi__ppi__internal_altref_allowed);
fprintf(file, "%d,", features->cpi__ppi__show_existing_alt_ref);
//fprintf(file, "%d,", features->cpi__ppi__lap_enabled);
//fprintf(file, "%d,", features->cpi__ppi__b_calculate_psnr);
fprintf(file, "%d,", features->cpi__ppi__frames_left);
//fprintf(file, "%d,", features->cpi__ppi__use_svc);
//fprintf(file, "%d,", features->cpi__ppi__buffer_removal_time_present);
//fprintf(file, "%d,", features->cpi__ppi__number_temporal_layers);
//fprintf(file, "%d,", features->cpi__ppi__number_spatial_layers);
fprintf(file, "%f,", features->cpi__ppi__tpl_sb_rdmult_scaling_factors);
fprintf(file, "%d,", features->cpi__ppi__gf_group__max_layer_depth);
fprintf(file, "%d,", features->cpi__ppi__gf_group__max_layer_depth_allowed);
fprintf(file, "%d,", features->cpi__ppi__gf_state__arf_gf_boost_lst);
fprintf(file, "%d,", features->cpi__ppi__twopass__section_intra_rating);
//fprintf(file, "%d,", features->cpi__ppi__twopass__first_pass_done);
fprintf(file, "%lld,", features->cpi__ppi__twopass__bits_left);
//fprintf(file, "%f,", features->cpi__ppi__twopass__modified_error_min);
fprintf(file, "%f,", features->cpi__ppi__twopass__modified_error_max);
//fprintf(file, "%f,", features->cpi__ppi__twopass__modified_error_left);
fprintf(file, "%lld,", features->cpi__ppi__twopass__kf_group_bits);
fprintf(file, "%f,", features->cpi__ppi__twopass__kf_group_error_left);
//fprintf(file, "%f,", features->cpi__ppi__twopass__bpm_factor);
fprintf(file, "%d,", features->cpi__ppi__twopass__rolling_arf_group_target_bits);
fprintf(file, "%d,", features->cpi__ppi__twopass__rolling_arf_group_actual_bits);
//fprintf(file, "%d,", features->cpi__ppi__twopass__sr_update_lag);
fprintf(file, "%d,", features->cpi__ppi__twopass__kf_zeromotion_pct);
fprintf(file, "%d,", features->cpi__ppi__twopass__last_kfgroup_zeromotion_pct);
//fprintf(file, "%d,", features->cpi__ppi__twopass__extend_minq);
//fprintf(file, "%d,", features->cpi__ppi__twopass__extend_maxq);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__frame);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__weight);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__intra_error);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__coded_error);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_cols);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__MVr);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__MVc);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__MVrv);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__MVcv);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__duration);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__count);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__raw_error_stdev);
//fprintf(file, "%lld,", features->cpi__ppi__twopass__firstpass_info__total_stats__is_flash);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__noise_var);
//fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__cor_coeff);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error);
fprintf(file, "%f,", features->cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__gf_group_bits);
fprintf(file, "%d,", features->cpi__ppi__p_rc__kf_boost);
fprintf(file, "%d,", features->cpi__ppi__p_rc__gfu_boost);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__cur_gf_index);
fprintf(file, "%d,", features->cpi__ppi__p_rc__num_regions);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__regions_offset);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__frames_till_regions_update);
fprintf(file, "%d,", features->cpi__ppi__p_rc__baseline_gf_interval);
fprintf(file, "%d,", features->cpi__ppi__p_rc__constrained_gf_group);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__this_key_frame_forced);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__next_key_frame_forced);
//fprintf(file, "%lld,", features->cpi__ppi__p_rc__starting_buffer_level);
//fprintf(file, "%lld,", features->cpi__ppi__p_rc__optimal_buffer_level);
//fprintf(file, "%lld,", features->cpi__ppi__p_rc__maximum_buffer_size);
fprintf(file, "%d,", features->cpi__ppi__p_rc__arf_q);
fprintf(file, "%f,", features->cpi__ppi__p_rc__arf_boost_factor);
fprintf(file, "%d,", features->cpi__ppi__p_rc__base_layer_qp);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__num_stats_used_for_kf_boost);
fprintf(file, "%d,", features->cpi__ppi__p_rc__num_stats_used_for_gfu_boost);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__num_stats_required_for_gfu_boost);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__enable_scenecut_detection);
//fprintf(file, "%d,", features->cpi__ppi__p_rc__use_arf_in_this_kf_group);
fprintf(file, "%d,", features->cpi__ppi__p_rc__ni_frames);
fprintf(file, "%f,", features->cpi__ppi__p_rc__tot_q);
fprintf(file, "%d,", features->cpi__ppi__p_rc__last_kf_qindex);
fprintf(file, "%d,", features->cpi__ppi__p_rc__last_boosted_qindex);
fprintf(file, "%f,", features->cpi__ppi__p_rc__avg_q);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__total_actual_bits);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__total_target_bits);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__buffer_level);
fprintf(file, "%d,", features->cpi__ppi__p_rc__rate_error_estimate);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__vbr_bits_off_target);
//fprintf(file, "%lld,", features->cpi__ppi__p_rc__vbr_bits_off_target_fast);
fprintf(file, "%lld,", features->cpi__ppi__p_rc__bits_off_target);
fprintf(file, "%d,", features->cpi__ppi__p_rc__rolling_target_bits);
fprintf(file, "%d,", features->cpi__ppi__p_rc__rolling_actual_bits);
//fprintf(file, "%d,", features->cpi__ppi__tf_info__is_temporal_filter_on);
fprintf(file, "%lld,", features->cpi__ppi__tf_info__frame_diff__0__sum);
fprintf(file, "%lld,", features->cpi__ppi__tf_info__frame_diff__0__sse);
fprintf(file, "%lld,", features->cpi__ppi__tf_info__frame_diff__1__sum);
fprintf(file, "%lld,", features->cpi__ppi__tf_info__frame_diff__1__sse);
//fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_gf_index__0);
fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_gf_index__1);
//fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_display_index_offset__0);
fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_display_index_offset__1);
fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_valid__0);
fprintf(file, "%d,", features->cpi__ppi__tf_info__tf_buf_valid__1);
fprintf(file, "%d,", features->cpi__ppi__seq_params__num_bits_width);
fprintf(file, "%d,", features->cpi__ppi__seq_params__num_bits_height);
fprintf(file, "%d,", features->cpi__ppi__seq_params__max_frame_width);
fprintf(file, "%d,", features->cpi__ppi__seq_params__max_frame_height);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__frame_id_numbers_present_flag);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__frame_id_length);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__delta_frame_id_length);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__sb_size);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__mib_size);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__mib_size_log2);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__force_screen_content_tools);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__still_picture);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__reduced_still_picture_hdr);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__force_integer_mv);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_filter_intra);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_intra_edge_filter);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_interintra_compound);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_masked_compound);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_dual_filter);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_warped_motion);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_superres);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_cdef);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__enable_restoration);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__profile);
fprintf(file, "%d,", features->cpi__ppi__seq_params__bit_depth);
fprintf(file, "%d,", features->cpi__ppi__seq_params__use_highbitdepth);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__monochrome);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__color_primaries);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__transfer_characteristics);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__matrix_coefficients);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__color_range);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__subsampling_x);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__subsampling_y);
//fprintf(file, "%s,", features->cpi__ppi__seq_params__chroma_sample_position);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__separate_uv_delta_q);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__film_grain_params_present);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__operating_points_cnt_minus_1);
//fprintf(file, "%d,", features->cpi__ppi__seq_params__has_nonzero_operating_point_idc);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__ready);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_stats_block_mis_log2);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_bsize_1d);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__frame_idx);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__border_in_pixels);
//fprintf(file, "%f,", features->cpi__ppi__tpl_data__r0_adjust_factor);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__is_valid);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__stride);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__width);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__height);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__mi_rows);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__mi_cols);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__base_rdmult);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__frame_display_index);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__use_pred_sad);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist);
//fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_sse);
//fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_dist);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate);
fprintf(file, "%lld,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate);
fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_rate);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__sf__x_scale_fp);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__sf__y_scale_fp);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__sf__x_step_q4);
//fprintf(file, "%d,", features->cpi__ppi__tpl_data__sf__y_step_q4);
fprintf(file, "%d,", features->cpi__common__width);
fprintf(file, "%d,", features->cpi__common__height);
fprintf(file, "%d,", features->cpi__common__render_width);
fprintf(file, "%d,", features->cpi__common__render_height);
fprintf(file, "%d,", features->cpi__common__superres_upscaled_width);
fprintf(file, "%d,", features->cpi__common__superres_upscaled_height);
//fprintf(file, "%d,", features->cpi__common__superres_scale_denominator);
fprintf(file, "%d,", features->cpi__common__frame_presentation_time);
fprintf(file, "%d,", features->cpi__common__show_frame);
fprintf(file, "%d,", features->cpi__common__showable_frame);
//fprintf(file, "%d,", features->cpi__common__show_existing_frame);
//fprintf(file, "%s,", features->cpi__common__current_frame__frame_type);
//fprintf(file, "%s,", features->cpi__common__current_frame__reference_mode);
fprintf(file, "%d,", features->cpi__common__current_frame__order_hint);
fprintf(file, "%d,", features->cpi__common__current_frame__display_order_hint);
fprintf(file, "%d,", features->cpi__common__current_frame__pyramid_level);
fprintf(file, "%d,", features->cpi__common__current_frame__frame_number);
fprintf(file, "%d,", features->cpi__common__current_frame__refresh_frame_flags);
//fprintf(file, "%d,", features->cpi__common__current_frame__frame_refs_short_signaling);
//fprintf(file, "%d,", features->cpi__common__sf_identity__x_scale_fp);
//fprintf(file, "%d,", features->cpi__common__sf_identity__y_scale_fp);
//fprintf(file, "%d,", features->cpi__common__sf_identity__x_step_q4);
//fprintf(file, "%d,", features->cpi__common__sf_identity__y_step_q4);
//fprintf(file, "%d,", features->cpi__common__features__disable_cdf_update);
fprintf(file, "%d,", features->cpi__common__features__allow_high_precision_mv);
//fprintf(file, "%d,", features->cpi__common__features__cur_frame_force_integer_mv);
fprintf(file, "%d,", features->cpi__common__features__allow_screen_content_tools);
fprintf(file, "%d,", features->cpi__common__features__allow_intrabc);
//fprintf(file, "%d,", features->cpi__common__features__allow_warped_motion);
//fprintf(file, "%d,", features->cpi__common__features__allow_ref_frame_mvs);
//fprintf(file, "%d,", features->cpi__common__features__coded_lossless);
//fprintf(file, "%d,", features->cpi__common__features__all_lossless);
//fprintf(file, "%d,", features->cpi__common__features__reduced_tx_set_used);
//fprintf(file, "%d,", features->cpi__common__features__error_resilient_mode);
//fprintf(file, "%d,", features->cpi__common__features__switchable_motion_mode);
fprintf(file, "%s,", features->cpi__common__features__tx_mode);
//fprintf(file, "%s,", features->cpi__common__features__interp_filter);
fprintf(file, "%d,", features->cpi__common__features__primary_ref_frame);
//fprintf(file, "%d,", features->cpi__common__features__byte_alignment);
//fprintf(file, "%s,", features->cpi__common__features__refresh_frame_context);
fprintf(file, "%d,", features->cpi__common__mi_params__mb_rows);
fprintf(file, "%d,", features->cpi__common__mi_params__mb_cols);
fprintf(file, "%d,", features->cpi__common__mi_params__MBs);
fprintf(file, "%d,", features->cpi__common__mi_params__mi_rows);
fprintf(file, "%d,", features->cpi__common__mi_params__mi_cols);
fprintf(file, "%d,", features->cpi__common__mi_params__mi_stride);
//fprintf(file, "%f,", features->cpi__oxcf__noise_level);
//fprintf(file, "%d,", features->cpi__oxcf__noise_block_size);
//fprintf(file, "%d,", features->cpi__oxcf__enable_dnl_denoising);
//fprintf(file, "%d,", features->cpi__oxcf__tier_mask);
//fprintf(file, "%d,", features->cpi__oxcf__border_in_pixels);
//fprintf(file, "%d,", features->cpi__oxcf__max_threads);
//fprintf(file, "%d,", features->cpi__oxcf__speed);
//fprintf(file, "%s,", features->cpi__oxcf__profile);
//fprintf(file, "%s,", features->cpi__oxcf__pass);
//fprintf(file, "%d,", features->cpi__oxcf__passes);
//fprintf(file, "%s,", features->cpi__oxcf__mode);
fprintf(file, "%d,", features->cpi__oxcf__use_highbitdepth);
//fprintf(file, "%d,", features->cpi__oxcf__save_as_annexb);
fprintf(file, "%f,", features->cpi__oxcf__input_cfg__init_framerate);
fprintf(file, "%d,", features->cpi__oxcf__input_cfg__input_bit_depth);
//fprintf(file, "%d,", features->cpi__oxcf__input_cfg__limit);
//fprintf(file, "%d,", features->cpi__oxcf__input_cfg__chroma_subsampling_x);
//fprintf(file, "%d,", features->cpi__oxcf__input_cfg__chroma_subsampling_y);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__sharpness);
//fprintf(file, "%s,", features->cpi__oxcf__algo_cfg__disable_trellis_quant);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__arnr_max_frames);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__arnr_strength);
//fprintf(file, "%s,", features->cpi__oxcf__algo_cfg__cdf_update_mode);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__enable_tpl_model);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__enable_overlay);
//fprintf(file, "%s,", features->cpi__oxcf__algo_cfg__loopfilter_control);
//fprintf(file, "%d,", features->cpi__oxcf__algo_cfg__skip_postproc_filtering);
//fprintf(file, "%lld,", features->cpi__oxcf__rc_cfg__starting_buffer_level_ms);
//fprintf(file, "%lld,", features->cpi__oxcf__rc_cfg__optimal_buffer_level_ms);
//fprintf(file, "%lld,", features->cpi__oxcf__rc_cfg__maximum_buffer_size_ms);
//fprintf(file, "%lld,", features->cpi__oxcf__rc_cfg__target_bandwidth);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__vbr_corpus_complexity_lap);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__max_intra_bitrate_pct);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__max_inter_bitrate_pct);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__gf_cbr_boost_pct);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__min_cr);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__drop_frames_water_mark);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__under_shoot_pct);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__over_shoot_pct);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__worst_allowed_q);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__best_allowed_q);
fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__cq_level);
//fprintf(file, "%s,", features->cpi__oxcf__rc_cfg__mode);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__vbrbias);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__vbrmin_section);
//fprintf(file, "%d,", features->cpi__oxcf__rc_cfg__vbrmax_section);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__use_fixed_qp_offsets);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__qm_minlevel);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__qm_maxlevel);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__quant_b_adapt);
//fprintf(file, "%s,", features->cpi__oxcf__q_cfg__aq_mode);
//fprintf(file, "%s,", features->cpi__oxcf__q_cfg__deltaq_mode);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__deltaq_strength);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__enable_chroma_deltaq);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__enable_hdr_deltaq);
//fprintf(file, "%d,", features->cpi__oxcf__q_cfg__using_qm);
//fprintf(file, "%d,", features->cpi__tf_ctx__num_frames);
fprintf(file, "%f,", features->cpi__tf_ctx__noise_levels__0);
fprintf(file, "%f,", features->cpi__tf_ctx__noise_levels__1);
fprintf(file, "%f,", features->cpi__tf_ctx__noise_levels__2);
//fprintf(file, "%d,", features->cpi__tf_ctx__num_pels);
fprintf(file, "%d,", features->cpi__tf_ctx__mb_rows);
fprintf(file, "%d,", features->cpi__tf_ctx__mb_cols);
//fprintf(file, "%d,", features->cpi__tf_ctx__q_factor);
fprintf(file, "%d,", features->cpi__rd__rdmult);
fprintf(file, "%f,", features->cpi__rd__r0);
//fprintf(file, "%d,", features->cpi__mv_search_params__max_mv_magnitude);
fprintf(file, "%d,", features->cpi__mv_search_params__mv_step_param);
fprintf(file, "%d,", features->cpi__frame_info__frame_width);
fprintf(file, "%d,", features->cpi__frame_info__frame_height);
fprintf(file, "%d,", features->cpi__frame_info__mi_rows);
fprintf(file, "%d,", features->cpi__frame_info__mi_cols);
fprintf(file, "%d,", features->cpi__frame_info__mb_rows);
fprintf(file, "%d,", features->cpi__frame_info__mb_cols);
fprintf(file, "%d,", features->cpi__frame_info__num_mbs);
fprintf(file, "%d,", features->cpi__frame_info__bit_depth);
//fprintf(file, "%d,", features->cpi__frame_info__subsampling_x);
//fprintf(file, "%d,", features->cpi__frame_info__subsampling_y);
//fprintf(file, "%d,", features->cpi__interp_search_flags__default_interp_skip_flags);
//fprintf(file, "%d,", features->cpi__interp_search_flags__interp_filter_search_mask);
fprintf(file, "%f,", features->cpi__twopass_frame__mb_av_energy);
fprintf(file, "%s,", features->cpi__twopass_frame__fr_content_type);
//fprintf(file, "%f,", features->cpi__twopass_frame__frame_avg_haar_energy);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__frame);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__weight);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__intra_error);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__frame_avg_wavelet_energy);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__coded_error);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__sr_coded_error);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__pcnt_inter);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__pcnt_motion);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__pcnt_second_ref);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__pcnt_neutral);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__intra_skip_pct);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__inactive_zone_rows);
//fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__inactive_zone_cols);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__MVr);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__mvr_abs);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__MVc);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__mvc_abs);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__MVrv);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__MVcv);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__mv_in_out_count);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__new_mv_count);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__duration);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__count);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__raw_error_stdev);
//fprintf(file, "%lld,", features->cpi__twopass_frame__stats_in__is_flash);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__noise_var);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__cor_coeff);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__log_intra_error);
fprintf(file, "%f,", features->cpi__twopass_frame__stats_in__log_coded_error);
//fprintf(file, "%llu,", features->tile_data__abs_sum_level);
//fprintf(file, "%u,", features->tile_data__allow_update_cdf);
//fprintf(file, "%d,", features->tile_data__tile_info__mi_row_start);
fprintf(file, "%d,", features->tile_data__tile_info__mi_row_end);
//fprintf(file, "%d,", features->tile_data__tile_info__mi_col_start);
fprintf(file, "%d,", features->tile_data__tile_info__mi_col_end);
//fprintf(file, "%d,", features->tile_data__tile_info__tile_row);
//fprintf(file, "%d,", features->tile_data__tile_info__tile_col);
//fprintf(file, "%d,", features->tile_data__firstpass_top_mv__row);
//fprintf(file, "%d,", features->tile_data__firstpass_top_mv__col);
fprintf(file, "%d,", features->args__ref_frame_cost);
fprintf(file, "%d,", features->args__single_comp_cost);
//fprintf(file, "%d,", features->args__skip_ifs);
//fprintf(file, "%d,", features->args__start_mv_cnt);
//fprintf(file, "%d,", features->args__interp_filter_stats_idx);
fprintf(file, "%d,", features->args__wedge_index);
fprintf(file, "%d,", features->args__wedge_sign);
fprintf(file, "%d,", features->args__diffwtd_index);
fprintf(file, "%u,", features->args__best_pred_sse);
fprintf(file, "%d", features->discard);
fprintf(file, "\n");

  fclose(file);
}
/* clang-format on */

void collect_filename(const char *fn) {
  strcpy(filename, fn);
  strcat(filename, "_warp");
  strcat(filename, ".csv");

  FILE *file = fopen(filename, "w");

  fprintf(file,
          "bsize,ref_best_rd,best_est_rd,do_tx_search,rate_mv,is_comp_pred,rate2_nocoeff,interintra_allowed,update_type,x__qindex,x__delta_qindex,x__rdmult_delta_qindex,x__rdmult_cur_qindex,x__rdmult,x__rb,x__errorperbit,x__sadperbit,x__cnt_zeromv,x__recalc_luma_mc_data,x__source_variance,x__palette_pixels,x__plane__0__src_diff,x__plane__0__dqcoeff,x__plane__0__qcoeff,x__plane__0__coeff,x__plane__0__eobs,x__plane__0__quant_fp_QTX,x__plane__0__round_fp_QTX,x__plane__0__quant_QTX,x__plane__0__round_QTX,x__plane__0__quant_shift_QTX,x__plane__0__zbin_QTX,x__plane__0__dequant_QTX,xd__mi_row,xd__mi_col,xd__mi_stride,xd__is_chroma_ref,xd__up_available,xd__left_available,xd__chroma_up_available,xd__chroma_left_available,xd__tx_type_map_stride,xd__mb_to_left_edge,xd__mb_to_right_edge,xd__mb_to_top_edge,xd__mb_to_bottom_edge,xd__width,xd__height,xd__is_last_vertical_rect,xd__is_first_horizontal_rect,xd__bd,xd__current_base_qindex,xd__plane__0__width,xd__plane__0__height,xd__plane__1__width,xd__plane__1__height,xd__plane__2__width,xd__plane__2__height,xd__tile__mi_row_end,xd__tile__mi_col_end,mbmi__bsize,mbmi__partition,mbmi__mode,mbmi__current_qindex,mbmi__num_proj_ref,mbmi__overlappable_neighbors,mbmi__interintra_mode,mbmi__interintra_wedge_index,mbmi__cfl_alpha_signs,mbmi__cfl_alpha_idx,mbmi__skip_txfm,mbmi__tx_size,mbmi__ref_mv_idx,mbmi__comp_group_idx,mbmi__compound_idx,mbmi__use_wedge_interintra,mbmi__interp_filters__as_filters__y_filter,mbmi__interp_filters__as_filters__x_filter,mbmi__mv__0__as_mv__row,mbmi__mv__1__as_mv__col,xd__left_mbmi__bsize,xd__left_mbmi__partition,xd__left_mbmi__mode,xd__left_mbmi__uv_mode,xd__left_mbmi__current_qindex,xd__left_mbmi__motion_mode,xd__left_mbmi__num_proj_ref,xd__left_mbmi__overlappable_neighbors,xd__left_mbmi__interintra_mode,xd__left_mbmi__interintra_wedge_index,xd__left_mbmi__cfl_alpha_signs,xd__left_mbmi__cfl_alpha_idx,xd__left_mbmi__skip_txfm,xd__left_mbmi__tx_size,xd__left_mbmi__ref_mv_idx,xd__left_mbmi__skip_mode,xd__left_mbmi__use_intrabc,xd__left_mbmi__comp_group_idx,xd__left_mbmi__compound_idx,xd__left_mbmi__use_wedge_interintra,xd__left_mbmi__cdef_strength,xd__left_mbmi__interp_filters__as_filters__y_filter,xd__left_mbmi__interp_filters__as_filters__x_filter,xd__left_mbmi__mv__0__as_mv__row,xd__left_mbmi__mv__1__as_mv__col,xd__above_mbmi__bsize,xd__above_mbmi__partition,xd__above_mbmi__mode,xd__above_mbmi__uv_mode,xd__above_mbmi__current_qindex,xd__above_mbmi__motion_mode,xd__above_mbmi__num_proj_ref,xd__above_mbmi__overlappable_neighbors,xd__above_mbmi__interintra_mode,xd__above_mbmi__interintra_wedge_index,xd__above_mbmi__cfl_alpha_signs,xd__above_mbmi__cfl_alpha_idx,xd__above_mbmi__skip_txfm,xd__above_mbmi__tx_size,xd__above_mbmi__ref_mv_idx,xd__above_mbmi__skip_mode,xd__above_mbmi__use_intrabc,xd__above_mbmi__comp_group_idx,xd__above_mbmi__compound_idx,xd__above_mbmi__use_wedge_interintra,xd__above_mbmi__cdef_strength,xd__above_mbmi__interp_filters__as_filters__y_filter,xd__above_mbmi__interp_filters__as_filters__x_filter,xd__above_mbmi__mv__0__as_mv__row,xd__above_mbmi__mv__1__as_mv__col,rd_stats__rate,rd_stats__zero_rate,rd_stats__dist,rd_stats__rdcost,rd_stats__sse,rd_stats__skip_txfm,rd_stats_y__rate,rd_stats_y__zero_rate,rd_stats_y__dist,rd_stats_y__rdcost,rd_stats_y__sse,rd_stats_y__skip_txfm,rd_stats_uv__zero_rate,rd_stats_uv__dist,rd_stats_uv__rdcost,rd_stats_uv__sse,rd_stats_uv__skip_txfm,cpi__tpl_rdmult_scaling_factors,cpi__framerate,cpi__ref_frame_flags,cpi__all_one_sided_refs,cpi__gf_frame_index,cpi__data_alloc_width,cpi__data_alloc_height,cpi__initial_mbs,cpi__last_coded_width,cpi__last_coded_height,cpi__use_screen_content_tools,cpi__is_screen_content_type,cpi__ppi__ts_start_last_show_frame,cpi__ppi__ts_end_last_show_frame,cpi__ppi__filter_level__0,cpi__ppi__filter_level__1,cpi__ppi__filter_level_u,cpi__ppi__filter_level_v,cpi__ppi__internal_altref_allowed,cpi__ppi__show_existing_alt_ref,cpi__ppi__frames_left,cpi__ppi__tpl_sb_rdmult_scaling_factors,cpi__ppi__gf_group__max_layer_depth,cpi__ppi__gf_group__max_layer_depth_allowed,cpi__ppi__gf_state__arf_gf_boost_lst,cpi__ppi__twopass__section_intra_rating,cpi__ppi__twopass__bits_left,cpi__ppi__twopass__modified_error_max,cpi__ppi__twopass__kf_group_bits,cpi__ppi__twopass__kf_group_error_left,cpi__ppi__twopass__rolling_arf_group_target_bits,cpi__ppi__twopass__rolling_arf_group_actual_bits,cpi__ppi__twopass__kf_zeromotion_pct,cpi__ppi__twopass__last_kfgroup_zeromotion_pct,cpi__ppi__twopass__firstpass_info__total_stats__weight,cpi__ppi__twopass__firstpass_info__total_stats__intra_error,cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy,cpi__ppi__twopass__firstpass_info__total_stats__coded_error,cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error,cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter,cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion,cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref,cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral,cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct,cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows,cpi__ppi__twopass__firstpass_info__total_stats__MVr,cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs,cpi__ppi__twopass__firstpass_info__total_stats__MVc,cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs,cpi__ppi__twopass__firstpass_info__total_stats__MVrv,cpi__ppi__twopass__firstpass_info__total_stats__MVcv,cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count,cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count,cpi__ppi__twopass__firstpass_info__total_stats__duration,cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error,cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error,cpi__ppi__p_rc__gf_group_bits,cpi__ppi__p_rc__kf_boost,cpi__ppi__p_rc__gfu_boost,cpi__ppi__p_rc__num_regions,cpi__ppi__p_rc__baseline_gf_interval,cpi__ppi__p_rc__constrained_gf_group,cpi__ppi__p_rc__arf_q,cpi__ppi__p_rc__arf_boost_factor,cpi__ppi__p_rc__base_layer_qp,cpi__ppi__p_rc__num_stats_used_for_gfu_boost,cpi__ppi__p_rc__ni_frames,cpi__ppi__p_rc__tot_q,cpi__ppi__p_rc__last_kf_qindex,cpi__ppi__p_rc__last_boosted_qindex,cpi__ppi__p_rc__avg_q,cpi__ppi__p_rc__total_actual_bits,cpi__ppi__p_rc__total_target_bits,cpi__ppi__p_rc__buffer_level,cpi__ppi__p_rc__rate_error_estimate,cpi__ppi__p_rc__vbr_bits_off_target,cpi__ppi__p_rc__bits_off_target,cpi__ppi__p_rc__rolling_target_bits,cpi__ppi__p_rc__rolling_actual_bits,cpi__ppi__tf_info__frame_diff__0__sum,cpi__ppi__tf_info__frame_diff__0__sse,cpi__ppi__tf_info__frame_diff__1__sum,cpi__ppi__tf_info__frame_diff__1__sse,cpi__ppi__tf_info__tf_buf_gf_index__1,cpi__ppi__tf_info__tf_buf_display_index_offset__1,cpi__ppi__tf_info__tf_buf_valid__0,cpi__ppi__tf_info__tf_buf_valid__1,cpi__ppi__seq_params__num_bits_width,cpi__ppi__seq_params__num_bits_height,cpi__ppi__seq_params__max_frame_width,cpi__ppi__seq_params__max_frame_height,cpi__ppi__seq_params__bit_depth,cpi__ppi__seq_params__use_highbitdepth,cpi__ppi__tpl_data__ready,cpi__ppi__tpl_data__frame_idx,cpi__ppi__tpl_data__tpl_frame__is_valid,cpi__ppi__tpl_data__tpl_frame__stride,cpi__ppi__tpl_data__tpl_frame__width,cpi__ppi__tpl_data__tpl_frame__height,cpi__ppi__tpl_data__tpl_frame__mi_rows,cpi__ppi__tpl_data__tpl_frame__mi_cols,cpi__ppi__tpl_data__tpl_frame__base_rdmult,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate,cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate,cpi__common__width,cpi__common__height,cpi__common__render_width,cpi__common__render_height,cpi__common__superres_upscaled_width,cpi__common__superres_upscaled_height,cpi__common__frame_presentation_time,cpi__common__show_frame,cpi__common__showable_frame,cpi__common__current_frame__order_hint,cpi__common__current_frame__display_order_hint,cpi__common__current_frame__pyramid_level,cpi__common__current_frame__frame_number,cpi__common__current_frame__refresh_frame_flags,cpi__common__features__allow_high_precision_mv,cpi__common__features__allow_screen_content_tools,cpi__common__features__allow_intrabc,cpi__common__features__tx_mode,cpi__common__features__primary_ref_frame,cpi__common__mi_params__mb_rows,cpi__common__mi_params__mb_cols,cpi__common__mi_params__MBs,cpi__common__mi_params__mi_rows,cpi__common__mi_params__mi_cols,cpi__common__mi_params__mi_stride,cpi__oxcf__use_highbitdepth,cpi__oxcf__input_cfg__init_framerate,cpi__oxcf__input_cfg__input_bit_depth,cpi__oxcf__rc_cfg__cq_level,cpi__tf_ctx__noise_levels__0,cpi__tf_ctx__noise_levels__1,cpi__tf_ctx__noise_levels__2,cpi__tf_ctx__mb_rows,cpi__tf_ctx__mb_cols,cpi__rd__rdmult,cpi__rd__r0,cpi__mv_search_params__mv_step_param,cpi__frame_info__frame_width,cpi__frame_info__frame_height,cpi__frame_info__mi_rows,cpi__frame_info__mi_cols,cpi__frame_info__mb_rows,cpi__frame_info__mb_cols,cpi__frame_info__num_mbs,cpi__frame_info__bit_depth,cpi__twopass_frame__mb_av_energy,cpi__twopass_frame__fr_content_type,cpi__twopass_frame__stats_in__frame,cpi__twopass_frame__stats_in__weight,cpi__twopass_frame__stats_in__intra_error,cpi__twopass_frame__stats_in__frame_avg_wavelet_energy,cpi__twopass_frame__stats_in__coded_error,cpi__twopass_frame__stats_in__sr_coded_error,cpi__twopass_frame__stats_in__pcnt_inter,cpi__twopass_frame__stats_in__pcnt_motion,cpi__twopass_frame__stats_in__pcnt_second_ref,cpi__twopass_frame__stats_in__pcnt_neutral,cpi__twopass_frame__stats_in__intra_skip_pct,cpi__twopass_frame__stats_in__inactive_zone_rows,cpi__twopass_frame__stats_in__MVr,cpi__twopass_frame__stats_in__mvr_abs,cpi__twopass_frame__stats_in__MVc,cpi__twopass_frame__stats_in__mvc_abs,cpi__twopass_frame__stats_in__MVrv,cpi__twopass_frame__stats_in__MVcv,cpi__twopass_frame__stats_in__mv_in_out_count,cpi__twopass_frame__stats_in__new_mv_count,cpi__twopass_frame__stats_in__duration,cpi__twopass_frame__stats_in__count,cpi__twopass_frame__stats_in__raw_error_stdev,cpi__twopass_frame__stats_in__noise_var,cpi__twopass_frame__stats_in__cor_coeff,cpi__twopass_frame__stats_in__log_intra_error,cpi__twopass_frame__stats_in__log_coded_error,tile_data__tile_info__mi_row_end,tile_data__tile_info__mi_col_end,args__ref_frame_cost,args__single_comp_cost,args__wedge_index,args__wedge_sign,args__diffwtd_index,args__best_pred_sse,discard");

  fprintf(file, "\n");
  fclose(file);
}