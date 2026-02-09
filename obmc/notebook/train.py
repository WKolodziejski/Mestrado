import sys

import numpy as np
import pandas as pd
from catboost import CatBoostClassifier
from catboost import EShapCalcType, EFeaturesSelectionAlgorithm
from catboost import Pool
from sklearn.feature_selection import mutual_info_classif
from sklearn.metrics import classification_report
from sklearn.model_selection import train_test_split, StratifiedKFold, GridSearchCV


class Tee:
    def __init__(self, *files):
        self.files = files

    def write(self, data):
        for f in self.files:
            f.write(data)

    def flush(self):
        for f in self.files:
            f.flush()


log_file = open("train.log", "w")
sys.stdout = Tee(sys.stdout, log_file)
sys.stderr = Tee(sys.stderr, log_file)

dtypes = {
    'bsize': 'string',
    'ref_best_rd': 'int64',
    'ref_skip_rd': 'int64',
    'best_est_rd': 'int64',
    'do_tx_search': 'int',
    'rate_mv': 'int',
    'num_planes': 'int64',
    'is_comp_pred': 'bool',
    'rate2_nocoeff': 'int',
    'interintra_allowed': 'int',
    'txfm_rd_gate_level': 'int',
    'update_type': 'string',
    'x__qindex': 'int',
    'x__delta_qindex': 'int',
    'x__rdmult_delta_qindex': 'int',
    'x__rdmult_cur_qindex': 'int',
    'x__rdmult': 'int',
    'x__intra_sb_rdmult_modifier': 'int',
    'x__rb': 'float',
    'x__mb_energy': 'int',
    'x__sb_energy_level': 'int',
    'x__errorperbit': 'int',
    'x__sadperbit': 'int',
    'x__seg_skip_block': 'int',
    'x__actual_num_seg1_blocks': 'int',
    'x__actual_num_seg2_blocks': 'int',
    'x__cnt_zeromv': 'int',
    'x__force_zeromv_skip_for_sb': 'int',
    'x__force_zeromv_skip_for_blk': 'int',
    'x__nonrd_prune_ref_frame_search': 'int',
    'x__must_find_valid_partition': 'int',
    'x__skip_mode': 'int',
    'x__winner_mode_count': 'int',
    'x__recalc_luma_mc_data': 'int',
    'x__reuse_inter_pred': 'int',
    'x__source_variance': 'int',
    'x__block_is_zero_sad': 'int',
    'x__sb_me_partition': 'int',
    'x__sb_me_block': 'int',
    'x__sb_force_fixed_part': 'int',
    'x__try_merge_partition': 'int',
    'x__palette_pixels': 'int',
    'x__plane__0__src_diff': 'int',
    'x__plane__0__dqcoeff': 'int',
    'x__plane__0__qcoeff': 'int',
    'x__plane__0__coeff': 'int',
    'x__plane__0__eobs': 'int',
    'x__plane__0__quant_fp_QTX': 'int',
    'x__plane__0__round_fp_QTX': 'int',
    'x__plane__0__quant_QTX': 'int',
    'x__plane__0__round_QTX': 'int',
    'x__plane__0__quant_shift_QTX': 'int',
    'x__plane__0__zbin_QTX': 'int',
    'x__plane__0__dequant_QTX': 'int',
    'xd__mi_row': 'int',
    'xd__mi_col': 'int',
    'xd__mi_stride': 'int',
    'xd__is_chroma_ref': 'int',
    'xd__up_available': 'int',
    'xd__left_available': 'int',
    'xd__chroma_up_available': 'int',
    'xd__chroma_left_available': 'int',
    'xd__tx_type_map_stride': 'int',
    'xd__mb_to_left_edge': 'int',
    'xd__mb_to_right_edge': 'int',
    'xd__mb_to_top_edge': 'int',
    'xd__mb_to_bottom_edge': 'int',
    'xd__width': 'int',
    'xd__height': 'int',
    'xd__is_last_vertical_rect': 'int',
    'xd__is_first_horizontal_rect': 'int',
    'xd__bd': 'int',
    'xd__current_base_qindex': 'int',
    'xd__cur_frame_force_integer_mv': 'int',
    'xd__delta_lf_from_base': 'int',
    'xd__block_ref_scale_factors__0__x_scale_fp': 'int',
    'xd__block_ref_scale_factors__0__y_scale_fp': 'int',
    'xd__block_ref_scale_factors__0__x_step_q4': 'int',
    'xd__block_ref_scale_factors__0__y_step_q4': 'int',
    'xd__block_ref_scale_factors__1__x_scale_fp': 'int',
    'xd__block_ref_scale_factors__1__y_scale_fp': 'int',
    'xd__block_ref_scale_factors__1__x_step_q4': 'int',
    'xd__block_ref_scale_factors__1__y_step_q4': 'int',
    'xd__plane__0__plane_type': 'string',
    'xd__plane__0__subsampling_x': 'int',
    'xd__plane__0__width': 'int',
    'xd__plane__0__height': 'int',
    'xd__plane__1__plane_type': 'string',
    'xd__plane__1__subsampling_x': 'int',
    'xd__plane__1__width': 'int',
    'xd__plane__1__height': 'int',
    'xd__plane__2__plane_type': 'string',
    'xd__plane__2__subsampling_x': 'int',
    'xd__plane__2__width': 'int',
    'xd__plane__2__height': 'int',
    'xd__tile__mi_row_start': 'int',
    'xd__tile__mi_row_end': 'int',
    'xd__tile__mi_col_start': 'int',
    'xd__tile__mi_col_end': 'int',
    'xd__tile__tile_row': 'int',
    'xd__tile__tile_col': 'int',
    'mbmi__bsize': 'string',
    'mbmi__partition': 'string',
    'mbmi__mode': 'string',
    'mbmi__uv_mode': 'string',
    'mbmi__current_qindex': 'int',
    'mbmi__motion_mode': 'string',
    'mbmi__num_proj_ref': 'int',
    'mbmi__overlappable_neighbors': 'int',
    'mbmi__interintra_mode': 'string',
    'mbmi__interintra_wedge_index': 'int',
    'mbmi__cfl_alpha_signs': 'int',
    'mbmi__cfl_alpha_idx': 'int',
    'mbmi__skip_txfm': 'int',
    'mbmi__tx_size': 'int',
    'mbmi__ref_mv_idx': 'int',
    'mbmi__skip_mode': 'int',
    'mbmi__use_intrabc': 'int',
    'mbmi__comp_group_idx': 'int',
    'mbmi__compound_idx': 'int',
    'mbmi__use_wedge_interintra': 'int',
    'mbmi__cdef_strength': 'int',
    'mbmi__interp_filters__as_filters__y_filter': 'string',
    'mbmi__interp_filters__as_filters__x_filter': 'string',
    'mbmi__mv__0__as_mv__row': 'int',
    'mbmi__mv__1__as_mv__col': 'int',
    'xd__left_mbmi__bsize': 'string',
    'xd__left_mbmi__partition': 'string',
    'xd__left_mbmi__mode': 'string',
    'xd__left_mbmi__uv_mode': 'string',
    'xd__left_mbmi__current_qindex': 'int',
    'xd__left_mbmi__motion_mode': 'string',
    'xd__left_mbmi__num_proj_ref': 'int',
    'xd__left_mbmi__overlappable_neighbors': 'int',
    'xd__left_mbmi__interintra_mode': 'string',
    'xd__left_mbmi__interintra_wedge_index': 'int',
    'xd__left_mbmi__cfl_alpha_signs': 'int',
    'xd__left_mbmi__cfl_alpha_idx': 'int',
    'xd__left_mbmi__skip_txfm': 'int',
    'xd__left_mbmi__tx_size': 'int',
    'xd__left_mbmi__ref_mv_idx': 'int',
    'xd__left_mbmi__skip_mode': 'int',
    'xd__left_mbmi__use_intrabc': 'int',
    'xd__left_mbmi__comp_group_idx': 'int',
    'xd__left_mbmi__compound_idx': 'int',
    'xd__left_mbmi__use_wedge_interintra': 'int',
    'xd__left_mbmi__cdef_strength': 'int',
    'xd__left_mbmi__interp_filters__as_filters__y_filter': 'string',
    'xd__left_mbmi__interp_filters__as_filters__x_filter': 'string',
    'xd__left_mbmi__mv__0__as_mv__row': 'int',
    'xd__left_mbmi__mv__1__as_mv__col': 'int',
    'xd__above_mbmi__bsize': 'string',
    'xd__above_mbmi__partition': 'string',
    'xd__above_mbmi__mode': 'string',
    'xd__above_mbmi__uv_mode': 'string',
    'xd__above_mbmi__current_qindex': 'int',
    'xd__above_mbmi__motion_mode': 'string',
    'xd__above_mbmi__num_proj_ref': 'int',
    'xd__above_mbmi__overlappable_neighbors': 'int',
    'xd__above_mbmi__interintra_mode': 'string',
    'xd__above_mbmi__interintra_wedge_index': 'int',
    'xd__above_mbmi__cfl_alpha_signs': 'int',
    'xd__above_mbmi__cfl_alpha_idx': 'int',
    'xd__above_mbmi__skip_txfm': 'int',
    'xd__above_mbmi__tx_size': 'int',
    'xd__above_mbmi__ref_mv_idx': 'int',
    'xd__above_mbmi__skip_mode': 'int',
    'xd__above_mbmi__use_intrabc': 'int',
    'xd__above_mbmi__comp_group_idx': 'int',
    'xd__above_mbmi__compound_idx': 'int',
    'xd__above_mbmi__use_wedge_interintra': 'int',
    'xd__above_mbmi__cdef_strength': 'int',
    'xd__above_mbmi__interp_filters__as_filters__y_filter': 'string',
    'xd__above_mbmi__interp_filters__as_filters__x_filter': 'string',
    'xd__above_mbmi__mv__0__as_mv__row': 'int',
    'xd__above_mbmi__mv__1__as_mv__col': 'int',
    'rd_stats__rate': 'int',
    'rd_stats__zero_rate': 'int',
    'rd_stats__dist': 'int64',
    'rd_stats__rdcost': 'int64',
    'rd_stats__sse': 'int64',
    'rd_stats__skip_txfm': 'int',
    'rd_stats_y__rate': 'int',
    'rd_stats_y__zero_rate': 'int',
    'rd_stats_y__dist': 'int64',
    'rd_stats_y__rdcost': 'int64',
    'rd_stats_y__sse': 'int64',
    'rd_stats_y__skip_txfm': 'int',
    'rd_stats_uv__rate': 'int',
    'rd_stats_uv__zero_rate': 'int',
    'rd_stats_uv__dist': 'int64',
    'rd_stats_uv__rdcost': 'int64',
    'rd_stats_uv__sse': 'int64',
    'rd_stats_uv__skip_txfm': 'int',
    'cpi__skip_tpl_setup_stats': 'int',
    'cpi__tpl_rdmult_scaling_factors': 'float',
    'cpi__rt_reduce_num_ref_buffers': 'int',
    'cpi__framerate': 'float',
    'cpi__ref_frame_flags': 'int',
    'cpi__speed': 'int',
    'cpi__all_one_sided_refs': 'int',
    'cpi__gf_frame_index': 'int',
    'cpi__droppable': 'int',
    'cpi__data_alloc_width': 'int',
    'cpi__data_alloc_height': 'int',
    'cpi__initial_mbs': 'int',
    'cpi__frame_size_related_setup_done': 'int',
    'cpi__last_coded_width': 'int',
    'cpi__last_coded_height': 'int',
    'cpi__num_frame_recode': 'int',
    'cpi__intrabc_used': 'int',
    'cpi__prune_ref_frame_mask': 'int',
    'cpi__use_screen_content_tools': 'int',
    'cpi__is_screen_content_type': 'int',
    'cpi__frame_header_count': 'int',
    'cpi__deltaq_used': 'int',
    'cpi__last_frame_type': 'string',
    'cpi__num_tg': 'int',
    'cpi__superres_mode': 'string',
    'cpi__fp_block_size': 'string',
    'cpi__sb_counter': 'int',
    'cpi__ref_refresh_index': 'int',
    'cpi__refresh_idx_available': 'int',
    'cpi__ref_idx_to_skip': 'int',
    'cpi__do_frame_data_update': 'int',
    'cpi__ext_rate_scale': 'float',
    'cpi__weber_bsize': 'string',
    'cpi__norm_wiener_variance': 'int64',
    'cpi__is_dropped_frame': 'int',
    'cpi__rec_sse': 'int64',
    'cpi__frames_since_last_update': 'int',
    'cpi__palette_pixel_num': 'int',
    'cpi__scaled_last_source_available': 'int',
    'cpi__ppi__ts_start_last_show_frame': 'int64',
    'cpi__ppi__ts_end_last_show_frame': 'int64',
    'cpi__ppi__num_fp_contexts': 'int',
    'cpi__ppi__filter_level__0': 'int',
    'cpi__ppi__filter_level__1': 'int',
    'cpi__ppi__filter_level_u': 'int',
    'cpi__ppi__filter_level_v': 'int',
    'cpi__ppi__seq_params_locked': 'int',
    'cpi__ppi__internal_altref_allowed': 'int',
    'cpi__ppi__show_existing_alt_ref': 'int',
    'cpi__ppi__lap_enabled': 'int',
    'cpi__ppi__b_calculate_psnr': 'int',
    'cpi__ppi__frames_left': 'int',
    'cpi__ppi__use_svc': 'int',
    'cpi__ppi__buffer_removal_time_present': 'int',
    'cpi__ppi__number_temporal_layers': 'int',
    'cpi__ppi__number_spatial_layers': 'int',
    'cpi__ppi__tpl_sb_rdmult_scaling_factors': 'float',
    'cpi__ppi__gf_group__max_layer_depth': 'int',
    'cpi__ppi__gf_group__max_layer_depth_allowed': 'int',
    'cpi__ppi__gf_state__arf_gf_boost_lst': 'int',
    'cpi__ppi__twopass__section_intra_rating': 'int',
    'cpi__ppi__twopass__first_pass_done': 'int',
    'cpi__ppi__twopass__bits_left': 'int64',
    'cpi__ppi__twopass__modified_error_min': 'float',
    'cpi__ppi__twopass__modified_error_max': 'float',
    'cpi__ppi__twopass__modified_error_left': 'float',
    'cpi__ppi__twopass__kf_group_bits': 'int64',
    'cpi__ppi__twopass__kf_group_error_left': 'float',
    'cpi__ppi__twopass__bpm_factor': 'float',
    'cpi__ppi__twopass__rolling_arf_group_target_bits': 'int',
    'cpi__ppi__twopass__rolling_arf_group_actual_bits': 'int',
    'cpi__ppi__twopass__sr_update_lag': 'int',
    'cpi__ppi__twopass__kf_zeromotion_pct': 'int',
    'cpi__ppi__twopass__last_kfgroup_zeromotion_pct': 'int',
    'cpi__ppi__twopass__extend_minq': 'int',
    'cpi__ppi__twopass__extend_maxq': 'int',
    'cpi__ppi__twopass__firstpass_info__total_stats__frame': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__weight': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__intra_error': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__coded_error': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_cols': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__MVr': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__MVc': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__MVrv': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__MVcv': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__duration': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__count': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__raw_error_stdev': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__is_flash': 'int64',
    'cpi__ppi__twopass__firstpass_info__total_stats__noise_var': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__cor_coeff': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error': 'float',
    'cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error': 'float',
    'cpi__ppi__p_rc__gf_group_bits': 'int64',
    'cpi__ppi__p_rc__kf_boost': 'int',
    'cpi__ppi__p_rc__gfu_boost': 'int',
    'cpi__ppi__p_rc__cur_gf_index': 'int',
    'cpi__ppi__p_rc__num_regions': 'int',
    'cpi__ppi__p_rc__regions_offset': 'int',
    'cpi__ppi__p_rc__frames_till_regions_update': 'int',
    'cpi__ppi__p_rc__baseline_gf_interval': 'int',
    'cpi__ppi__p_rc__constrained_gf_group': 'int',
    'cpi__ppi__p_rc__this_key_frame_forced': 'int',
    'cpi__ppi__p_rc__next_key_frame_forced': 'int',
    'cpi__ppi__p_rc__starting_buffer_level': 'int64',
    'cpi__ppi__p_rc__optimal_buffer_level': 'int64',
    'cpi__ppi__p_rc__maximum_buffer_size': 'int64',
    'cpi__ppi__p_rc__arf_q': 'int',
    'cpi__ppi__p_rc__arf_boost_factor': 'float',
    'cpi__ppi__p_rc__base_layer_qp': 'int',
    'cpi__ppi__p_rc__num_stats_used_for_kf_boost': 'int',
    'cpi__ppi__p_rc__num_stats_used_for_gfu_boost': 'int',
    'cpi__ppi__p_rc__num_stats_required_for_gfu_boost': 'int',
    'cpi__ppi__p_rc__enable_scenecut_detection': 'int',
    'cpi__ppi__p_rc__use_arf_in_this_kf_group': 'int',
    'cpi__ppi__p_rc__ni_frames': 'int',
    'cpi__ppi__p_rc__tot_q': 'float',
    'cpi__ppi__p_rc__last_kf_qindex': 'int',
    'cpi__ppi__p_rc__last_boosted_qindex': 'int',
    'cpi__ppi__p_rc__avg_q': 'float',
    'cpi__ppi__p_rc__total_actual_bits': 'int64',
    'cpi__ppi__p_rc__total_target_bits': 'int64',
    'cpi__ppi__p_rc__buffer_level': 'int64',
    'cpi__ppi__p_rc__rate_error_estimate': 'int',
    'cpi__ppi__p_rc__vbr_bits_off_target': 'int64',
    'cpi__ppi__p_rc__vbr_bits_off_target_fast': 'int64',
    'cpi__ppi__p_rc__bits_off_target': 'int64',
    'cpi__ppi__p_rc__rolling_target_bits': 'int',
    'cpi__ppi__p_rc__rolling_actual_bits': 'int',
    'cpi__ppi__tf_info__is_temporal_filter_on': 'int',
    'cpi__ppi__tf_info__frame_diff__0__sum': 'int64',
    'cpi__ppi__tf_info__frame_diff__0__sse': 'int64',
    'cpi__ppi__tf_info__frame_diff__1__sum': 'int64',
    'cpi__ppi__tf_info__frame_diff__1__sse': 'int64',
    'cpi__ppi__tf_info__tf_buf_gf_index__0': 'int',
    'cpi__ppi__tf_info__tf_buf_gf_index__1': 'int',
    'cpi__ppi__tf_info__tf_buf_display_index_offset__0': 'int',
    'cpi__ppi__tf_info__tf_buf_display_index_offset__1': 'int',
    'cpi__ppi__tf_info__tf_buf_valid__0': 'int',
    'cpi__ppi__tf_info__tf_buf_valid__1': 'int',
    'cpi__ppi__seq_params__num_bits_width': 'int',
    'cpi__ppi__seq_params__num_bits_height': 'int',
    'cpi__ppi__seq_params__max_frame_width': 'int',
    'cpi__ppi__seq_params__max_frame_height': 'int',
    'cpi__ppi__seq_params__frame_id_numbers_present_flag': 'int',
    'cpi__ppi__seq_params__frame_id_length': 'int',
    'cpi__ppi__seq_params__delta_frame_id_length': 'int',
    'cpi__ppi__seq_params__sb_size': 'string',
    'cpi__ppi__seq_params__mib_size': 'int',
    'cpi__ppi__seq_params__mib_size_log2': 'int',
    'cpi__ppi__seq_params__force_screen_content_tools': 'string',
    'cpi__ppi__seq_params__still_picture': 'int',
    'cpi__ppi__seq_params__reduced_still_picture_hdr': 'int',
    'cpi__ppi__seq_params__force_integer_mv': 'string',
    'cpi__ppi__seq_params__enable_filter_intra': 'int',
    'cpi__ppi__seq_params__enable_intra_edge_filter': 'int',
    'cpi__ppi__seq_params__enable_interintra_compound': 'int',
    'cpi__ppi__seq_params__enable_masked_compound': 'int',
    'cpi__ppi__seq_params__enable_dual_filter': 'int',
    'cpi__ppi__seq_params__enable_warped_motion': 'int',
    'cpi__ppi__seq_params__enable_superres': 'int',
    'cpi__ppi__seq_params__enable_cdef': 'int',
    'cpi__ppi__seq_params__enable_restoration': 'int',
    'cpi__ppi__seq_params__profile': 'string',
    'cpi__ppi__seq_params__bit_depth': 'int',
    'cpi__ppi__seq_params__use_highbitdepth': 'int',
    'cpi__ppi__seq_params__monochrome': 'int',
    'cpi__ppi__seq_params__color_primaries': 'string',
    'cpi__ppi__seq_params__transfer_characteristics': 'string',
    'cpi__ppi__seq_params__matrix_coefficients': 'string',
    'cpi__ppi__seq_params__color_range': 'int',
    'cpi__ppi__seq_params__subsampling_x': 'int',
    'cpi__ppi__seq_params__subsampling_y': 'int',
    'cpi__ppi__seq_params__chroma_sample_position': 'string',
    'cpi__ppi__seq_params__separate_uv_delta_q': 'int',
    'cpi__ppi__seq_params__film_grain_params_present': 'int',
    'cpi__ppi__seq_params__operating_points_cnt_minus_1': 'int',
    'cpi__ppi__seq_params__has_nonzero_operating_point_idc': 'int',
    'cpi__ppi__tpl_data__ready': 'int',
    'cpi__ppi__tpl_data__tpl_stats_block_mis_log2': 'int',
    'cpi__ppi__tpl_data__tpl_bsize_1d': 'int',
    'cpi__ppi__tpl_data__frame_idx': 'int',
    'cpi__ppi__tpl_data__border_in_pixels': 'int',
    'cpi__ppi__tpl_data__r0_adjust_factor': 'float',
    'cpi__ppi__tpl_data__tpl_frame__is_valid': 'int',
    'cpi__ppi__tpl_data__tpl_frame__stride': 'int',
    'cpi__ppi__tpl_data__tpl_frame__width': 'int',
    'cpi__ppi__tpl_data__tpl_frame__height': 'int',
    'cpi__ppi__tpl_data__tpl_frame__mi_rows': 'int',
    'cpi__ppi__tpl_data__tpl_frame__mi_cols': 'int',
    'cpi__ppi__tpl_data__tpl_frame__base_rdmult': 'int',
    'cpi__ppi__tpl_data__tpl_frame__frame_display_index': 'int',
    'cpi__ppi__tpl_data__tpl_frame__use_pred_sad': 'int',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_sse': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_dist': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist': 'int64',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost': 'int',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost': 'int',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate': 'int',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate': 'int',
    'cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_rate': 'int',
    'cpi__ppi__tpl_data__sf__x_scale_fp': 'int',
    'cpi__ppi__tpl_data__sf__y_scale_fp': 'int',
    'cpi__ppi__tpl_data__sf__x_step_q4': 'int',
    'cpi__ppi__tpl_data__sf__y_step_q4': 'int',
    'cpi__common__width': 'int',
    'cpi__common__height': 'int',
    'cpi__common__render_width': 'int',
    'cpi__common__render_height': 'int',
    'cpi__common__superres_upscaled_width': 'int',
    'cpi__common__superres_upscaled_height': 'int',
    'cpi__common__superres_scale_denominator': 'int',
    'cpi__common__frame_presentation_time': 'int',
    'cpi__common__show_frame': 'int',
    'cpi__common__showable_frame': 'int',
    'cpi__common__show_existing_frame': 'int',
    'cpi__common__current_frame__frame_type': 'string',
    'cpi__common__current_frame__reference_mode': 'string',
    'cpi__common__current_frame__order_hint': 'int',
    'cpi__common__current_frame__display_order_hint': 'int',
    'cpi__common__current_frame__pyramid_level': 'int',
    'cpi__common__current_frame__frame_number': 'int',
    'cpi__common__current_frame__refresh_frame_flags': 'int',
    'cpi__common__current_frame__frame_refs_short_signaling': 'int',
    'cpi__common__sf_identity__x_scale_fp': 'int',
    'cpi__common__sf_identity__y_scale_fp': 'int',
    'cpi__common__sf_identity__x_step_q4': 'int',
    'cpi__common__sf_identity__y_step_q4': 'int',
    'cpi__common__features__disable_cdf_update': 'int',
    'cpi__common__features__allow_high_precision_mv': 'int',
    'cpi__common__features__cur_frame_force_integer_mv': 'int',
    'cpi__common__features__allow_screen_content_tools': 'int',
    'cpi__common__features__allow_intrabc': 'int',
    'cpi__common__features__allow_warped_motion': 'int',
    'cpi__common__features__allow_ref_frame_mvs': 'int',
    'cpi__common__features__coded_lossless': 'int',
    'cpi__common__features__all_lossless': 'int',
    'cpi__common__features__reduced_tx_set_used': 'int',
    'cpi__common__features__error_resilient_mode': 'int',
    'cpi__common__features__switchable_motion_mode': 'int',
    'cpi__common__features__tx_mode': 'string',
    'cpi__common__features__interp_filter': 'string',
    'cpi__common__features__primary_ref_frame': 'int',
    'cpi__common__features__byte_alignment': 'int',
    'cpi__common__features__refresh_frame_context': 'string',
    'cpi__common__mi_params__mb_rows': 'int',
    'cpi__common__mi_params__mb_cols': 'int',
    'cpi__common__mi_params__MBs': 'int',
    'cpi__common__mi_params__mi_rows': 'int',
    'cpi__common__mi_params__mi_cols': 'int',
    'cpi__common__mi_params__mi_stride': 'int',
    'cpi__oxcf__noise_level': 'float',
    'cpi__oxcf__noise_block_size': 'int',
    'cpi__oxcf__enable_dnl_denoising': 'int',
    'cpi__oxcf__tier_mask': 'int',
    'cpi__oxcf__border_in_pixels': 'int',
    'cpi__oxcf__max_threads': 'int',
    'cpi__oxcf__speed': 'int',
    'cpi__oxcf__profile': 'string',
    'cpi__oxcf__pass': 'string',
    'cpi__oxcf__passes': 'int',
    'cpi__oxcf__mode': 'string',
    'cpi__oxcf__use_highbitdepth': 'int',
    'cpi__oxcf__save_as_annexb': 'int',
    'cpi__oxcf__input_cfg__init_framerate': 'float',
    'cpi__oxcf__input_cfg__input_bit_depth': 'int',
    'cpi__oxcf__input_cfg__limit': 'int',
    'cpi__oxcf__input_cfg__chroma_subsampling_x': 'int',
    'cpi__oxcf__input_cfg__chroma_subsampling_y': 'int',
    'cpi__oxcf__algo_cfg__sharpness': 'int',
    'cpi__oxcf__algo_cfg__disable_trellis_quant': 'string',
    'cpi__oxcf__algo_cfg__arnr_max_frames': 'int',
    'cpi__oxcf__algo_cfg__arnr_strength': 'int',
    'cpi__oxcf__algo_cfg__cdf_update_mode': 'string',
    'cpi__oxcf__algo_cfg__enable_tpl_model': 'int',
    'cpi__oxcf__algo_cfg__enable_overlay': 'int',
    'cpi__oxcf__algo_cfg__loopfilter_control': 'string',
    'cpi__oxcf__algo_cfg__skip_postproc_filtering': 'int',
    'cpi__oxcf__rc_cfg__starting_buffer_level_ms': 'int64',
    'cpi__oxcf__rc_cfg__optimal_buffer_level_ms': 'int64',
    'cpi__oxcf__rc_cfg__maximum_buffer_size_ms': 'int64',
    'cpi__oxcf__rc_cfg__target_bandwidth': 'int64',
    'cpi__oxcf__rc_cfg__vbr_corpus_complexity_lap': 'int',
    'cpi__oxcf__rc_cfg__max_intra_bitrate_pct': 'int',
    'cpi__oxcf__rc_cfg__max_inter_bitrate_pct': 'int',
    'cpi__oxcf__rc_cfg__gf_cbr_boost_pct': 'int',
    'cpi__oxcf__rc_cfg__min_cr': 'int',
    'cpi__oxcf__rc_cfg__drop_frames_water_mark': 'int',
    'cpi__oxcf__rc_cfg__under_shoot_pct': 'int',
    'cpi__oxcf__rc_cfg__over_shoot_pct': 'int',
    'cpi__oxcf__rc_cfg__worst_allowed_q': 'int',
    'cpi__oxcf__rc_cfg__best_allowed_q': 'int',
    'cpi__oxcf__rc_cfg__cq_level': 'int',
    'cpi__oxcf__rc_cfg__mode': 'string',
    'cpi__oxcf__rc_cfg__vbrbias': 'int',
    'cpi__oxcf__rc_cfg__vbrmin_section': 'int',
    'cpi__oxcf__rc_cfg__vbrmax_section': 'int',
    'cpi__oxcf__q_cfg__use_fixed_qp_offsets': 'int',
    'cpi__oxcf__q_cfg__qm_minlevel': 'int',
    'cpi__oxcf__q_cfg__qm_maxlevel': 'int',
    'cpi__oxcf__q_cfg__quant_b_adapt': 'int',
    'cpi__oxcf__q_cfg__aq_mode': 'string',
    'cpi__oxcf__q_cfg__deltaq_mode': 'string',
    'cpi__oxcf__q_cfg__deltaq_strength': 'int',
    'cpi__oxcf__q_cfg__enable_chroma_deltaq': 'int',
    'cpi__oxcf__q_cfg__enable_hdr_deltaq': 'int',
    'cpi__oxcf__q_cfg__using_qm': 'int',
    'cpi__tf_ctx__num_frames': 'int',
    'cpi__tf_ctx__noise_levels__0': 'float',
    'cpi__tf_ctx__noise_levels__1': 'float',
    'cpi__tf_ctx__noise_levels__2': 'float',
    'cpi__tf_ctx__num_pels': 'int',
    'cpi__tf_ctx__mb_rows': 'int',
    'cpi__tf_ctx__mb_cols': 'int',
    'cpi__tf_ctx__q_factor': 'int',
    'cpi__rd__rdmult': 'int',
    'cpi__rd__r0': 'float',
    'cpi__mv_search_params__max_mv_magnitude': 'int',
    'cpi__mv_search_params__mv_step_param': 'int',
    'cpi__frame_info__frame_width': 'int',
    'cpi__frame_info__frame_height': 'int',
    'cpi__frame_info__mi_rows': 'int',
    'cpi__frame_info__mi_cols': 'int',
    'cpi__frame_info__mb_rows': 'int',
    'cpi__frame_info__mb_cols': 'int',
    'cpi__frame_info__num_mbs': 'int',
    'cpi__frame_info__bit_depth': 'int',
    'cpi__frame_info__subsampling_x': 'int',
    'cpi__frame_info__subsampling_y': 'int',
    'cpi__interp_search_flags__default_interp_skip_flags': 'int',
    'cpi__interp_search_flags__interp_filter_search_mask': 'int',
    'cpi__twopass_frame__mb_av_energy': 'float',
    'cpi__twopass_frame__fr_content_type': 'string',
    'cpi__twopass_frame__frame_avg_haar_energy': 'float',
    'cpi__twopass_frame__stats_in__frame': 'float',
    'cpi__twopass_frame__stats_in__weight': 'float',
    'cpi__twopass_frame__stats_in__intra_error': 'float',
    'cpi__twopass_frame__stats_in__frame_avg_wavelet_energy': 'float',
    'cpi__twopass_frame__stats_in__coded_error': 'float',
    'cpi__twopass_frame__stats_in__sr_coded_error': 'float',
    'cpi__twopass_frame__stats_in__pcnt_inter': 'float',
    'cpi__twopass_frame__stats_in__pcnt_motion': 'float',
    'cpi__twopass_frame__stats_in__pcnt_second_ref': 'float',
    'cpi__twopass_frame__stats_in__pcnt_neutral': 'float',
    'cpi__twopass_frame__stats_in__intra_skip_pct': 'float',
    'cpi__twopass_frame__stats_in__inactive_zone_rows': 'float',
    'cpi__twopass_frame__stats_in__inactive_zone_cols': 'float',
    'cpi__twopass_frame__stats_in__MVr': 'float',
    'cpi__twopass_frame__stats_in__mvr_abs': 'float',
    'cpi__twopass_frame__stats_in__MVc': 'float',
    'cpi__twopass_frame__stats_in__mvc_abs': 'float',
    'cpi__twopass_frame__stats_in__MVrv': 'float',
    'cpi__twopass_frame__stats_in__MVcv': 'float',
    'cpi__twopass_frame__stats_in__mv_in_out_count': 'float',
    'cpi__twopass_frame__stats_in__new_mv_count': 'float',
    'cpi__twopass_frame__stats_in__duration': 'float',
    'cpi__twopass_frame__stats_in__count': 'float',
    'cpi__twopass_frame__stats_in__raw_error_stdev': 'float',
    'cpi__twopass_frame__stats_in__is_flash': 'int64',
    'cpi__twopass_frame__stats_in__noise_var': 'float',
    'cpi__twopass_frame__stats_in__cor_coeff': 'float',
    'cpi__twopass_frame__stats_in__log_intra_error': 'float',
    'cpi__twopass_frame__stats_in__log_coded_error': 'float',
    'tile_data__abs_sum_level': 'int64',
    'tile_data__allow_update_cdf': 'bool',
    'tile_data__tile_info__mi_row_start': 'int',
    'tile_data__tile_info__mi_row_end': 'int',
    'tile_data__tile_info__mi_col_start': 'int',
    'tile_data__tile_info__mi_col_end': 'int',
    'tile_data__tile_info__tile_row': 'int',
    'tile_data__tile_info__tile_col': 'int',
    'tile_data__firstpass_top_mv__row': 'int',
    'tile_data__firstpass_top_mv__col': 'int',
    'args__ref_frame_cost': 'int',
    'args__single_comp_cost': 'int',
    'args__skip_ifs': 'int',
    'args__start_mv_cnt': 'int',
    'args__interp_filter_stats_idx': 'int',
    'args__wedge_index': 'int',
    'args__wedge_sign': 'int',
    'args__diffwtd_index': 'int',
    'args__best_pred_sse': 'int',
    'discard': 'bool',
    'prediction': 'bool',
}

features_drop = []


def cross_validation(X, y):
    n_fold = 5
    folds = StratifiedKFold(n_splits=n_fold, shuffle=True, random_state=0)
    scores = []
    models = []

    cat_features_names = X.select_dtypes(exclude=['number']).columns.tolist()
    cat_features = [X.columns.get_loc(col) for col in cat_features_names]

    params = {
        "iterations": 100,
        "depth": 5,
        "learning_rate": 0.05,
        "l2_leaf_reg": 15,
        "model_shrink_rate": 0.1,
        "model_shrink_mode": "Constant",
        "grow_policy": "SymmetricTree",
        "boosting_type": "Plain",
        "leaf_estimation_iterations": 1,
        "leaf_estimation_method": "Gradient",
        "max_bin": 32,
        "verbose": 100,
        "random_seed": 42,
        'loss_function': 'Logloss',
        'eval_metric': 'F1',
        'custom_metric': ['F1', 'Recall', 'Precision', 'AUC', 'Logloss']
    }

    for fold_n, (train_index, valid_index) in enumerate(folds.split(X, y)):
        X_train, X_valid = X.iloc[train_index], X.iloc[valid_index]
        y_train, y_valid = y.values[train_index], y.values[valid_index]

        train_data = Pool(data=X_train,
                          label=y_train,
                          cat_features=cat_features)

        valid_data = Pool(data=X_valid,
                          label=y_valid,
                          cat_features=cat_features)

        model = CatBoostClassifier(**params)
        model.fit(train_data,
                  eval_set=valid_data,
                  use_best_model=True,
                  plot=True)

        print(model.get_best_score())

        score = model.get_best_score()['validation']['F1']
        scores.append(score)
        models.append(model)

        print(model.get_feature_importance(prettified=True))

        y_pred = model.predict(X_valid)
        print(classification_report(y_valid, y_pred))

    print('CV mean: {:.4f}, CV std: {:.4f}'.format(np.mean(scores), np.std(scores)))

    return models


def select_features(X, y, steps: int = 1, num_features_to_select: int = 1):
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.25)

    cat_features = X_train.select_dtypes(exclude=[np.number]).columns.tolist()

    train_data = Pool(data=X_train,
                      label=y_train,
                      cat_features=cat_features)

    test_data = Pool(data=X_test,
                     label=y_test,
                     cat_features=cat_features)
                     
    params = {
        "iterations": 100,
        "depth": 5,
        "learning_rate": 0.05,
        "l2_leaf_reg": 15,
        "model_shrink_rate": 0.1,
        "model_shrink_mode": "Constant",
        "grow_policy": "SymmetricTree",
        "boosting_type": "Plain",
        "leaf_estimation_iterations": 1,
        "leaf_estimation_method": "Gradient",
        "max_bin": 32,
        "verbose": 100,
        "random_seed": 42,
        'loss_function': 'Logloss',
        'eval_metric': 'F1',
        'custom_metric': ['F1', 'Recall', 'Precision', 'AUC', 'Logloss']
    }

    model = CatBoostClassifier(**params)

    return model.select_features(
        train_data,
        eval_set=test_data,
        features_for_select=list(range(train_data.num_col())),
        num_features_to_select=num_features_to_select,
        steps=steps,
        algorithm=EFeaturesSelectionAlgorithm.RecursiveByShapValues,
        shap_calc_type=EShapCalcType.Regular,
        train_final_model=False,
        logging_level='Silent'
    )


def grid_search(X, y):
    cat_features_names = X.select_dtypes(exclude=['number']).columns.tolist()
    cat_features = [X.columns.get_loc(col) for col in cat_features_names]

    params = {
        'cat_features': cat_features,
        'verbose': 200,
        'random_seed': 0,
        'early_stopping_rounds': 200,
    }

    model = CatBoostClassifier(**params)

    param_grid = {
        'iterations': [100, 200, 300, 400],
        'depth': [2, 4, 6, 8, 10, 12],
        'learning_rate': [0.01, 0.1, 0.2, 0.3],
        'l2_leaf_reg': [1, 3, 5, 7]
    }

    grid = GridSearchCV(
        estimator=model,
        param_grid=param_grid,
        cv=5,
        scoring='f1',
        n_jobs=-1
    )

    grid.fit(X, y)

    print("BEST PARAMETERS: ", grid.best_params_)
    print("BEST F1: ", grid.best_score_)

    return grid.best_estimator_


def build_pool(X_train, y_train, X_test, y_test):
    cat_features_names = X_train.select_dtypes(exclude=['number']).columns.tolist()
    cat_features = [X_train.columns.get_loc(col) for col in cat_features_names]

    train_data = Pool(data=X_train,
                      label=y_train,
                      cat_features=cat_features)

    test_data = Pool(data=X_test,
                     label=y_test,
                     cat_features=cat_features)

    return train_data, test_data


def train_model(params, X_train, y_train, X_test, y_test):
    train_data, test_data = build_pool(X_train, y_train, X_test, y_test)

    model = CatBoostClassifier(**params)

    model.fit(train_data,
              eval_set=test_data,
              use_best_model=True)

    return model


def eval_model(model, X_test, y_test):
    y_pred = model.predict(X_test)

    print(model.get_params())
    #print(model.get_best_score()['validation']['F1'])
    print(model.get_feature_importance(prettified=True))
    print(classification_report(y_test, y_pred))


def save_model(model, model_name, X_train, y_train, X_test, y_test):
    train_data, test_data = build_pool(X_train, y_train, X_test, y_test)

    model.save_model(f"{model_name}.cbm", pool=train_data)
    model.save_model(f"{model_name}.cpp", format="cpp", pool=train_data)


def train(dataset_train: str,
          dataset_test: str,
          model_name: str,
          do_mutual_info: bool = True,
          do_select_features: bool = True,
          do_cross_validation: bool = True,
          do_grid_search: bool = False):
    print('LOADING DATASETS')

    df_train = pd.read_csv(dataset_train, dtype=dtypes, low_memory=False)
    df_test = pd.read_csv(dataset_test, dtype=dtypes, low_memory=False)
    
    print('FINDING NAT')
    df_bool = df_train.isnull()
    col_empty = df_bool.any()
    col_empty = col_empty[col_empty].index.tolist()
    print(col_empty)

    df_train.fillna("INVALID", inplace=True)
    df_test.fillna("INVALID", inplace=True)

    df_train = df_train.reindex(columns=sorted(df_train.columns))
    df_test = df_test.reindex(columns=sorted(df_test.columns))

    df_train.drop(features_drop, axis=1, inplace=True)
    df_test.drop(features_drop, axis=1, inplace=True)

    y_train = df_train['discard']
    X_train = df_train.drop(columns='discard', axis=1)

    y_test = df_test['discard']
    X_test = df_test.drop(columns='discard', axis=1)

    params = {
        "iterations": 100,
        "depth": 5,
        "learning_rate": 0.05,
        "l2_leaf_reg": 15,
        "model_shrink_rate": 0.1,
        "model_shrink_mode": "Constant",
        "grow_policy": "SymmetricTree",
        "boosting_type": "Plain",
        "leaf_estimation_iterations": 1,
        "leaf_estimation_method": "Gradient",
        "max_bin": 32,
        "verbose": 100,
        "random_seed": 42,
        'loss_function': 'Logloss',
        'eval_metric': 'F1',
        'custom_metric': ['F1', 'Recall', 'Precision', 'AUC', 'Logloss']
    }

    if do_mutual_info:
        print('MUTUAL INFO')

        cat_features = df_train.select_dtypes(['string']).columns.tolist()

        print(f'{len(cat_features)} CATEGORICAL FEATURES')
        print(cat_features)

        X_num = X_train.drop(cat_features, axis=1)

        mutual_info_num = mutual_info_classif(X_num, y_train)
        mutual_info_num_ser = pd.Series(mutual_info_num, index=X_num.columns)

        num_features = mutual_info_num_ser[mutual_info_num_ser > 0.02].index.tolist()

        print(f'{len(num_features)} NUMERICAL FEATURES')
        print(num_features)

        train_num_df = X_train[num_features]
        train_cat_df = X_train[cat_features]

        test_num_df = X_test[num_features]
        test_cat_df = X_test[cat_features]

        X_train = pd.concat([train_num_df, train_cat_df], axis=1)
        X_test = pd.concat([test_num_df, test_cat_df], axis=1)

    if do_select_features:
        print('SEARCHING NUMBER OF FEATURES')

        summary = select_features(X_train, y_train, 10, 1)

        min_idx = 0
        min_loss = 100000

        for idx, loss in enumerate(summary['loss_graph']['loss_values']):
            if loss <= min_loss:
                min_loss = loss
                min_idx = idx

        count_features = df_train.shape[1] - 1 - min_idx

        print(f'SELECTING {count_features} FEATURES')

        summary = select_features(X_train, y_train, 10, count_features)

        selected_features_names = summary['selected_features_names']

        print(f'FEATURES: {selected_features_names}')

        X_train = X_train[selected_features_names]
        X_test = X_test[selected_features_names]

    if do_cross_validation:
        print(f'CROSS VALIDATING')

        models = cross_validation(X_train, y_train)

        important_features = []

        for model in models:
            for value in model.get_feature_importance(prettified=True).values:
                if value[1] >= 1:  # apenas features com mais de 1% de importância
                    important_features.append(value[0])

        important_features = set(important_features)

        print(f'IMPORTANT FEATURES: {important_features}')

        X_train = X_train[list(important_features)]
        X_test = X_test[list(important_features)]

    if do_grid_search:
        print(f'GRID SEARCHING')

        grid_model = grid_search(X_train, y_train)

        params = grid_model.get_params()

    print(f'TRAINING FIRST MODEL')

    model = train_model(params, X_train, y_train, X_test, y_test)

    eval_model(model, X_test, y_test)

    print(f'DROPING UNUSED FEATURES')

    important_features = []

    for value in model.get_feature_importance(prettified=True).values:
        if value[1] > 0:  # apenas features com mais de 0% de importância
            important_features.append(value[0])

    print(f'IMPORTANT FEATURES: {important_features}')

    X_train = X_train[important_features]
    X_test = X_test[important_features]

    print(f'TRAINING FINAL MODEL')

    model = train_model(params, X_train, y_train, X_test, y_test)

    print(f'MODEL TRAINED')

    eval_model(model, X_test, y_test)

    save_model(model, model_name, X_train, y_train, X_test, y_test)

    print(f'MODEL SAVED')


train(f"datasets/train_obmc.csv",
      f"datasets/test_obmc.csv",
      f"models/obmc")

log_file.close()
