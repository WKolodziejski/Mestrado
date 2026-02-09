#ifndef AOM_FEATURES_COLLECT_H
#define AOM_FEATURES_COLLECT_H

// Deve gerar csv?
#define WILLIAM_COLLECT_FEATURES 0

struct features_interp_filters {
  int filter_idx;
  char *bsize;  // str
  int skip_pred;
  int num_planes;
  int tmp_rs;
  long long int min_rd;
  int switchable_rate;
  long long int rd;
  int last_best__as_filters__y_filter;
  int last_best__as_filters__x_filter;

  int discard;  // feature a ser predita

  // x
  int x__qindex;
  int x__delta_qindex;
  int x__rdmult_delta_qindex;
  int x__rdmult_cur_qindex;
  int x__rdmult;
  int x__intra_sb_rdmult_modifier;
  double x__rb;
  int x__mb_energy;
  int x__sb_energy_level;
  int x__errorperbit;
  int x__sadperbit;
  int x__seg_skip_block;
  int x__actual_num_seg1_blocks;
  int x__actual_num_seg2_blocks;
  int x__cnt_zeromv;
  int x__force_zeromv_skip_for_sb;
  int x__force_zeromv_skip_for_blk;
  int x__nonrd_prune_ref_frame_search;
  int x__must_find_valid_partition;
  int x__skip_mode;
  int x__winner_mode_count;
  int x__recalc_luma_mc_data;
  int x__reuse_inter_pred;
  unsigned int x__source_variance;
  int x__block_is_zero_sad;
  int x__sb_me_partition;
  int x__sb_me_block;
  int x__sb_force_fixed_part;
  int x__try_merge_partition;
  int x__palette_pixels;

  // x->plane[0]
  int x__plane__0__src_diff;
  int x__plane__0__dqcoeff;
  int x__plane__0__qcoeff;
  int x__plane__0__coeff;
  int x__plane__0__eobs;
  int x__plane__0__quant_fp_QTX;
  int x__plane__0__round_fp_QTX;
  int x__plane__0__quant_QTX;
  int x__plane__0__round_QTX;
  int x__plane__0__quant_shift_QTX;
  int x__plane__0__zbin_QTX;
  int x__plane__0__dequant_QTX;

  // xd
  int xd__mi_row;
  int xd__mi_col;
  int xd__mi_stride;
  int xd__is_chroma_ref;
  int xd__up_available;
  int xd__left_available;
  int xd__chroma_up_available;
  int xd__chroma_left_available;
  int xd__tx_type_map_stride;
  int xd__mb_to_left_edge;
  int xd__mb_to_right_edge;
  int xd__mb_to_top_edge;
  int xd__mb_to_bottom_edge;
  int xd__width;
  int xd__height;
  int xd__is_last_vertical_rect;
  int xd__is_first_horizontal_rect;
  int xd__bd;
  int xd__current_base_qindex;
  int xd__cur_frame_force_integer_mv;
  signed char xd__delta_lf_from_base;

  // xd->block_ref_scale_factors
  int xd__block_ref_scale_factors__0__x_scale_fp;
  int xd__block_ref_scale_factors__0__y_scale_fp;
  int xd__block_ref_scale_factors__0__x_step_q4;
  int xd__block_ref_scale_factors__0__y_step_q4;
  int xd__block_ref_scale_factors__1__x_scale_fp;
  int xd__block_ref_scale_factors__1__y_scale_fp;
  int xd__block_ref_scale_factors__1__x_step_q4;
  int xd__block_ref_scale_factors__1__y_step_q4;

  // xd->plane
  char *xd__plane__0__plane_type;  // str
  int xd__plane__0__subsampling_x;
  int xd__plane__0__width;
  int xd__plane__0__height;
  char *xd__plane__1__plane_type;  // str
  int xd__plane__1__subsampling_x;
  int xd__plane__1__width;
  int xd__plane__1__height;
  char *xd__plane__2__plane_type;  // str
  int xd__plane__2__subsampling_x;
  int xd__plane__2__width;
  int xd__plane__2__height;

  // xd->tile
  int xd__tile__mi_row_start;
  int xd__tile__mi_row_end;
  int xd__tile__mi_col_start;
  int xd__tile__mi_col_end;
  int xd__tile__tile_row;
  int xd__tile__tile_col;

  // mbi
  char *mbmi__bsize;      // str
  char *mbmi__partition;  // str
  char *mbmi__mode;       // str
  char *mbmi__uv_mode;    // str
  int mbmi__current_qindex;
  char *mbmi__motion_mode;  // str
  int mbmi__num_proj_ref;
  int mbmi__overlappable_neighbors;
  char *mbmi__interintra_mode;  // str
  unsigned char mbmi__interintra_wedge_index;
  unsigned char mbmi__cfl_alpha_signs;
  int mbmi__cfl_alpha_idx;
  int mbmi__skip_txfm;
  int mbmi__tx_size;
  int mbmi__ref_mv_idx;
  int mbmi__skip_mode;
  int mbmi__use_intrabc;
  int mbmi__comp_group_idx;
  int mbmi__compound_idx;
  int mbmi__use_wedge_interintra;
  unsigned char mbmi__cdef_strength;

  // mbi->interp_filter
  int mbmi__interp_filters__as_filters__y_filter;
  int mbmi__interp_filters__as_filters__x_filter;

  // mbi->mv
  int mbmi__mv__0__as_mv__row;
  int mbmi__mv__1__as_mv__col;

  // xd__left_mbmi
  char *xd__left_mbmi__bsize;      // str
  char *xd__left_mbmi__partition;  // str
  char *xd__left_mbmi__mode;       // str
  char *xd__left_mbmi__uv_mode;    // str
  int xd__left_mbmi__current_qindex;
  char *xd__left_mbmi__motion_mode;  // str
  int xd__left_mbmi__num_proj_ref;
  int xd__left_mbmi__overlappable_neighbors;
  char *xd__left_mbmi__interintra_mode;  // str
  unsigned char xd__left_mbmi__interintra_wedge_index;
  unsigned char xd__left_mbmi__cfl_alpha_signs;
  int xd__left_mbmi__cfl_alpha_idx;
  int xd__left_mbmi__skip_txfm;
  int xd__left_mbmi__tx_size;
  int xd__left_mbmi__ref_mv_idx;
  int xd__left_mbmi__skip_mode;
  int xd__left_mbmi__use_intrabc;
  int xd__left_mbmi__comp_group_idx;
  int xd__left_mbmi__compound_idx;
  int xd__left_mbmi__use_wedge_interintra;
  unsigned char xd__left_mbmi__cdef_strength;

  // xd__left_mbmi->interp_filter
  int xd__left_mbmi__interp_filters__as_filters__y_filter;
  int xd__left_mbmi__interp_filters__as_filters__x_filter;

  // xd__left_mbmi->mv
  int xd__left_mbmi__mv__0__as_mv__row;
  int xd__left_mbmi__mv__1__as_mv__col;

  // xd__above_mbmi
  char *xd__above_mbmi__bsize;      // str
  char *xd__above_mbmi__partition;  // str
  char *xd__above_mbmi__mode;       // str
  char *xd__above_mbmi__uv_mode;    // str
  int xd__above_mbmi__current_qindex;
  char *xd__above_mbmi__motion_mode;  // str
  int xd__above_mbmi__num_proj_ref;
  int xd__above_mbmi__overlappable_neighbors;
  char *xd__above_mbmi__interintra_mode;  // str
  unsigned char xd__above_mbmi__interintra_wedge_index;
  unsigned char xd__above_mbmi__cfl_alpha_signs;
  int xd__above_mbmi__cfl_alpha_idx;
  int xd__above_mbmi__skip_txfm;
  int xd__above_mbmi__tx_size;
  int xd__above_mbmi__ref_mv_idx;
  int xd__above_mbmi__skip_mode;
  int xd__above_mbmi__use_intrabc;
  int xd__above_mbmi__comp_group_idx;
  int xd__above_mbmi__compound_idx;
  int xd__above_mbmi__use_wedge_interintra;
  unsigned char xd__above_mbmi__cdef_strength;

  // xd__above_mbmi->interp_filter
  int xd__above_mbmi__interp_filters__as_filters__y_filter;
  int xd__above_mbmi__interp_filters__as_filters__x_filter;

  // xd__above_mbmi->mv
  int xd__above_mbmi__mv__0__as_mv__row;
  int xd__above_mbmi__mv__1__as_mv__col;

  // rd_stats
  int rd_stats__rate;
  int rd_stats__zero_rate;
  long long int rd_stats__dist;
  long long int rd_stats__rdcost;
  long long int rd_stats__sse;
  int rd_stats__skip_txfm;

  // rd_stats_luma
  int rd_stats_luma__rate;
  int rd_stats_luma__zero_rate;
  long long int rd_stats_luma__dist;
  long long int rd_stats_luma__rdcost;
  long long int rd_stats_luma__sse;
  int rd_stats_luma__skip_txfm;

  // cpi
  int cpi__skip_tpl_setup_stats;
  double cpi__tpl_rdmult_scaling_factors;
  int cpi__rt_reduce_num_ref_buffers;
  double cpi__framerate;
  int cpi__ref_frame_flags;
  int cpi__speed;
  int cpi__all_one_sided_refs;
  unsigned char cpi__gf_frame_index;
  int cpi__droppable;
  int cpi__data_alloc_width;
  int cpi__data_alloc_height;
  int cpi__initial_mbs;
  int cpi__frame_size_related_setup_done;
  int cpi__last_coded_width;
  int cpi__last_coded_height;
  int cpi__num_frame_recode;
  int cpi__intrabc_used;
  int cpi__prune_ref_frame_mask;
  int cpi__use_screen_content_tools;
  int cpi__is_screen_content_type;
  int cpi__frame_header_count;
  int cpi__deltaq_used;
  char *cpi__last_frame_type;  // str
  int cpi__num_tg;
  char *cpi__superres_mode;  // str
  char *cpi__fp_block_size;  // str
  int cpi__sb_counter;
  int cpi__ref_refresh_index;
  int cpi__refresh_idx_available;
  int cpi__ref_idx_to_skip;
  int cpi__do_frame_data_update;
  double cpi__ext_rate_scale;
  char *cpi__weber_bsize;  // str
  long long int cpi__norm_wiener_variance;
  int cpi__is_dropped_frame;
  unsigned long long cpi__rec_sse;
  int cpi__frames_since_last_update;
  int cpi__palette_pixel_num;
  int cpi__scaled_last_source_available;

  // cpi->ppi
  long long cpi__ppi__ts_start_last_show_frame;
  long long cpi__ppi__ts_end_last_show_frame;
  int cpi__ppi__num_fp_contexts;
  int cpi__ppi__filter_level__0;
  int cpi__ppi__filter_level__1;
  int cpi__ppi__filter_level_u;
  int cpi__ppi__filter_level_v;
  int cpi__ppi__seq_params_locked;
  int cpi__ppi__internal_altref_allowed;
  int cpi__ppi__show_existing_alt_ref;
  int cpi__ppi__lap_enabled;
  int cpi__ppi__b_calculate_psnr;
  int cpi__ppi__frames_left;
  int cpi__ppi__use_svc;
  int cpi__ppi__buffer_removal_time_present;
  unsigned int cpi__ppi__number_temporal_layers;
  unsigned int cpi__ppi__number_spatial_layers;
  double cpi__ppi__tpl_sb_rdmult_scaling_factors;

  // cpi->ppi->gf_group
  int cpi__ppi__gf_group__max_layer_depth;
  int cpi__ppi__gf_group__max_layer_depth_allowed;

  // cpi->ppi->gf_state
  int cpi__ppi__gf_state__arf_gf_boost_lst;

  // cpi->ppi->twopass
  unsigned int cpi__ppi__twopass__section_intra_rating;
  int cpi__ppi__twopass__first_pass_done;
  long long cpi__ppi__twopass__bits_left;
  double cpi__ppi__twopass__modified_error_min;
  double cpi__ppi__twopass__modified_error_max;
  double cpi__ppi__twopass__modified_error_left;
  long long cpi__ppi__twopass__kf_group_bits;
  double cpi__ppi__twopass__kf_group_error_left;
  double cpi__ppi__twopass__bpm_factor;
  int cpi__ppi__twopass__rolling_arf_group_target_bits;
  int cpi__ppi__twopass__rolling_arf_group_actual_bits;
  int cpi__ppi__twopass__sr_update_lag;
  int cpi__ppi__twopass__kf_zeromotion_pct;
  int cpi__ppi__twopass__last_kfgroup_zeromotion_pct;
  int cpi__ppi__twopass__extend_minq;
  int cpi__ppi__twopass__extend_maxq;

  // cpi->ppi->twopass->firstpass_info->total_stats
  double cpi__ppi__twopass__firstpass_info__total_stats__frame;
  double cpi__ppi__twopass__firstpass_info__total_stats__weight;
  double cpi__ppi__twopass__firstpass_info__total_stats__intra_error;
  double
      cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy;
  double cpi__ppi__twopass__firstpass_info__total_stats__coded_error;
  double cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error;
  double cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter;
  double cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion;
  double cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref;
  double cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral;
  double cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct;
  double cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows;
  double cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_cols;
  double cpi__ppi__twopass__firstpass_info__total_stats__MVr;
  double cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs;
  double cpi__ppi__twopass__firstpass_info__total_stats__MVc;
  double cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs;
  double cpi__ppi__twopass__firstpass_info__total_stats__MVrv;
  double cpi__ppi__twopass__firstpass_info__total_stats__MVcv;
  double cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count;
  double cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count;
  double cpi__ppi__twopass__firstpass_info__total_stats__duration;
  double cpi__ppi__twopass__firstpass_info__total_stats__count;
  double cpi__ppi__twopass__firstpass_info__total_stats__raw_error_stdev;
  long long cpi__ppi__twopass__firstpass_info__total_stats__is_flash;
  double cpi__ppi__twopass__firstpass_info__total_stats__noise_var;
  double cpi__ppi__twopass__firstpass_info__total_stats__cor_coeff;
  double cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error;
  double cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error;

  // cpi->ppi->p_rc
  long long cpi__ppi__p_rc__gf_group_bits;
  int cpi__ppi__p_rc__kf_boost;
  int cpi__ppi__p_rc__gfu_boost;
  int cpi__ppi__p_rc__cur_gf_index;
  int cpi__ppi__p_rc__num_regions;
  int cpi__ppi__p_rc__regions_offset;
  int cpi__ppi__p_rc__frames_till_regions_update;
  int cpi__ppi__p_rc__baseline_gf_interval;
  int cpi__ppi__p_rc__constrained_gf_group;
  int cpi__ppi__p_rc__this_key_frame_forced;
  int cpi__ppi__p_rc__next_key_frame_forced;
  long long cpi__ppi__p_rc__starting_buffer_level;
  long long cpi__ppi__p_rc__optimal_buffer_level;
  long long cpi__ppi__p_rc__maximum_buffer_size;
  int cpi__ppi__p_rc__arf_q;
  float cpi__ppi__p_rc__arf_boost_factor;
  int cpi__ppi__p_rc__base_layer_qp;
  int cpi__ppi__p_rc__num_stats_used_for_kf_boost;
  int cpi__ppi__p_rc__num_stats_used_for_gfu_boost;
  int cpi__ppi__p_rc__num_stats_required_for_gfu_boost;
  int cpi__ppi__p_rc__enable_scenecut_detection;
  int cpi__ppi__p_rc__use_arf_in_this_kf_group;
  int cpi__ppi__p_rc__ni_frames;
  double cpi__ppi__p_rc__tot_q;
  int cpi__ppi__p_rc__last_kf_qindex;
  int cpi__ppi__p_rc__last_boosted_qindex;
  double cpi__ppi__p_rc__avg_q;
  long long cpi__ppi__p_rc__total_actual_bits;
  long long cpi__ppi__p_rc__total_target_bits;
  long long cpi__ppi__p_rc__buffer_level;
  int cpi__ppi__p_rc__rate_error_estimate;
  long long cpi__ppi__p_rc__vbr_bits_off_target;
  long long cpi__ppi__p_rc__vbr_bits_off_target_fast;
  long long cpi__ppi__p_rc__bits_off_target;
  int cpi__ppi__p_rc__rolling_target_bits;
  int cpi__ppi__p_rc__rolling_actual_bits;

  // cpi->ppi->tf_info
  int cpi__ppi__tf_info__is_temporal_filter_on;
  long long cpi__ppi__tf_info__frame_diff__0__sum;
  long long cpi__ppi__tf_info__frame_diff__0__sse;
  long long cpi__ppi__tf_info__frame_diff__1__sum;
  long long cpi__ppi__tf_info__frame_diff__1__sse;
  int cpi__ppi__tf_info__tf_buf_gf_index__0;
  int cpi__ppi__tf_info__tf_buf_gf_index__1;
  int cpi__ppi__tf_info__tf_buf_display_index_offset__0;
  int cpi__ppi__tf_info__tf_buf_display_index_offset__1;
  int cpi__ppi__tf_info__tf_buf_valid__0;
  int cpi__ppi__tf_info__tf_buf_valid__1;

  // cpi->ppi->seq_params
  int cpi__ppi__seq_params__num_bits_width;
  int cpi__ppi__seq_params__num_bits_height;
  int cpi__ppi__seq_params__max_frame_width;
  int cpi__ppi__seq_params__max_frame_height;
  int cpi__ppi__seq_params__frame_id_numbers_present_flag;
  int cpi__ppi__seq_params__frame_id_length;
  int cpi__ppi__seq_params__delta_frame_id_length;
  char *cpi__ppi__seq_params__sb_size;  // str
  int cpi__ppi__seq_params__mib_size;
  int cpi__ppi__seq_params__mib_size_log2;
  char *cpi__ppi__seq_params__force_screen_content_tools;  // str
  int cpi__ppi__seq_params__still_picture;
  int cpi__ppi__seq_params__reduced_still_picture_hdr;
  char *cpi__ppi__seq_params__force_integer_mv;  // str
  int cpi__ppi__seq_params__enable_filter_intra;
  int cpi__ppi__seq_params__enable_intra_edge_filter;
  int cpi__ppi__seq_params__enable_interintra_compound;
  int cpi__ppi__seq_params__enable_masked_compound;
  int cpi__ppi__seq_params__enable_dual_filter;
  int cpi__ppi__seq_params__enable_warped_motion;
  int cpi__ppi__seq_params__enable_superres;
  int cpi__ppi__seq_params__enable_cdef;
  int cpi__ppi__seq_params__enable_restoration;
  char *cpi__ppi__seq_params__profile;  // str
  int cpi__ppi__seq_params__bit_depth;
  int cpi__ppi__seq_params__use_highbitdepth;
  int cpi__ppi__seq_params__monochrome;
  char *cpi__ppi__seq_params__color_primaries;           // str
  char *cpi__ppi__seq_params__transfer_characteristics;  // str
  char *cpi__ppi__seq_params__matrix_coefficients;       // str
  int cpi__ppi__seq_params__color_range;
  int cpi__ppi__seq_params__subsampling_x;
  int cpi__ppi__seq_params__subsampling_y;
  char *cpi__ppi__seq_params__chroma_sample_position;  // str
  int cpi__ppi__seq_params__separate_uv_delta_q;
  int cpi__ppi__seq_params__film_grain_params_present;
  int cpi__ppi__seq_params__operating_points_cnt_minus_1;
  int cpi__ppi__seq_params__has_nonzero_operating_point_idc;

  // cpi->ppi->tpl_data
  int cpi__ppi__tpl_data__ready;
  int cpi__ppi__tpl_data__tpl_stats_block_mis_log2;
  int cpi__ppi__tpl_data__tpl_bsize_1d;
  int cpi__ppi__tpl_data__frame_idx;
  int cpi__ppi__tpl_data__border_in_pixels;
  double cpi__ppi__tpl_data__r0_adjust_factor;

  // cpi->ppi->tpl_data->tpl_frame
  int cpi__ppi__tpl_data__tpl_frame__is_valid;
  int cpi__ppi__tpl_data__tpl_frame__stride;
  int cpi__ppi__tpl_data__tpl_frame__width;
  int cpi__ppi__tpl_data__tpl_frame__height;
  int cpi__ppi__tpl_data__tpl_frame__mi_rows;
  int cpi__ppi__tpl_data__tpl_frame__mi_cols;
  int cpi__ppi__tpl_data__tpl_frame__base_rdmult;
  unsigned int cpi__ppi__tpl_data__tpl_frame__frame_display_index;
  int cpi__ppi__tpl_data__tpl_frame__use_pred_sad;

  // cpi->ppi->tpl_data->tpl_frame->tpl_stats_ptr
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_sse;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_dist;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate;
  long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist;
  int cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost;
  int cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost;
  int cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate;
  int cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate;
  int cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_rate;

  // cpi->ppi->tpl_data->sf
  int cpi__ppi__tpl_data__sf__x_scale_fp;
  int cpi__ppi__tpl_data__sf__y_scale_fp;
  int cpi__ppi__tpl_data__sf__x_step_q4;
  int cpi__ppi__tpl_data__sf__y_step_q4;

  // cpi->common
  int cpi__common__width;
  int cpi__common__height;
  int cpi__common__render_width;
  int cpi__common__render_height;
  int cpi__common__superres_upscaled_width;
  int cpi__common__superres_upscaled_height;
  int cpi__common__superres_scale_denominator;
  unsigned int cpi__common__frame_presentation_time;
  int cpi__common__show_frame;
  int cpi__common__showable_frame;
  int cpi__common__show_existing_frame;

  // cpi->common->current_frame
  char *cpi__common__current_frame__frame_type;      // str
  char *cpi__common__current_frame__reference_mode;  // str
  unsigned int cpi__common__current_frame__order_hint;
  unsigned int cpi__common__current_frame__display_order_hint;
  unsigned int cpi__common__current_frame__pyramid_level;
  unsigned int cpi__common__current_frame__frame_number;
  int cpi__common__current_frame__refresh_frame_flags;
  int cpi__common__current_frame__frame_refs_short_signaling;

  // cpi->common->sf_identity
  int cpi__common__sf_identity__x_scale_fp;
  int cpi__common__sf_identity__y_scale_fp;
  int cpi__common__sf_identity__x_step_q4;
  int cpi__common__sf_identity__y_step_q4;

  // cpi->common->features
  int cpi__common__features__disable_cdf_update;
  int cpi__common__features__allow_high_precision_mv;
  int cpi__common__features__cur_frame_force_integer_mv;
  int cpi__common__features__allow_screen_content_tools;
  int cpi__common__features__allow_intrabc;
  int cpi__common__features__allow_warped_motion;
  int cpi__common__features__allow_ref_frame_mvs;
  int cpi__common__features__coded_lossless;
  int cpi__common__features__all_lossless;
  int cpi__common__features__reduced_tx_set_used;
  int cpi__common__features__error_resilient_mode;
  int cpi__common__features__switchable_motion_mode;
  char *cpi__common__features__tx_mode;        // str
  char *cpi__common__features__interp_filter;  // str
  int cpi__common__features__primary_ref_frame;
  int cpi__common__features__byte_alignment;
  char *cpi__common__features__refresh_frame_context;  // str

  // cpi->common->mi_params
  int cpi__common__mi_params__mb_rows;
  int cpi__common__mi_params__mb_cols;
  int cpi__common__mi_params__MBs;
  int cpi__common__mi_params__mi_rows;
  int cpi__common__mi_params__mi_cols;
  int cpi__common__mi_params__mi_stride;

  // cpi->oxcf
  float cpi__oxcf__noise_level;
  int cpi__oxcf__noise_block_size;
  int cpi__oxcf__enable_dnl_denoising;
  unsigned int cpi__oxcf__tier_mask;
  int cpi__oxcf__border_in_pixels;
  int cpi__oxcf__max_threads;
  int cpi__oxcf__speed;
  char *cpi__oxcf__profile;  // str
  char *cpi__oxcf__pass;     // str
  int cpi__oxcf__passes;
  char *cpi__oxcf__mode;  // str
  int cpi__oxcf__use_highbitdepth;
  int cpi__oxcf__save_as_annexb;

  // cpi->oxcf->input_cfg
  double cpi__oxcf__input_cfg__init_framerate;
  unsigned int cpi__oxcf__input_cfg__input_bit_depth;
  unsigned int cpi__oxcf__input_cfg__limit;
  unsigned int cpi__oxcf__input_cfg__chroma_subsampling_x;
  unsigned int cpi__oxcf__input_cfg__chroma_subsampling_y;

  // cpi->oxcf->algo_cfg
  int cpi__oxcf__algo_cfg__sharpness;
  char *cpi__oxcf__algo_cfg__disable_trellis_quant;  // str
  int cpi__oxcf__algo_cfg__arnr_max_frames;
  int cpi__oxcf__algo_cfg__arnr_strength;
  char *cpi__oxcf__algo_cfg__cdf_update_mode;  // str
  int cpi__oxcf__algo_cfg__enable_tpl_model;
  int cpi__oxcf__algo_cfg__enable_overlay;
  char *cpi__oxcf__algo_cfg__loopfilter_control;  // str
  int cpi__oxcf__algo_cfg__skip_postproc_filtering;

  // cpi->oxcf->rc_cfg
  long long cpi__oxcf__rc_cfg__starting_buffer_level_ms;
  long long cpi__oxcf__rc_cfg__optimal_buffer_level_ms;
  long long cpi__oxcf__rc_cfg__maximum_buffer_size_ms;
  long long cpi__oxcf__rc_cfg__target_bandwidth;
  unsigned int cpi__oxcf__rc_cfg__vbr_corpus_complexity_lap;
  unsigned int cpi__oxcf__rc_cfg__max_intra_bitrate_pct;
  unsigned int cpi__oxcf__rc_cfg__max_inter_bitrate_pct;
  unsigned int cpi__oxcf__rc_cfg__gf_cbr_boost_pct;
  unsigned int cpi__oxcf__rc_cfg__min_cr;
  int cpi__oxcf__rc_cfg__drop_frames_water_mark;
  int cpi__oxcf__rc_cfg__under_shoot_pct;
  int cpi__oxcf__rc_cfg__over_shoot_pct;
  int cpi__oxcf__rc_cfg__worst_allowed_q;
  int cpi__oxcf__rc_cfg__best_allowed_q;
  int cpi__oxcf__rc_cfg__cq_level;
  char *cpi__oxcf__rc_cfg__mode;  // str
  int cpi__oxcf__rc_cfg__vbrbias;
  int cpi__oxcf__rc_cfg__vbrmin_section;
  int cpi__oxcf__rc_cfg__vbrmax_section;

  // cpi->oxcf->q_cfg
  int cpi__oxcf__q_cfg__use_fixed_qp_offsets;
  int cpi__oxcf__q_cfg__qm_minlevel;
  int cpi__oxcf__q_cfg__qm_maxlevel;
  int cpi__oxcf__q_cfg__quant_b_adapt;
  char *cpi__oxcf__q_cfg__aq_mode;          // str
  char *cpi__oxcf__q_cfg__deltaq_mode;      // str
  int cpi__oxcf__q_cfg__deltaq_strength;
  int cpi__oxcf__q_cfg__enable_chroma_deltaq;
  int cpi__oxcf__q_cfg__enable_hdr_deltaq;
  int cpi__oxcf__q_cfg__using_qm;

  // cpi->tf_ctx
  int cpi__tf_ctx__num_frames;
  double cpi__tf_ctx__noise_levels__0;
  double cpi__tf_ctx__noise_levels__1;
  double cpi__tf_ctx__noise_levels__2;
  int cpi__tf_ctx__num_pels;
  int cpi__tf_ctx__mb_rows;
  int cpi__tf_ctx__mb_cols;
  int cpi__tf_ctx__q_factor;

  // cpi->rd
  int cpi__rd__rdmult;
  double cpi__rd__r0;

  // cpi->mv_search_params
  int cpi__mv_search_params__max_mv_magnitude;
  int cpi__mv_search_params__mv_step_param;

  // cpi->frame_info
  int cpi__frame_info__frame_width;
  int cpi__frame_info__frame_height;
  int cpi__frame_info__mi_rows;
  int cpi__frame_info__mi_cols;
  int cpi__frame_info__mb_rows;
  int cpi__frame_info__mb_cols;
  int cpi__frame_info__num_mbs;
  int cpi__frame_info__bit_depth;
  int cpi__frame_info__subsampling_x;
  int cpi__frame_info__subsampling_y;

  // cpi->interp_search_flags
  int cpi__interp_search_flags__default_interp_skip_flags;
  int cpi__interp_search_flags__interp_filter_search_mask;

  // cpi->twopass_frame
  double cpi__twopass_frame__mb_av_energy;
  char *cpi__twopass_frame__fr_content_type;  // str
  double cpi__twopass_frame__frame_avg_haar_energy;

  // cpi->twopass_frame->stats_in
  double cpi__twopass_frame__stats_in__frame;
  double cpi__twopass_frame__stats_in__weight;
  double cpi__twopass_frame__stats_in__intra_error;
  double cpi__twopass_frame__stats_in__frame_avg_wavelet_energy;
  double cpi__twopass_frame__stats_in__coded_error;
  double cpi__twopass_frame__stats_in__sr_coded_error;
  double cpi__twopass_frame__stats_in__pcnt_inter;
  double cpi__twopass_frame__stats_in__pcnt_motion;
  double cpi__twopass_frame__stats_in__pcnt_second_ref;
  double cpi__twopass_frame__stats_in__pcnt_neutral;
  double cpi__twopass_frame__stats_in__intra_skip_pct;
  double cpi__twopass_frame__stats_in__inactive_zone_rows;
  double cpi__twopass_frame__stats_in__inactive_zone_cols;
  double cpi__twopass_frame__stats_in__MVr;
  double cpi__twopass_frame__stats_in__mvr_abs;
  double cpi__twopass_frame__stats_in__MVc;
  double cpi__twopass_frame__stats_in__mvc_abs;
  double cpi__twopass_frame__stats_in__MVrv;
  double cpi__twopass_frame__stats_in__MVcv;
  double cpi__twopass_frame__stats_in__mv_in_out_count;
  double cpi__twopass_frame__stats_in__new_mv_count;
  double cpi__twopass_frame__stats_in__duration;
  double cpi__twopass_frame__stats_in__count;
  double cpi__twopass_frame__stats_in__raw_error_stdev;
  long long cpi__twopass_frame__stats_in__is_flash;
  double cpi__twopass_frame__stats_in__noise_var;
  double cpi__twopass_frame__stats_in__cor_coeff;
  double cpi__twopass_frame__stats_in__log_intra_error;
  double cpi__twopass_frame__stats_in__log_coded_error;

  int prediction;
};

char *block_size(int i);

char *plane_type(int i);

char *partition_type(int i);

char *prediction_mode(int i);

char *uv_prediction_mode(int i);

char *motion_mode(int i);

char *interintra_mode(int i);

char *frame_type(int i);

char *superres_mode(int i);

char *screen_content_tool(int i);

char *force_integer_mv(int i);

char *bitstream_profile(int i);

char *color_primary(int i);

char *transfer_characteristic(int i);

char *matrix_coefficient(int i);

char *chroma_sample_position(int i);

char *reference_mode(int i);

char *tx_mode(int i);

char *interp_filter(int i);

char *refresh_frame_context_mode(int i);

char *enc_pass(int i);

char *mode(int i);

char *disable_trellis_quant(int i);

char *cdf_update_mode(int i);

char *loopfilter_control(int i);

char *rc_mode(int i);

char *aq_mode(int i);

char *deltaq_mode(int i);

char *frame_content_type(int i);

void collect_init(struct features_interp_filters *features);

void collect_finalize(struct features_interp_filters *features);

void open_file(const char *fn);

#endif  // AOM_FEATURES_COLLECT_H
