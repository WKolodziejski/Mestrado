/*
 * Copyright (c) 2020, Alliance for Open Media. All rights reserved.
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include "av1/common/pred_common.h"
#include "av1/encoder/interp_search.h"
#include "av1/encoder/model_rd.h"
#include "av1/encoder/rdopt_utils.h"
#include "av1/encoder/reconinter_enc.h"

#include "william/features_collect.h"
#include "william/features_predict.h"
#include "william/features_best_case.h"

// return mv_diff
static INLINE int is_interp_filter_good_match(
    const INTERPOLATION_FILTER_STATS *st, MB_MODE_INFO *const mi,
    int skip_level) {
  const int is_comp = has_second_ref(mi);
  int i;

  for (i = 0; i < 1 + is_comp; ++i) {
    if (st->ref_frames[i] != mi->ref_frame[i]) return INT_MAX;
  }

  if (skip_level == 1 && is_comp) {
    if (st->comp_type != mi->interinter_comp.type) return INT_MAX;
    if (st->compound_idx != mi->compound_idx) return INT_MAX;
  }

  int mv_diff = 0;
  for (i = 0; i < 1 + is_comp; ++i) {
    mv_diff += abs(st->mv[i].as_mv.row - mi->mv[i].as_mv.row) +
               abs(st->mv[i].as_mv.col - mi->mv[i].as_mv.col);
  }
  return mv_diff;
}

static INLINE int save_interp_filter_search_stat(
    MB_MODE_INFO *const mbmi, int64_t rd, unsigned int pred_sse,
    INTERPOLATION_FILTER_STATS *interp_filter_stats,
    int interp_filter_stats_idx) {
  if (interp_filter_stats_idx < MAX_INTERP_FILTER_STATS) {
    INTERPOLATION_FILTER_STATS stat = { mbmi->interp_filters,
                                        { mbmi->mv[0], mbmi->mv[1] },
                                        { mbmi->ref_frame[0],
                                          mbmi->ref_frame[1] },
                                        mbmi->interinter_comp.type,
                                        mbmi->compound_idx,
                                        rd,
                                        pred_sse };
    interp_filter_stats[interp_filter_stats_idx] = stat;
    interp_filter_stats_idx++;
  }
  return interp_filter_stats_idx;
}

static INLINE int find_interp_filter_in_stats(
    MB_MODE_INFO *const mbmi, INTERPOLATION_FILTER_STATS *interp_filter_stats,
    int interp_filter_stats_idx, int skip_level) {
  // [skip_levels][single or comp]
  const int thr[2][2] = { { 0, 0 }, { 3, 7 } };
  const int is_comp = has_second_ref(mbmi);

  // Find good enough match.
  // TODO(yunqing): Separate single-ref mode and comp mode stats for fast
  // search.
  int best = INT_MAX;
  int match = -1;
  for (int j = 0; j < interp_filter_stats_idx; ++j) {
    const INTERPOLATION_FILTER_STATS *st = &interp_filter_stats[j];
    const int mv_diff = is_interp_filter_good_match(st, mbmi, skip_level);
    // Exact match is found.
    if (mv_diff == 0) {
      match = j;
      break;
    } else if (mv_diff < best && mv_diff <= thr[skip_level - 1][is_comp]) {
      best = mv_diff;
      match = j;
    }
  }

  if (match != -1) {
    mbmi->interp_filters = interp_filter_stats[match].filters;
    return match;
  }
  return -1;  // no match result found
}

int av1_find_interp_filter_match(
    MB_MODE_INFO *const mbmi, const AV1_COMP *const cpi,
    const InterpFilter assign_filter, const int need_search,
    INTERPOLATION_FILTER_STATS *interp_filter_stats,
    int interp_filter_stats_idx) {
  int match_found_idx = -1;
  if (cpi->sf.interp_sf.use_interp_filter && need_search)
    match_found_idx = find_interp_filter_in_stats(
        mbmi, interp_filter_stats, interp_filter_stats_idx,
        cpi->sf.interp_sf.use_interp_filter);

  if (!need_search || match_found_idx == -1)
    set_default_interp_filters(mbmi, assign_filter);
  return match_found_idx;
}

static INLINE int get_switchable_rate(MACROBLOCK *const x,
                                      const int_interpfilters filters,
                                      const int ctx[2], int dual_filter) {
  const InterpFilter filter0 = filters.as_filters.y_filter;
  int inter_filter_cost =
      x->mode_costs.switchable_interp_costs[ctx[0]][filter0];
  if (dual_filter) {
    const InterpFilter filter1 = filters.as_filters.x_filter;
    inter_filter_cost += x->mode_costs.switchable_interp_costs[ctx[1]][filter1];
  }
  return SWITCHABLE_INTERP_RATE_FACTOR * inter_filter_cost;
}

// Build inter predictor and calculate model rd
// for a given plane.
static INLINE void interp_model_rd_eval(
    MACROBLOCK *const x, const AV1_COMP *const cpi, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int plane_from, int plane_to,
    RD_STATS *rd_stats, int is_skip_build_pred) {
  const AV1_COMMON *cm = &cpi->common;
  MACROBLOCKD *const xd = &x->e_mbd;
  RD_STATS tmp_rd_stats;
  av1_init_rd_stats(&tmp_rd_stats);

  // Skip inter predictor if the predictor is already available.
  if (!is_skip_build_pred) {
    const int mi_row = xd->mi_row;
    const int mi_col = xd->mi_col;
    av1_enc_build_inter_predictor(cm, xd, mi_row, mi_col, orig_dst, bsize,
                                  plane_from, plane_to);
  }

  model_rd_sb_fn[cpi->sf.rt_sf.use_simple_rd_model
                     ? MODELRD_LEGACY
                     : MODELRD_TYPE_INTERP_FILTER](
      cpi, bsize, x, xd, plane_from, plane_to, &tmp_rd_stats.rate,
      &tmp_rd_stats.dist, &tmp_rd_stats.skip_txfm, &tmp_rd_stats.sse, NULL,
      NULL, NULL);

  av1_merge_rd_stats(rd_stats, &tmp_rd_stats);
}

#if WILLIAM_COLLECT_FEATURES

void collect_features(struct features_interp_filters *features,
                      MACROBLOCK *const x, MACROBLOCKD *const xd,
                      MB_MODE_INFO *const mbmi, RD_STATS *rd_stats,
                      RD_STATS *rd_stats_luma, const AV1_COMP *const cpi) {
  // x
  features->x__qindex = x->qindex;
  features->x__delta_qindex = x->delta_qindex;
  features->x__rdmult_delta_qindex = x->rdmult_delta_qindex;
  features->x__rdmult_cur_qindex = x->rdmult_cur_qindex;
  features->x__rdmult = x->rdmult;
  features->x__intra_sb_rdmult_modifier = x->intra_sb_rdmult_modifier;
  features->x__rb = x->rb;
  features->x__mb_energy = x->mb_energy;
  features->x__sb_energy_level = x->sb_energy_level;
  features->x__errorperbit = x->errorperbit;
  features->x__sadperbit = x->sadperbit;
  features->x__seg_skip_block = x->seg_skip_block;
  features->x__actual_num_seg1_blocks = x->actual_num_seg1_blocks;
  features->x__actual_num_seg2_blocks = x->actual_num_seg2_blocks;
  features->x__cnt_zeromv = x->cnt_zeromv;
  features->x__force_zeromv_skip_for_sb = x->force_zeromv_skip_for_sb;
  features->x__force_zeromv_skip_for_blk = x->force_zeromv_skip_for_blk;
  features->x__nonrd_prune_ref_frame_search = x->nonrd_prune_ref_frame_search;
  features->x__must_find_valid_partition = x->must_find_valid_partition;
  features->x__skip_mode = x->skip_mode;
  features->x__winner_mode_count = x->winner_mode_count;
  features->x__recalc_luma_mc_data = x->recalc_luma_mc_data;
  features->x__reuse_inter_pred = x->reuse_inter_pred;
  features->x__source_variance = x->source_variance;
  features->x__block_is_zero_sad = x->block_is_zero_sad;
  features->x__sb_me_partition = x->sb_me_partition;
  features->x__sb_me_block = x->sb_me_block;
  features->x__sb_force_fixed_part = x->sb_force_fixed_part;
  features->x__try_merge_partition = x->try_merge_partition;
  features->x__palette_pixels = x->palette_pixels;

  // x->plane[i]
  features->x__plane__0__src_diff = *x->plane[0].src_diff;
  features->x__plane__0__dqcoeff = *x->plane[0].dqcoeff;
  features->x__plane__0__qcoeff = *x->plane[0].qcoeff;
  features->x__plane__0__coeff = *x->plane[0].coeff;
  features->x__plane__0__eobs = *x->plane[0].eobs;
  features->x__plane__0__quant_fp_QTX = *x->plane[0].quant_fp_QTX;
  features->x__plane__0__round_fp_QTX = *x->plane[0].round_fp_QTX;
  features->x__plane__0__quant_QTX = *x->plane[0].quant_QTX;
  features->x__plane__0__round_QTX = *x->plane[0].round_QTX;
  features->x__plane__0__quant_shift_QTX = *x->plane[0].quant_shift_QTX;
  features->x__plane__0__zbin_QTX = *x->plane[0].zbin_QTX;
  features->x__plane__0__dequant_QTX = *x->plane[0].dequant_QTX;

  // x->e_mbd
  features->xd__mi_row = x->e_mbd.mi_row;
  features->xd__mi_col = x->e_mbd.mi_col;
  features->xd__mi_stride = x->e_mbd.mi_stride;
  features->xd__is_chroma_ref = x->e_mbd.is_chroma_ref;
  features->xd__up_available = x->e_mbd.up_available;
  features->xd__left_available = x->e_mbd.left_available;
  features->xd__chroma_up_available = x->e_mbd.chroma_up_available;
  features->xd__chroma_left_available = x->e_mbd.chroma_left_available;
  features->xd__tx_type_map_stride = x->e_mbd.tx_type_map_stride;
  features->xd__mb_to_left_edge = x->e_mbd.mb_to_left_edge;
  features->xd__mb_to_right_edge = x->e_mbd.mb_to_right_edge;
  features->xd__mb_to_top_edge = x->e_mbd.mb_to_top_edge;
  features->xd__mb_to_bottom_edge = x->e_mbd.mb_to_bottom_edge;
  features->xd__width = x->e_mbd.width;
  features->xd__height = x->e_mbd.height;
  features->xd__is_last_vertical_rect = x->e_mbd.is_last_vertical_rect;
  features->xd__is_first_horizontal_rect = x->e_mbd.is_first_horizontal_rect;
  features->xd__bd = x->e_mbd.bd;
  features->xd__current_base_qindex = x->e_mbd.current_base_qindex;
  features->xd__cur_frame_force_integer_mv =
      x->e_mbd.cur_frame_force_integer_mv;
  features->xd__delta_lf_from_base = x->e_mbd.delta_lf_from_base;

  // x->e_mbd->block_ref_scale_factors
  features->xd__block_ref_scale_factors__0__x_scale_fp =
      x->e_mbd.block_ref_scale_factors[0]->x_scale_fp;
  features->xd__block_ref_scale_factors__0__y_scale_fp =
      x->e_mbd.block_ref_scale_factors[0]->y_scale_fp;
  features->xd__block_ref_scale_factors__0__x_step_q4 =
      x->e_mbd.block_ref_scale_factors[0]->x_step_q4;
  features->xd__block_ref_scale_factors__0__y_step_q4 =
      x->e_mbd.block_ref_scale_factors[0]->y_step_q4;
  features->xd__block_ref_scale_factors__1__x_scale_fp =
      x->e_mbd.block_ref_scale_factors[1]->x_scale_fp;
  features->xd__block_ref_scale_factors__1__y_scale_fp =
      x->e_mbd.block_ref_scale_factors[1]->y_scale_fp;
  features->xd__block_ref_scale_factors__1__x_step_q4 =
      x->e_mbd.block_ref_scale_factors[1]->x_step_q4;
  features->xd__block_ref_scale_factors__1__y_step_q4 =
      x->e_mbd.block_ref_scale_factors[1]->y_step_q4;

  // x->e_mbd->plane
  features->xd__plane__0__plane_type = plane_type(x->e_mbd.plane[0].plane_type);
  features->xd__plane__0__subsampling_x = x->e_mbd.plane[0].subsampling_x;
  features->xd__plane__0__width = x->e_mbd.plane[0].width;
  features->xd__plane__0__height = x->e_mbd.plane[0].height;
  features->xd__plane__1__plane_type = plane_type(x->e_mbd.plane[1].plane_type);
  features->xd__plane__1__subsampling_x = x->e_mbd.plane[1].subsampling_x;
  features->xd__plane__1__width = x->e_mbd.plane[1].width;
  features->xd__plane__1__height = x->e_mbd.plane[1].height;
  features->xd__plane__2__plane_type = plane_type(x->e_mbd.plane[2].plane_type);
  features->xd__plane__2__subsampling_x = x->e_mbd.plane[2].subsampling_x;
  features->xd__plane__2__width = x->e_mbd.plane[2].width;
  features->xd__plane__2__height = x->e_mbd.plane[2].height;

  // x->e_mbd->tile
  features->xd__tile__mi_row_start = x->e_mbd.tile.mi_row_start;
  features->xd__tile__mi_row_end = x->e_mbd.tile.mi_row_end;
  features->xd__tile__mi_col_start = x->e_mbd.tile.mi_col_start;
  features->xd__tile__mi_col_end = x->e_mbd.tile.mi_col_end;
  features->xd__tile__tile_row = x->e_mbd.tile.tile_row;
  features->xd__tile__tile_col = x->e_mbd.tile.tile_col;

  // mbmi
  features->mbmi__bsize = block_size(mbmi->bsize);
  features->mbmi__partition = partition_type(mbmi->partition);
  features->mbmi__mode = prediction_mode(mbmi->mode);
  features->mbmi__uv_mode = uv_prediction_mode(mbmi->uv_mode);
  features->mbmi__current_qindex = mbmi->current_qindex;
  features->mbmi__motion_mode = motion_mode(mbmi->motion_mode);
  features->mbmi__num_proj_ref = mbmi->num_proj_ref;
  features->mbmi__overlappable_neighbors = mbmi->overlappable_neighbors;
  features->mbmi__interintra_mode = interintra_mode(mbmi->interintra_mode);
  features->mbmi__interintra_wedge_index = mbmi->interintra_wedge_index;
  features->mbmi__cfl_alpha_signs = mbmi->cfl_alpha_signs;
  features->mbmi__cfl_alpha_idx = mbmi->cfl_alpha_idx;
  features->mbmi__skip_txfm = mbmi->skip_txfm;
  features->mbmi__tx_size = mbmi->tx_size;
  features->mbmi__ref_mv_idx = mbmi->ref_mv_idx;
  features->mbmi__skip_mode = mbmi->skip_mode;
  features->mbmi__use_intrabc = mbmi->use_intrabc;
  features->mbmi__comp_group_idx = mbmi->comp_group_idx;
  features->mbmi__compound_idx = mbmi->compound_idx;
  features->mbmi__use_wedge_interintra = mbmi->use_wedge_interintra;
  features->mbmi__cdef_strength = mbmi->cdef_strength;

  // mbmi->interp_filter
  features->mbmi__interp_filters__as_filters__y_filter =
      mbmi->interp_filters.as_filters.y_filter;
  features->mbmi__interp_filters__as_filters__x_filter =
      mbmi->interp_filters.as_filters.x_filter;

  // mbmi->mv
  features->mbmi__mv__0__as_mv__row = mbmi->mv[0].as_mv.row;
  features->mbmi__mv__1__as_mv__col = mbmi->mv[1].as_mv.col;

  // xd->left_mbmi
  if (xd->left_mbmi) {
    features->xd__left_mbmi__bsize = block_size(xd->left_mbmi->bsize);
    features->xd__left_mbmi__partition =
        partition_type(xd->left_mbmi->partition);
    features->xd__left_mbmi__mode = prediction_mode(xd->left_mbmi->mode);
    features->xd__left_mbmi__uv_mode =
        uv_prediction_mode(xd->left_mbmi->uv_mode);
    features->xd__left_mbmi__current_qindex = xd->left_mbmi->current_qindex;
    features->xd__left_mbmi__motion_mode =
        motion_mode(xd->left_mbmi->motion_mode);
    features->xd__left_mbmi__num_proj_ref = xd->left_mbmi->num_proj_ref;
    features->xd__left_mbmi__overlappable_neighbors =
        xd->left_mbmi->overlappable_neighbors;
    features->xd__left_mbmi__interintra_mode =
        interintra_mode(xd->left_mbmi->interintra_mode);
    features->xd__left_mbmi__interintra_wedge_index =
        xd->left_mbmi->interintra_wedge_index;
    features->xd__left_mbmi__cfl_alpha_signs = xd->left_mbmi->cfl_alpha_signs;
    features->xd__left_mbmi__cfl_alpha_idx = xd->left_mbmi->cfl_alpha_idx;
    features->xd__left_mbmi__skip_txfm = xd->left_mbmi->skip_txfm;
    features->xd__left_mbmi__tx_size = xd->left_mbmi->tx_size;
    features->xd__left_mbmi__ref_mv_idx = xd->left_mbmi->ref_mv_idx;
    features->xd__left_mbmi__skip_mode = xd->left_mbmi->skip_mode;
    features->xd__left_mbmi__use_intrabc = xd->left_mbmi->use_intrabc;
    features->xd__left_mbmi__comp_group_idx = xd->left_mbmi->comp_group_idx;
    features->xd__left_mbmi__compound_idx = xd->left_mbmi->compound_idx;
    features->xd__left_mbmi__use_wedge_interintra =
        xd->left_mbmi->use_wedge_interintra;
    features->xd__left_mbmi__cdef_strength = xd->left_mbmi->cdef_strength;

    // xd->left_mbmi->interp_filter
    features->xd__left_mbmi__interp_filters__as_filters__y_filter =
        xd->left_mbmi->interp_filters.as_filters.y_filter;
    features->xd__left_mbmi__interp_filters__as_filters__x_filter =
        xd->left_mbmi->interp_filters.as_filters.x_filter;

    // xd->left_mbmi->mv
    features->xd__left_mbmi__mv__0__as_mv__row = xd->left_mbmi->mv[0].as_mv.row;
    features->xd__left_mbmi__mv__1__as_mv__col = xd->left_mbmi->mv[1].as_mv.col;
  }

  // xd->above_mbmi
  if (xd->above_mbmi) {
    features->xd__above_mbmi__bsize = block_size(xd->above_mbmi->bsize);
    features->xd__above_mbmi__partition =
        partition_type(xd->above_mbmi->partition);
    features->xd__above_mbmi__mode = prediction_mode(xd->above_mbmi->mode);
    features->xd__above_mbmi__uv_mode =
        uv_prediction_mode(xd->above_mbmi->uv_mode);
    features->xd__above_mbmi__current_qindex = xd->above_mbmi->current_qindex;
    features->xd__above_mbmi__motion_mode =
        motion_mode(xd->above_mbmi->motion_mode);
    features->xd__above_mbmi__num_proj_ref = xd->above_mbmi->num_proj_ref;
    features->xd__above_mbmi__overlappable_neighbors =
        xd->above_mbmi->overlappable_neighbors;
    features->xd__above_mbmi__interintra_mode =
        interintra_mode(xd->above_mbmi->interintra_mode);
    features->xd__above_mbmi__interintra_wedge_index =
        xd->above_mbmi->interintra_wedge_index;
    features->xd__above_mbmi__cfl_alpha_signs = xd->above_mbmi->cfl_alpha_signs;
    features->xd__above_mbmi__cfl_alpha_idx = xd->above_mbmi->cfl_alpha_idx;
    features->xd__above_mbmi__skip_txfm = xd->above_mbmi->skip_txfm;
    features->xd__above_mbmi__tx_size = xd->above_mbmi->tx_size;
    features->xd__above_mbmi__ref_mv_idx = xd->above_mbmi->ref_mv_idx;
    features->xd__above_mbmi__skip_mode = xd->above_mbmi->skip_mode;
    features->xd__above_mbmi__use_intrabc = xd->above_mbmi->use_intrabc;
    features->xd__above_mbmi__comp_group_idx = xd->above_mbmi->comp_group_idx;
    features->xd__above_mbmi__compound_idx = xd->above_mbmi->compound_idx;
    features->xd__above_mbmi__use_wedge_interintra =
        xd->above_mbmi->use_wedge_interintra;
    features->xd__above_mbmi__cdef_strength = xd->above_mbmi->cdef_strength;

    // xd->above_mbmi->interp_filter
    features->xd__above_mbmi__interp_filters__as_filters__y_filter =
        xd->above_mbmi->interp_filters.as_filters.y_filter;
    features->xd__above_mbmi__interp_filters__as_filters__x_filter =
        xd->above_mbmi->interp_filters.as_filters.x_filter;

    // xd->above_mbmi->mv
    features->xd__above_mbmi__mv__0__as_mv__row =
        xd->above_mbmi->mv[0].as_mv.row;
    features->xd__above_mbmi__mv__1__as_mv__col =
        xd->above_mbmi->mv[1].as_mv.col;
  }
  // rd_stats
  features->rd_stats__rate = rd_stats->rate;
  features->rd_stats__zero_rate = rd_stats->zero_rate;
  features->rd_stats__dist = rd_stats->dist;
  features->rd_stats__rdcost = rd_stats->rdcost;
  features->rd_stats__sse = rd_stats->sse;
  features->rd_stats__skip_txfm = rd_stats->skip_txfm;

  // rd_stats_luma
  features->rd_stats_luma__rate = rd_stats_luma->rate;
  features->rd_stats_luma__zero_rate = rd_stats_luma->zero_rate;
  features->rd_stats_luma__dist = rd_stats_luma->dist;
  features->rd_stats_luma__rdcost = rd_stats_luma->rdcost;
  features->rd_stats_luma__sse = rd_stats_luma->sse;
  features->rd_stats_luma__skip_txfm = rd_stats_luma->skip_txfm;

  // cpi
  features->cpi__skip_tpl_setup_stats = cpi->skip_tpl_setup_stats;
  features->cpi__tpl_rdmult_scaling_factors = *cpi->tpl_rdmult_scaling_factors;
  features->cpi__rt_reduce_num_ref_buffers = cpi->rt_reduce_num_ref_buffers;
  features->cpi__framerate = cpi->framerate;
  features->cpi__ref_frame_flags = cpi->ref_frame_flags;
  features->cpi__speed = cpi->speed;
  features->cpi__all_one_sided_refs = cpi->all_one_sided_refs;
  features->cpi__gf_frame_index = cpi->gf_frame_index;
  features->cpi__droppable = cpi->droppable;
  features->cpi__data_alloc_width = cpi->data_alloc_width;
  features->cpi__data_alloc_height = cpi->data_alloc_height;
  features->cpi__initial_mbs = cpi->initial_mbs;
  features->cpi__frame_size_related_setup_done =
      cpi->frame_size_related_setup_done;
  features->cpi__last_coded_width = cpi->last_coded_width;
  features->cpi__last_coded_height = cpi->last_coded_height;
  features->cpi__num_frame_recode = cpi->num_frame_recode;
  features->cpi__intrabc_used = cpi->intrabc_used;
  features->cpi__prune_ref_frame_mask = cpi->prune_ref_frame_mask;
  features->cpi__use_screen_content_tools = cpi->use_screen_content_tools;
  features->cpi__is_screen_content_type = cpi->is_screen_content_type;
  features->cpi__frame_header_count = cpi->frame_header_count;
  features->cpi__deltaq_used = cpi->deltaq_used;
  features->cpi__last_frame_type = frame_type(cpi->last_frame_type);
  features->cpi__num_tg = cpi->num_tg;
  features->cpi__superres_mode = superres_mode(cpi->superres_mode);
  features->cpi__fp_block_size = block_size(cpi->fp_block_size);
  features->cpi__sb_counter = cpi->sb_counter;
  features->cpi__ref_refresh_index = cpi->ref_refresh_index;
  features->cpi__refresh_idx_available = cpi->refresh_idx_available;
  features->cpi__ref_idx_to_skip = cpi->ref_idx_to_skip;
  features->cpi__do_frame_data_update = cpi->do_frame_data_update;
  features->cpi__ext_rate_scale = cpi->ext_rate_scale;
  features->cpi__weber_bsize = block_size(cpi->weber_bsize);
  features->cpi__norm_wiener_variance = cpi->norm_wiener_variance;
  features->cpi__is_dropped_frame = cpi->is_dropped_frame;
  features->cpi__rec_sse = cpi->rec_sse;
  features->cpi__frames_since_last_update = cpi->frames_since_last_update;
  features->cpi__palette_pixel_num = cpi->palette_pixel_num;
  features->cpi__scaled_last_source_available =
      cpi->scaled_last_source_available;

  // cpi->ppi
  features->cpi__ppi__ts_start_last_show_frame =
      cpi->ppi->ts_start_last_show_frame;
  features->cpi__ppi__ts_end_last_show_frame = cpi->ppi->ts_end_last_show_frame;
  features->cpi__ppi__num_fp_contexts = cpi->ppi->num_fp_contexts;
  features->cpi__ppi__filter_level__0 = cpi->ppi->filter_level[0];
  features->cpi__ppi__filter_level__1 = cpi->ppi->filter_level[1];
  features->cpi__ppi__filter_level_u = cpi->ppi->filter_level_u;
  features->cpi__ppi__filter_level_v = cpi->ppi->filter_level_v;
  features->cpi__ppi__seq_params_locked = cpi->ppi->seq_params_locked;
  features->cpi__ppi__internal_altref_allowed =
      cpi->ppi->internal_altref_allowed;
  features->cpi__ppi__show_existing_alt_ref = cpi->ppi->show_existing_alt_ref;
  features->cpi__ppi__lap_enabled = cpi->ppi->lap_enabled;
  features->cpi__ppi__b_calculate_psnr = cpi->ppi->b_calculate_psnr;
  features->cpi__ppi__frames_left = cpi->ppi->frames_left;
  features->cpi__ppi__use_svc = cpi->ppi->use_svc;
  features->cpi__ppi__buffer_removal_time_present =
      cpi->ppi->buffer_removal_time_present;
  features->cpi__ppi__number_temporal_layers = cpi->ppi->number_temporal_layers;
  features->cpi__ppi__number_spatial_layers = cpi->ppi->number_spatial_layers;
  features->cpi__ppi__tpl_sb_rdmult_scaling_factors =
      *cpi->ppi->tpl_sb_rdmult_scaling_factors;

  // cpi->ppi->gf_group
  features->cpi__ppi__gf_group__max_layer_depth =
      cpi->ppi->gf_group.max_layer_depth;
  features->cpi__ppi__gf_group__max_layer_depth_allowed =
      cpi->ppi->gf_group.max_layer_depth_allowed;

  // cpi->ppi->gf_state
  features->cpi__ppi__gf_state__arf_gf_boost_lst =
      cpi->ppi->gf_state.arf_gf_boost_lst;

  // cpi->ppi->twopass
  features->cpi__ppi__twopass__section_intra_rating =
      cpi->ppi->twopass.section_intra_rating;
  features->cpi__ppi__twopass__first_pass_done =
      cpi->ppi->twopass.first_pass_done;
  features->cpi__ppi__twopass__bits_left = cpi->ppi->twopass.bits_left;
  features->cpi__ppi__twopass__modified_error_min =
      cpi->ppi->twopass.modified_error_min;
  features->cpi__ppi__twopass__modified_error_max =
      cpi->ppi->twopass.modified_error_max;
  features->cpi__ppi__twopass__modified_error_left =
      cpi->ppi->twopass.modified_error_left;
  features->cpi__ppi__twopass__kf_group_bits = cpi->ppi->twopass.kf_group_bits;
  features->cpi__ppi__twopass__kf_group_error_left =
      cpi->ppi->twopass.kf_group_error_left;
  features->cpi__ppi__twopass__bpm_factor = cpi->ppi->twopass.bpm_factor;
  features->cpi__ppi__twopass__rolling_arf_group_target_bits =
      cpi->ppi->twopass.rolling_arf_group_target_bits;
  features->cpi__ppi__twopass__rolling_arf_group_actual_bits =
      cpi->ppi->twopass.rolling_arf_group_actual_bits;
  features->cpi__ppi__twopass__sr_update_lag = cpi->ppi->twopass.sr_update_lag;
  features->cpi__ppi__twopass__kf_zeromotion_pct =
      cpi->ppi->twopass.kf_zeromotion_pct;
  features->cpi__ppi__twopass__last_kfgroup_zeromotion_pct =
      cpi->ppi->twopass.last_kfgroup_zeromotion_pct;
  features->cpi__ppi__twopass__extend_minq = cpi->ppi->twopass.extend_minq;
  features->cpi__ppi__twopass__extend_maxq = cpi->ppi->twopass.extend_maxq;

  // cpi->ppi->twopass->firstpass_info->total_stats
  features->cpi__ppi__twopass__firstpass_info__total_stats__frame =
      cpi->ppi->twopass.firstpass_info.total_stats.frame;
  features->cpi__ppi__twopass__firstpass_info__total_stats__weight =
      cpi->ppi->twopass.firstpass_info.total_stats.weight;
  features->cpi__ppi__twopass__firstpass_info__total_stats__intra_error =
      cpi->ppi->twopass.firstpass_info.total_stats.intra_error;
  features
      ->cpi__ppi__twopass__firstpass_info__total_stats__frame_avg_wavelet_energy =
      cpi->ppi->twopass.firstpass_info.total_stats.frame_avg_wavelet_energy;
  features->cpi__ppi__twopass__firstpass_info__total_stats__coded_error =
      cpi->ppi->twopass.firstpass_info.total_stats.coded_error;
  features->cpi__ppi__twopass__firstpass_info__total_stats__sr_coded_error =
      cpi->ppi->twopass.firstpass_info.total_stats.sr_coded_error;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_inter =
      cpi->ppi->twopass.firstpass_info.total_stats.pcnt_inter;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_motion =
      cpi->ppi->twopass.firstpass_info.total_stats.pcnt_motion;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_second_ref =
      cpi->ppi->twopass.firstpass_info.total_stats.pcnt_second_ref;
  features->cpi__ppi__twopass__firstpass_info__total_stats__pcnt_neutral =
      cpi->ppi->twopass.firstpass_info.total_stats.pcnt_neutral;
  features->cpi__ppi__twopass__firstpass_info__total_stats__intra_skip_pct =
      cpi->ppi->twopass.firstpass_info.total_stats.intra_skip_pct;
  features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_rows =
      cpi->ppi->twopass.firstpass_info.total_stats.inactive_zone_rows;
  features->cpi__ppi__twopass__firstpass_info__total_stats__inactive_zone_cols =
      cpi->ppi->twopass.firstpass_info.total_stats.inactive_zone_cols;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVr =
      cpi->ppi->twopass.firstpass_info.total_stats.MVr;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mvr_abs =
      cpi->ppi->twopass.firstpass_info.total_stats.mvr_abs;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVc =
      cpi->ppi->twopass.firstpass_info.total_stats.MVc;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mvc_abs =
      cpi->ppi->twopass.firstpass_info.total_stats.mvc_abs;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVrv =
      cpi->ppi->twopass.firstpass_info.total_stats.MVrv;
  features->cpi__ppi__twopass__firstpass_info__total_stats__MVcv =
      cpi->ppi->twopass.firstpass_info.total_stats.MVcv;
  features->cpi__ppi__twopass__firstpass_info__total_stats__mv_in_out_count =
      cpi->ppi->twopass.firstpass_info.total_stats.mv_in_out_count;
  features->cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count =
      cpi->ppi->twopass.firstpass_info.total_stats.new_mv_count;
  features->cpi__ppi__twopass__firstpass_info__total_stats__duration =
      cpi->ppi->twopass.firstpass_info.total_stats.duration;
  features->cpi__ppi__twopass__firstpass_info__total_stats__count =
      cpi->ppi->twopass.firstpass_info.total_stats.count;
  features->cpi__ppi__twopass__firstpass_info__total_stats__raw_error_stdev =
      cpi->ppi->twopass.firstpass_info.total_stats.raw_error_stdev;
  features->cpi__ppi__twopass__firstpass_info__total_stats__is_flash =
      cpi->ppi->twopass.firstpass_info.total_stats.is_flash;
  features->cpi__ppi__twopass__firstpass_info__total_stats__noise_var =
      cpi->ppi->twopass.firstpass_info.total_stats.noise_var;
  features->cpi__ppi__twopass__firstpass_info__total_stats__cor_coeff =
      cpi->ppi->twopass.firstpass_info.total_stats.cor_coeff;
  features->cpi__ppi__twopass__firstpass_info__total_stats__log_intra_error =
      cpi->ppi->twopass.firstpass_info.total_stats.log_intra_error;
  features->cpi__ppi__twopass__firstpass_info__total_stats__log_coded_error =
      cpi->ppi->twopass.firstpass_info.total_stats.log_coded_error;

  // cpi->ppi->p_rc
  features->cpi__ppi__p_rc__gf_group_bits = cpi->ppi->p_rc.gf_group_bits;
  features->cpi__ppi__p_rc__kf_boost = cpi->ppi->p_rc.kf_boost;
  features->cpi__ppi__p_rc__gfu_boost = cpi->ppi->p_rc.gfu_boost;
  features->cpi__ppi__p_rc__cur_gf_index = cpi->ppi->p_rc.cur_gf_index;
  features->cpi__ppi__p_rc__num_regions = cpi->ppi->p_rc.num_regions;
  features->cpi__ppi__p_rc__regions_offset = cpi->ppi->p_rc.regions_offset;
  features->cpi__ppi__p_rc__frames_till_regions_update =
      cpi->ppi->p_rc.frames_till_regions_update;
  features->cpi__ppi__p_rc__baseline_gf_interval =
      cpi->ppi->p_rc.baseline_gf_interval;
  features->cpi__ppi__p_rc__constrained_gf_group =
      cpi->ppi->p_rc.constrained_gf_group;
  features->cpi__ppi__p_rc__this_key_frame_forced =
      cpi->ppi->p_rc.this_key_frame_forced;
  features->cpi__ppi__p_rc__next_key_frame_forced =
      cpi->ppi->p_rc.next_key_frame_forced;
  features->cpi__ppi__p_rc__starting_buffer_level =
      cpi->ppi->p_rc.starting_buffer_level;
  features->cpi__ppi__p_rc__optimal_buffer_level =
      cpi->ppi->p_rc.optimal_buffer_level;
  features->cpi__ppi__p_rc__maximum_buffer_size =
      cpi->ppi->p_rc.maximum_buffer_size;
  features->cpi__ppi__p_rc__arf_q = cpi->ppi->p_rc.arf_q;
  features->cpi__ppi__p_rc__arf_boost_factor = cpi->ppi->p_rc.arf_boost_factor;
  features->cpi__ppi__p_rc__base_layer_qp = cpi->ppi->p_rc.base_layer_qp;
  features->cpi__ppi__p_rc__num_stats_used_for_kf_boost =
      cpi->ppi->p_rc.num_stats_used_for_kf_boost;
  features->cpi__ppi__p_rc__num_stats_used_for_gfu_boost =
      cpi->ppi->p_rc.num_stats_used_for_gfu_boost;
  features->cpi__ppi__p_rc__num_stats_required_for_gfu_boost =
      cpi->ppi->p_rc.num_stats_required_for_gfu_boost;
  features->cpi__ppi__p_rc__enable_scenecut_detection =
      cpi->ppi->p_rc.enable_scenecut_detection;
  features->cpi__ppi__p_rc__use_arf_in_this_kf_group =
      cpi->ppi->p_rc.use_arf_in_this_kf_group;
  features->cpi__ppi__p_rc__ni_frames = cpi->ppi->p_rc.ni_frames;
  features->cpi__ppi__p_rc__tot_q = cpi->ppi->p_rc.tot_q;
  features->cpi__ppi__p_rc__last_kf_qindex = cpi->ppi->p_rc.last_kf_qindex;
  features->cpi__ppi__p_rc__last_boosted_qindex =
      cpi->ppi->p_rc.last_boosted_qindex;
  features->cpi__ppi__p_rc__avg_q = cpi->ppi->p_rc.avg_q;
  features->cpi__ppi__p_rc__total_actual_bits =
      cpi->ppi->p_rc.total_actual_bits;
  features->cpi__ppi__p_rc__total_target_bits =
      cpi->ppi->p_rc.total_target_bits;
  features->cpi__ppi__p_rc__buffer_level = cpi->ppi->p_rc.buffer_level;
  features->cpi__ppi__p_rc__rate_error_estimate =
      cpi->ppi->p_rc.rate_error_estimate;
  features->cpi__ppi__p_rc__vbr_bits_off_target =
      cpi->ppi->p_rc.vbr_bits_off_target;
  features->cpi__ppi__p_rc__vbr_bits_off_target_fast =
      cpi->ppi->p_rc.vbr_bits_off_target_fast;
  features->cpi__ppi__p_rc__bits_off_target = cpi->ppi->p_rc.bits_off_target;
  features->cpi__ppi__p_rc__rolling_target_bits =
      cpi->ppi->p_rc.rolling_target_bits;
  features->cpi__ppi__p_rc__rolling_actual_bits =
      cpi->ppi->p_rc.rolling_actual_bits;

  // cpi->ppi->tf_info
  features->cpi__ppi__tf_info__is_temporal_filter_on =
      cpi->ppi->tf_info.is_temporal_filter_on;
  features->cpi__ppi__tf_info__frame_diff__0__sum =
      cpi->ppi->tf_info.frame_diff[0].sum;
  features->cpi__ppi__tf_info__frame_diff__0__sse =
      cpi->ppi->tf_info.frame_diff[0].sse;
  features->cpi__ppi__tf_info__frame_diff__1__sum =
      cpi->ppi->tf_info.frame_diff[1].sum;
  features->cpi__ppi__tf_info__frame_diff__1__sse =
      cpi->ppi->tf_info.frame_diff[1].sse;
  features->cpi__ppi__tf_info__tf_buf_gf_index__0 =
      cpi->ppi->tf_info.tf_buf_gf_index[0];
  features->cpi__ppi__tf_info__tf_buf_gf_index__1 =
      cpi->ppi->tf_info.tf_buf_gf_index[1];
  features->cpi__ppi__tf_info__tf_buf_display_index_offset__0 =
      cpi->ppi->tf_info.tf_buf_display_index_offset[0];
  features->cpi__ppi__tf_info__tf_buf_display_index_offset__1 =
      cpi->ppi->tf_info.tf_buf_display_index_offset[1];
  features->cpi__ppi__tf_info__tf_buf_valid__0 =
      cpi->ppi->tf_info.tf_buf_valid[0];
  features->cpi__ppi__tf_info__tf_buf_valid__1 =
      cpi->ppi->tf_info.tf_buf_valid[1];

  // cpi->ppi->seq_params
  features->cpi__ppi__seq_params__num_bits_width =
      cpi->ppi->seq_params.num_bits_width;
  features->cpi__ppi__seq_params__num_bits_height =
      cpi->ppi->seq_params.num_bits_height;
  features->cpi__ppi__seq_params__max_frame_width =
      cpi->ppi->seq_params.max_frame_width;
  features->cpi__ppi__seq_params__max_frame_height =
      cpi->ppi->seq_params.max_frame_height;
  features->cpi__ppi__seq_params__frame_id_numbers_present_flag =
      cpi->ppi->seq_params.frame_id_numbers_present_flag;
  features->cpi__ppi__seq_params__frame_id_length =
      cpi->ppi->seq_params.frame_id_length;
  features->cpi__ppi__seq_params__delta_frame_id_length =
      cpi->ppi->seq_params.delta_frame_id_length;
  features->cpi__ppi__seq_params__sb_size =
      block_size(cpi->ppi->seq_params.sb_size);
  features->cpi__ppi__seq_params__mib_size = cpi->ppi->seq_params.mib_size;
  features->cpi__ppi__seq_params__mib_size_log2 =
      cpi->ppi->seq_params.mib_size_log2;
  features->cpi__ppi__seq_params__force_screen_content_tools =
      screen_content_tool(cpi->ppi->seq_params.force_screen_content_tools);
  features->cpi__ppi__seq_params__still_picture =
      cpi->ppi->seq_params.still_picture;
  features->cpi__ppi__seq_params__reduced_still_picture_hdr =
      cpi->ppi->seq_params.reduced_still_picture_hdr;
  features->cpi__ppi__seq_params__force_integer_mv =
      force_integer_mv(cpi->ppi->seq_params.force_integer_mv);
  features->cpi__ppi__seq_params__enable_filter_intra =
      cpi->ppi->seq_params.enable_filter_intra;
  features->cpi__ppi__seq_params__enable_intra_edge_filter =
      cpi->ppi->seq_params.enable_intra_edge_filter;
  features->cpi__ppi__seq_params__enable_interintra_compound =
      cpi->ppi->seq_params.enable_interintra_compound;
  features->cpi__ppi__seq_params__enable_masked_compound =
      cpi->ppi->seq_params.enable_masked_compound;
  features->cpi__ppi__seq_params__enable_dual_filter =
      cpi->ppi->seq_params.enable_dual_filter;
  features->cpi__ppi__seq_params__enable_warped_motion =
      cpi->ppi->seq_params.enable_warped_motion;
  features->cpi__ppi__seq_params__enable_superres =
      cpi->ppi->seq_params.enable_superres;
  features->cpi__ppi__seq_params__enable_cdef =
      cpi->ppi->seq_params.enable_cdef;
  features->cpi__ppi__seq_params__enable_restoration =
      cpi->ppi->seq_params.enable_restoration;
  features->cpi__ppi__seq_params__profile =
      bitstream_profile(cpi->ppi->seq_params.profile);
  features->cpi__ppi__seq_params__bit_depth = cpi->ppi->seq_params.bit_depth;
  features->cpi__ppi__seq_params__use_highbitdepth =
      cpi->ppi->seq_params.use_highbitdepth;
  features->cpi__ppi__seq_params__monochrome = cpi->ppi->seq_params.monochrome;
  features->cpi__ppi__seq_params__color_primaries =
      color_primary(cpi->ppi->seq_params.color_primaries);
  features->cpi__ppi__seq_params__transfer_characteristics =
      transfer_characteristic(cpi->ppi->seq_params.transfer_characteristics);
  features->cpi__ppi__seq_params__matrix_coefficients =
      matrix_coefficient(cpi->ppi->seq_params.matrix_coefficients);
  features->cpi__ppi__seq_params__color_range =
      cpi->ppi->seq_params.color_range;
  features->cpi__ppi__seq_params__subsampling_x =
      cpi->ppi->seq_params.subsampling_x;
  features->cpi__ppi__seq_params__subsampling_y =
      cpi->ppi->seq_params.subsampling_y;
  features->cpi__ppi__seq_params__chroma_sample_position =
      chroma_sample_position(cpi->ppi->seq_params.chroma_sample_position);
  features->cpi__ppi__seq_params__separate_uv_delta_q =
      cpi->ppi->seq_params.separate_uv_delta_q;
  features->cpi__ppi__seq_params__film_grain_params_present =
      cpi->ppi->seq_params.film_grain_params_present;
  features->cpi__ppi__seq_params__operating_points_cnt_minus_1 =
      cpi->ppi->seq_params.operating_points_cnt_minus_1;
  features->cpi__ppi__seq_params__has_nonzero_operating_point_idc =
      cpi->ppi->seq_params.has_nonzero_operating_point_idc;

  // cpi->ppi->tpl_data
  features->cpi__ppi__tpl_data__ready = cpi->ppi->tpl_data.ready;
  features->cpi__ppi__tpl_data__tpl_stats_block_mis_log2 =
      cpi->ppi->tpl_data.tpl_stats_block_mis_log2;
  features->cpi__ppi__tpl_data__tpl_bsize_1d = cpi->ppi->tpl_data.tpl_bsize_1d;
  features->cpi__ppi__tpl_data__frame_idx = cpi->ppi->tpl_data.frame_idx;
  features->cpi__ppi__tpl_data__border_in_pixels =
      cpi->ppi->tpl_data.border_in_pixels;
  features->cpi__ppi__tpl_data__r0_adjust_factor =
      cpi->ppi->tpl_data.r0_adjust_factor;

  // cpi->ppi->tpl_data->tpl_frame
  features->cpi__ppi__tpl_data__tpl_frame__is_valid =
      cpi->ppi->tpl_data.tpl_frame->is_valid;
  features->cpi__ppi__tpl_data__tpl_frame__stride =
      cpi->ppi->tpl_data.tpl_frame->stride;
  features->cpi__ppi__tpl_data__tpl_frame__width =
      cpi->ppi->tpl_data.tpl_frame->width;
  features->cpi__ppi__tpl_data__tpl_frame__height =
      cpi->ppi->tpl_data.tpl_frame->height;
  features->cpi__ppi__tpl_data__tpl_frame__mi_rows =
      cpi->ppi->tpl_data.tpl_frame->mi_rows;
  features->cpi__ppi__tpl_data__tpl_frame__mi_cols =
      cpi->ppi->tpl_data.tpl_frame->mi_cols;
  features->cpi__ppi__tpl_data__tpl_frame__base_rdmult =
      cpi->ppi->tpl_data.tpl_frame->base_rdmult;
  features->cpi__ppi__tpl_data__tpl_frame__frame_display_index =
      cpi->ppi->tpl_data.tpl_frame->frame_display_index;
  features->cpi__ppi__tpl_data__tpl_frame__use_pred_sad =
      cpi->ppi->tpl_data.tpl_frame->use_pred_sad;

  // cpi->ppi->tpl_data->tpl_frame->tpl_stats_ptr
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_sse =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->srcrf_sse;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_dist =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->srcrf_dist;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->recrf_sse;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_dist =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->recrf_dist;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_sse =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->intra_sse;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_dist =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->intra_dist;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_rate =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->mc_dep_rate;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__mc_dep_dist =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->mc_dep_dist;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_cost =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->intra_cost;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__inter_cost =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->inter_cost;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__srcrf_rate =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->srcrf_rate;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_rate =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->recrf_rate;
  features->cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__intra_rate =
      cpi->ppi->tpl_data.tpl_frame->tpl_stats_ptr->intra_rate;

  // cpi->ppi->tpl_data->sf
  features->cpi__ppi__tpl_data__sf__x_scale_fp =
      cpi->ppi->tpl_data.sf.x_scale_fp;
  features->cpi__ppi__tpl_data__sf__y_scale_fp =
      cpi->ppi->tpl_data.sf.y_scale_fp;
  features->cpi__ppi__tpl_data__sf__x_step_q4 = cpi->ppi->tpl_data.sf.x_step_q4;
  features->cpi__ppi__tpl_data__sf__y_step_q4 = cpi->ppi->tpl_data.sf.y_step_q4;

  // cpi->common
  features->cpi__common__width = cpi->common.width;
  features->cpi__common__height = cpi->common.height;
  features->cpi__common__render_width = cpi->common.render_width;
  features->cpi__common__render_height = cpi->common.render_height;
  features->cpi__common__superres_upscaled_width =
      cpi->common.superres_upscaled_width;
  features->cpi__common__superres_upscaled_height =
      cpi->common.superres_upscaled_height;
  features->cpi__common__superres_scale_denominator =
      cpi->common.superres_scale_denominator;
  features->cpi__common__frame_presentation_time =
      cpi->common.frame_presentation_time;
  features->cpi__common__show_frame = cpi->common.show_frame;
  features->cpi__common__showable_frame = cpi->common.showable_frame;
  features->cpi__common__show_existing_frame = cpi->common.show_existing_frame;

  // cpi->common->current_frame
  features->cpi__common__current_frame__frame_type =
      frame_type(cpi->common.current_frame.frame_type);
  features->cpi__common__current_frame__reference_mode =
      reference_mode(cpi->common.current_frame.reference_mode);
  features->cpi__common__current_frame__order_hint =
      cpi->common.current_frame.order_hint;
  features->cpi__common__current_frame__display_order_hint =
      cpi->common.current_frame.display_order_hint;
  features->cpi__common__current_frame__pyramid_level =
      cpi->common.current_frame.pyramid_level;
  features->cpi__common__current_frame__frame_number =
      cpi->common.current_frame.frame_number;
  features->cpi__common__current_frame__refresh_frame_flags =
      cpi->common.current_frame.refresh_frame_flags;
  features->cpi__common__current_frame__frame_refs_short_signaling =
      cpi->common.current_frame.frame_refs_short_signaling;

  // cpi->common->sf_identity
  features->cpi__common__sf_identity__x_scale_fp =
      cpi->common.sf_identity.x_scale_fp;
  features->cpi__common__sf_identity__y_scale_fp =
      cpi->common.sf_identity.y_scale_fp;
  features->cpi__common__sf_identity__x_step_q4 =
      cpi->common.sf_identity.x_step_q4;
  features->cpi__common__sf_identity__y_step_q4 =
      cpi->common.sf_identity.y_step_q4;

  // cpi->common->features
  features->cpi__common__features__disable_cdf_update =
      cpi->common.features.disable_cdf_update;
  features->cpi__common__features__allow_high_precision_mv =
      cpi->common.features.allow_high_precision_mv;
  features->cpi__common__features__cur_frame_force_integer_mv =
      cpi->common.features.cur_frame_force_integer_mv;
  features->cpi__common__features__allow_screen_content_tools =
      cpi->common.features.allow_screen_content_tools;
  features->cpi__common__features__allow_intrabc =
      cpi->common.features.allow_intrabc;
  features->cpi__common__features__allow_warped_motion =
      cpi->common.features.allow_warped_motion;
  features->cpi__common__features__allow_ref_frame_mvs =
      cpi->common.features.allow_ref_frame_mvs;
  features->cpi__common__features__coded_lossless =
      cpi->common.features.coded_lossless;
  features->cpi__common__features__all_lossless =
      cpi->common.features.all_lossless;
  features->cpi__common__features__reduced_tx_set_used =
      cpi->common.features.reduced_tx_set_used;
  features->cpi__common__features__error_resilient_mode =
      cpi->common.features.error_resilient_mode;
  features->cpi__common__features__switchable_motion_mode =
      cpi->common.features.switchable_motion_mode;
  features->cpi__common__features__tx_mode =
      tx_mode(cpi->common.features.tx_mode);
  features->cpi__common__features__interp_filter =
      interp_filter(cpi->common.features.interp_filter);
  features->cpi__common__features__primary_ref_frame =
      cpi->common.features.primary_ref_frame;
  features->cpi__common__features__byte_alignment =
      cpi->common.features.byte_alignment;
  features->cpi__common__features__refresh_frame_context =
      refresh_frame_context_mode(cpi->common.features.refresh_frame_context);

  // cpi->common->mi_params
  features->cpi__common__mi_params__mb_rows = cpi->common.mi_params.mb_rows;
  features->cpi__common__mi_params__mb_cols = cpi->common.mi_params.mb_cols;
  features->cpi__common__mi_params__MBs = cpi->common.mi_params.MBs;
  features->cpi__common__mi_params__mi_rows = cpi->common.mi_params.mi_rows;
  features->cpi__common__mi_params__mi_cols = cpi->common.mi_params.mi_cols;
  features->cpi__common__mi_params__mi_stride = cpi->common.mi_params.mi_stride;

  // cpi->oxcf
  features->cpi__oxcf__noise_level = cpi->oxcf.noise_level;
  features->cpi__oxcf__noise_block_size = cpi->oxcf.noise_block_size;
  features->cpi__oxcf__enable_dnl_denoising = cpi->oxcf.enable_dnl_denoising;
  features->cpi__oxcf__tier_mask = cpi->oxcf.tier_mask;
  features->cpi__oxcf__border_in_pixels = cpi->oxcf.border_in_pixels;
  features->cpi__oxcf__max_threads = cpi->oxcf.max_threads;
  features->cpi__oxcf__speed = cpi->oxcf.speed;
  features->cpi__oxcf__profile = bitstream_profile(cpi->oxcf.profile);
  features->cpi__oxcf__pass = enc_pass(cpi->oxcf.pass);
  features->cpi__oxcf__passes = cpi->oxcf.passes;
  features->cpi__oxcf__mode = mode(cpi->oxcf.mode);
  features->cpi__oxcf__use_highbitdepth = cpi->oxcf.use_highbitdepth;
  features->cpi__oxcf__save_as_annexb = cpi->oxcf.save_as_annexb;

  // cpi->oxcf->input_cfg
  features->cpi__oxcf__input_cfg__init_framerate =
      cpi->oxcf.input_cfg.init_framerate;
  features->cpi__oxcf__input_cfg__input_bit_depth =
      cpi->oxcf.input_cfg.input_bit_depth;
  features->cpi__oxcf__input_cfg__limit = cpi->oxcf.input_cfg.limit;
  features->cpi__oxcf__input_cfg__chroma_subsampling_x =
      cpi->oxcf.input_cfg.chroma_subsampling_x;
  features->cpi__oxcf__input_cfg__chroma_subsampling_y =
      cpi->oxcf.input_cfg.chroma_subsampling_y;

  // cpi->oxcf->algo_cfg
  features->cpi__oxcf__algo_cfg__sharpness = cpi->oxcf.algo_cfg.sharpness;
  features->cpi__oxcf__algo_cfg__disable_trellis_quant =
      disable_trellis_quant(cpi->oxcf.algo_cfg.disable_trellis_quant);
  features->cpi__oxcf__algo_cfg__arnr_max_frames =
      cpi->oxcf.algo_cfg.arnr_max_frames;
  features->cpi__oxcf__algo_cfg__arnr_strength =
      cpi->oxcf.algo_cfg.arnr_strength;
  features->cpi__oxcf__algo_cfg__cdf_update_mode =
      cdf_update_mode(cpi->oxcf.algo_cfg.cdf_update_mode);
  features->cpi__oxcf__algo_cfg__enable_tpl_model =
      cpi->oxcf.algo_cfg.enable_tpl_model;
  features->cpi__oxcf__algo_cfg__enable_overlay =
      cpi->oxcf.algo_cfg.enable_overlay;
  features->cpi__oxcf__algo_cfg__loopfilter_control =
      loopfilter_control(cpi->oxcf.algo_cfg.loopfilter_control);
  features->cpi__oxcf__algo_cfg__skip_postproc_filtering =
      cpi->oxcf.algo_cfg.skip_postproc_filtering;

  // cpi->oxcf->rc_cfg
  features->cpi__oxcf__rc_cfg__starting_buffer_level_ms =
      cpi->oxcf.rc_cfg.starting_buffer_level_ms;
  features->cpi__oxcf__rc_cfg__optimal_buffer_level_ms =
      cpi->oxcf.rc_cfg.optimal_buffer_level_ms;
  features->cpi__oxcf__rc_cfg__maximum_buffer_size_ms =
      cpi->oxcf.rc_cfg.maximum_buffer_size_ms;
  features->cpi__oxcf__rc_cfg__target_bandwidth =
      cpi->oxcf.rc_cfg.target_bandwidth;
  features->cpi__oxcf__rc_cfg__vbr_corpus_complexity_lap =
      cpi->oxcf.rc_cfg.vbr_corpus_complexity_lap;
  features->cpi__oxcf__rc_cfg__max_intra_bitrate_pct =
      cpi->oxcf.rc_cfg.max_intra_bitrate_pct;
  features->cpi__oxcf__rc_cfg__max_inter_bitrate_pct =
      cpi->oxcf.rc_cfg.max_inter_bitrate_pct;
  features->cpi__oxcf__rc_cfg__gf_cbr_boost_pct =
      cpi->oxcf.rc_cfg.gf_cbr_boost_pct;
  features->cpi__oxcf__rc_cfg__min_cr = cpi->oxcf.rc_cfg.min_cr;
  features->cpi__oxcf__rc_cfg__drop_frames_water_mark =
      cpi->oxcf.rc_cfg.drop_frames_water_mark;
  features->cpi__oxcf__rc_cfg__under_shoot_pct =
      cpi->oxcf.rc_cfg.under_shoot_pct;
  features->cpi__oxcf__rc_cfg__over_shoot_pct = cpi->oxcf.rc_cfg.over_shoot_pct;
  features->cpi__oxcf__rc_cfg__worst_allowed_q =
      cpi->oxcf.rc_cfg.worst_allowed_q;
  features->cpi__oxcf__rc_cfg__best_allowed_q = cpi->oxcf.rc_cfg.best_allowed_q;
  features->cpi__oxcf__rc_cfg__cq_level = cpi->oxcf.rc_cfg.cq_level;
  features->cpi__oxcf__rc_cfg__mode = rc_mode(cpi->oxcf.rc_cfg.mode);
  features->cpi__oxcf__rc_cfg__vbrbias = cpi->oxcf.rc_cfg.vbrbias;
  features->cpi__oxcf__rc_cfg__vbrmin_section = cpi->oxcf.rc_cfg.vbrmin_section;
  features->cpi__oxcf__rc_cfg__vbrmax_section = cpi->oxcf.rc_cfg.vbrmax_section;

  // cpi->oxcf->q_cfg
  features->cpi__oxcf__q_cfg__use_fixed_qp_offsets =
      cpi->oxcf.q_cfg.use_fixed_qp_offsets;
  features->cpi__oxcf__q_cfg__qm_minlevel = cpi->oxcf.q_cfg.qm_minlevel;
  features->cpi__oxcf__q_cfg__qm_maxlevel = cpi->oxcf.q_cfg.qm_maxlevel;
  features->cpi__oxcf__q_cfg__quant_b_adapt = cpi->oxcf.q_cfg.quant_b_adapt;
  features->cpi__oxcf__q_cfg__aq_mode = aq_mode(cpi->oxcf.q_cfg.aq_mode);
  features->cpi__oxcf__q_cfg__deltaq_mode =
      deltaq_mode(cpi->oxcf.q_cfg.deltaq_mode);
  features->cpi__oxcf__q_cfg__deltaq_strength = cpi->oxcf.q_cfg.deltaq_strength;
  features->cpi__oxcf__q_cfg__enable_chroma_deltaq =
      cpi->oxcf.q_cfg.enable_chroma_deltaq;
  features->cpi__oxcf__q_cfg__enable_hdr_deltaq =
      cpi->oxcf.q_cfg.enable_hdr_deltaq;
  features->cpi__oxcf__q_cfg__using_qm = cpi->oxcf.q_cfg.using_qm;

  // cpi->tf_ctx
  features->cpi__tf_ctx__num_frames = cpi->tf_ctx.num_frames;
  features->cpi__tf_ctx__noise_levels__0 = cpi->tf_ctx.noise_levels[0];
  features->cpi__tf_ctx__noise_levels__1 = cpi->tf_ctx.noise_levels[1];
  features->cpi__tf_ctx__noise_levels__2 = cpi->tf_ctx.noise_levels[2];
  features->cpi__tf_ctx__num_pels = cpi->tf_ctx.num_pels;
  features->cpi__tf_ctx__mb_rows = cpi->tf_ctx.mb_rows;
  features->cpi__tf_ctx__mb_cols = cpi->tf_ctx.mb_cols;
  features->cpi__tf_ctx__q_factor = cpi->tf_ctx.q_factor;

  // cpi->rd
  features->cpi__rd__rdmult = cpi->rd.RDMULT;
  features->cpi__rd__r0 = cpi->rd.r0;

  // cpi->mv_search_params
  features->cpi__mv_search_params__max_mv_magnitude =
      cpi->mv_search_params.max_mv_magnitude;
  features->cpi__mv_search_params__mv_step_param =
      cpi->mv_search_params.mv_step_param;

  // cpi->frame_info
  features->cpi__frame_info__frame_width = cpi->frame_info.frame_width;
  features->cpi__frame_info__frame_height = cpi->frame_info.frame_height;
  features->cpi__frame_info__mi_rows = cpi->frame_info.mi_rows;
  features->cpi__frame_info__mi_cols = cpi->frame_info.mi_cols;
  features->cpi__frame_info__mb_rows = cpi->frame_info.mb_rows;
  features->cpi__frame_info__mb_cols = cpi->frame_info.mb_cols;
  features->cpi__frame_info__num_mbs = cpi->frame_info.num_mbs;
  features->cpi__frame_info__bit_depth = cpi->frame_info.bit_depth;
  features->cpi__frame_info__subsampling_x = cpi->frame_info.subsampling_x;
  features->cpi__frame_info__subsampling_y = cpi->frame_info.subsampling_y;

  // cpi->interp_search_flags
  features->cpi__interp_search_flags__default_interp_skip_flags =
      cpi->interp_search_flags.default_interp_skip_flags;
  features->cpi__interp_search_flags__interp_filter_search_mask =
      cpi->interp_search_flags.interp_filter_search_mask;

  // cpi->twopass_frame
  features->cpi__twopass_frame__mb_av_energy = cpi->twopass_frame.mb_av_energy;
  features->cpi__twopass_frame__fr_content_type =
      frame_content_type(cpi->twopass_frame.fr_content_type);
  features->cpi__twopass_frame__frame_avg_haar_energy =
      cpi->twopass_frame.frame_avg_haar_energy;

  // cpi->twopass_frame->stats_in
  features->cpi__twopass_frame__stats_in__frame =
      cpi->twopass_frame.stats_in->frame;
  features->cpi__twopass_frame__stats_in__weight =
      cpi->twopass_frame.stats_in->weight;
  features->cpi__twopass_frame__stats_in__intra_error =
      cpi->twopass_frame.stats_in->intra_error;
  features->cpi__twopass_frame__stats_in__frame_avg_wavelet_energy =
      cpi->twopass_frame.stats_in->frame_avg_wavelet_energy;
  features->cpi__twopass_frame__stats_in__coded_error =
      cpi->twopass_frame.stats_in->coded_error;
  features->cpi__twopass_frame__stats_in__sr_coded_error =
      cpi->twopass_frame.stats_in->sr_coded_error;
  features->cpi__twopass_frame__stats_in__pcnt_inter =
      cpi->twopass_frame.stats_in->pcnt_inter;
  features->cpi__twopass_frame__stats_in__pcnt_motion =
      cpi->twopass_frame.stats_in->pcnt_motion;
  features->cpi__twopass_frame__stats_in__pcnt_second_ref =
      cpi->twopass_frame.stats_in->pcnt_second_ref;
  features->cpi__twopass_frame__stats_in__pcnt_neutral =
      cpi->twopass_frame.stats_in->pcnt_neutral;
  features->cpi__twopass_frame__stats_in__intra_skip_pct =
      cpi->twopass_frame.stats_in->intra_skip_pct;
  features->cpi__twopass_frame__stats_in__inactive_zone_rows =
      cpi->twopass_frame.stats_in->inactive_zone_rows;
  features->cpi__twopass_frame__stats_in__inactive_zone_cols =
      cpi->twopass_frame.stats_in->inactive_zone_cols;
  features->cpi__twopass_frame__stats_in__MVr =
      cpi->twopass_frame.stats_in->MVr;
  features->cpi__twopass_frame__stats_in__mvr_abs =
      cpi->twopass_frame.stats_in->mvr_abs;
  features->cpi__twopass_frame__stats_in__MVc =
      cpi->twopass_frame.stats_in->MVc;
  features->cpi__twopass_frame__stats_in__mvc_abs =
      cpi->twopass_frame.stats_in->mvc_abs;
  features->cpi__twopass_frame__stats_in__MVrv =
      cpi->twopass_frame.stats_in->MVrv;
  features->cpi__twopass_frame__stats_in__MVcv =
      cpi->twopass_frame.stats_in->MVcv;
  features->cpi__twopass_frame__stats_in__mv_in_out_count =
      cpi->twopass_frame.stats_in->mv_in_out_count;
  features->cpi__twopass_frame__stats_in__new_mv_count =
      cpi->twopass_frame.stats_in->new_mv_count;
  features->cpi__twopass_frame__stats_in__duration =
      cpi->twopass_frame.stats_in->duration;
  features->cpi__twopass_frame__stats_in__count =
      cpi->twopass_frame.stats_in->count;
  features->cpi__twopass_frame__stats_in__raw_error_stdev =
      cpi->twopass_frame.stats_in->raw_error_stdev;
  features->cpi__twopass_frame__stats_in__is_flash =
      cpi->twopass_frame.stats_in->is_flash;
  features->cpi__twopass_frame__stats_in__noise_var =
      cpi->twopass_frame.stats_in->noise_var;
  features->cpi__twopass_frame__stats_in__cor_coeff =
      cpi->twopass_frame.stats_in->cor_coeff;
  features->cpi__twopass_frame__stats_in__log_intra_error =
      cpi->twopass_frame.stats_in->log_intra_error;
  features->cpi__twopass_frame__stats_in__log_coded_error =
      cpi->twopass_frame.stats_in->log_coded_error;
}
#endif  // WILLIAM_PREDICT_FEATURES

// calculate the rdcost of given interpolation_filter
// william: esse método deve receber early-skip
static INLINE int64_t interpolation_filter_rd(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int64_t *const rd,
    RD_STATS *rd_stats_luma, RD_STATS *rd_stats, int *const switchable_rate,
    const BUFFER_SET *dst_bufs[2], int filter_idx, const int switchable_ctx[2],
    const int skip_pred) {
  const AV1_COMMON *cm = &cpi->common;
  const InterpSearchFlags *interp_search_flags = &cpi->interp_search_flags;
  const int num_planes = av1_num_planes(cm);
  MACROBLOCKD *const xd = &x->e_mbd;
  MB_MODE_INFO *const mbmi = xd->mi[0];
  RD_STATS this_rd_stats_luma, this_rd_stats;

  // Força o pior caso
  //  return 0;

  // Initialize rd_stats structures to default values.
  av1_init_rd_stats(&this_rd_stats_luma);
  this_rd_stats = *rd_stats_luma;
  const int_interpfilters last_best = mbmi->interp_filters;
  mbmi->interp_filters = filter_sets[filter_idx];
  const int tmp_rs =
      get_switchable_rate(x, mbmi->interp_filters, switchable_ctx,
                          cm->seq_params->enable_dual_filter);

  int64_t min_rd = RDCOST(x->rdmult, tmp_rs, 0);
  if (min_rd > *rd) {
    mbmi->interp_filters = last_best;
    return 0;
  }

  (void)tile_data;

  assert(skip_pred != 2);
  assert((rd_stats_luma->rate >= 0) && (rd_stats->rate >= 0));
  assert((rd_stats_luma->dist >= 0) && (rd_stats->dist >= 0));
  assert((rd_stats_luma->sse >= 0) && (rd_stats->sse >= 0));
  assert((rd_stats_luma->skip_txfm == 0) || (rd_stats_luma->skip_txfm == 1));
  assert((rd_stats->skip_txfm == 0) || (rd_stats->skip_txfm == 1));
  assert((skip_pred >= 0) &&
         (skip_pred <= interp_search_flags->default_interp_skip_flags));

  // When skip_txfm pred is equal to default_interp_skip_flags,
  // skip both luma and chroma MC.
  // For mono-chrome images:
  // num_planes = 1 and cpi->default_interp_skip_flags = 1,
  // skip_pred = 1: skip both luma and chroma
  // skip_pred = 0: Evaluate luma and as num_planes=1,
  // skip chroma evaluation
  int tmp_skip_pred =
      (skip_pred == interp_search_flags->default_interp_skip_flags)
          ? INTERP_SKIP_LUMA_SKIP_CHROMA
          : skip_pred;

#if WILLIAM_COLLECT_FEATURES
  struct features_interp_filters features;
  collect_init(&features);

  features.filter_idx = filter_idx;
  features.bsize = block_size(bsize);
  features.skip_pred = skip_pred;
  features.num_planes = num_planes;
  features.tmp_rs = tmp_rs;
  features.min_rd = min_rd;
  features.switchable_rate = *switchable_rate;
  features.rd = *rd;
  features.last_best__as_filters__x_filter = last_best.as_filters.x_filter;
  features.last_best__as_filters__y_filter = last_best.as_filters.y_filter;
  collect_features(&features, x, xd, mbmi, rd_stats, rd_stats_luma, cpi);
#endif  // WILLIAM_COLLECT_FEATURES

  // william: baseado no filtro anterior, tamanho de bloco, etc, o filtro a ser
  // testado vai ser melhor?

#if WILLIAM_BEST_CASE_FILTER_READ
  int skip = read_discard();

  if (skip) {
    mbmi->interp_filters = last_best;
    return 0;
  }
#endif

#if WILLIAM_PREDICT_FEATURES

#if CONFIG_COLLECT_COMPONENT_TIMING
  start_timing(cpi, interpolation_filter_search_pred_time);
#endif

  int skip = will_discard(tmp_rs,
                          min_rd,
                          *switchable_rate,
                          x->cnt_zeromv,
                          *cpi->tpl_rdmult_scaling_factors,
                          cpi->ppi->twopass.rolling_arf_group_actual_bits,
                          cpi->ppi->p_rc.rolling_actual_bits,
                          cpi->rd.r0,
                          *x->plane[0].round_fp_QTX,
                          *x->plane[0].quant_QTX,
                          cpi->twopass_frame.stats_in->intra_error,
                          cpi->twopass_frame.stats_in->pcnt_neutral,
                          cpi->twopass_frame.stats_in->MVr,
                          cpi->twopass_frame.stats_in->raw_error_stdev,
                          bsize,
                          filter_idx,
                          mbmi->mode);

#if CONFIG_COLLECT_COMPONENT_TIMING
  end_timing(cpi, interpolation_filter_search_pred_time);
#endif

#if WILLIAM_COLLECT_FEATURES
  features.prediction = skip;
#endif  // WILLIAM_COLLECT_FEATURES

#if WILLIAM_PREDICT_SKIP
  if (skip) {
#if WILLIAM_COLLECT_FEATURES
    collect_finalize(&features);
#endif  // WILLIAM_COLLECT_FEATURES

    mbmi->interp_filters = last_best;
    return 0;
  }
#endif  // WILLIAM_PREDICT_SKIP

#endif  // WILLIAM_PREDICT_FEATURES

  switch (tmp_skip_pred) {
    case INTERP_EVAL_LUMA_EVAL_CHROMA:
      // skip_pred = 0: Evaluate both luma and chroma.
      // Luma MC
      interp_model_rd_eval(x, cpi, bsize, orig_dst, AOM_PLANE_Y, AOM_PLANE_Y,
                           &this_rd_stats_luma, 0);
      this_rd_stats = this_rd_stats_luma;
#if CONFIG_COLLECT_RD_STATS == 3
      RD_STATS rd_stats_y;
      av1_pick_recursive_tx_size_type_yrd(cpi, x, &rd_stats_y, bsize,
                                          INT64_MAX);
      PrintPredictionUnitStats(cpi, tile_data, x, &rd_stats_y, bsize);
#endif  // CONFIG_COLLECT_RD_STATS == 3
      AOM_FALLTHROUGH_INTENDED;
    case INTERP_SKIP_LUMA_EVAL_CHROMA:
      // skip_pred = 1: skip luma evaluation (retain previous best luma stats)
      // and do chroma evaluation.
      for (int plane = 1; plane < num_planes; ++plane) {
        int64_t tmp_rd =
            RDCOST(x->rdmult, tmp_rs + this_rd_stats.rate, this_rd_stats.dist);
        if (tmp_rd >= *rd) {
          mbmi->interp_filters = last_best;

#if WILLIAM_COLLECT_FEATURES
          // william: foi pior; desfez
          features.discard = 1;
          collect_finalize(&features);
#endif

#if WILLIAM_BEST_CASE_FILTER_WRITE
          print_discard(1);
#endif

          return 0;
        }
        interp_model_rd_eval(x, cpi, bsize, orig_dst, plane, plane,
                             &this_rd_stats, 0);
      }
      break;
    case INTERP_SKIP_LUMA_SKIP_CHROMA:
      // both luma and chroma evaluation is skipped
      this_rd_stats = *rd_stats;
      break;
    case INTERP_EVAL_INVALID:
    default: assert(0); return 0;
  }
  int64_t tmp_rd =
      RDCOST(x->rdmult, tmp_rs + this_rd_stats.rate, this_rd_stats.dist);

  if (tmp_rd < *rd) {
    *rd = tmp_rd;
    *switchable_rate = tmp_rs;
    if (skip_pred != interp_search_flags->default_interp_skip_flags) {
      if (skip_pred == INTERP_EVAL_LUMA_EVAL_CHROMA) {
        // Overwrite the data as current filter is the best one
        *rd_stats_luma = this_rd_stats_luma;
        *rd_stats = this_rd_stats;
        // As luma MC data is computed, no need to recompute after the search
        x->recalc_luma_mc_data = 0;
      } else if (skip_pred == INTERP_SKIP_LUMA_EVAL_CHROMA) {
        // As luma MC data is not computed, update of luma data can be skipped
        *rd_stats = this_rd_stats;
        // As luma MC data is not recomputed and current filter is the best,
        // indicate the possibility of recomputing MC data
        // If current buffer contains valid MC data, toggle to indicate that
        // luma MC data needs to be recomputed
        x->recalc_luma_mc_data ^= 1;
      }
      swap_dst_buf(xd, dst_bufs, num_planes);
    }

#if WILLIAM_COLLECT_FEATURES
    // william: foi melhor; salvou
    features.discard = 0;
    collect_finalize(&features);
#endif

#if WILLIAM_BEST_CASE_FILTER_WRITE
    print_discard(0);
#endif

    return 1;
  }

#if WILLIAM_COLLECT_FEATURES
  // william: foi pior; desfez
  features.discard = 1;
  collect_finalize(&features);
#endif

#if WILLIAM_BEST_CASE_FILTER_WRITE
  print_discard(1);
#endif

  mbmi->interp_filters = last_best;
  return 0;
}

static INLINE INTERP_PRED_TYPE is_pred_filter_search_allowed(
    const AV1_COMP *const cpi, MACROBLOCKD *xd, BLOCK_SIZE bsize,
    int_interpfilters *af, int_interpfilters *lf) {
  const AV1_COMMON *cm = &cpi->common;
  const MB_MODE_INFO *const above_mbmi = xd->above_mbmi;
  const MB_MODE_INFO *const left_mbmi = xd->left_mbmi;
  const int bsl = mi_size_wide_log2[bsize];
  int is_horiz_eq = 0, is_vert_eq = 0;

  if (above_mbmi && is_inter_block(above_mbmi))
    *af = above_mbmi->interp_filters;

  if (left_mbmi && is_inter_block(left_mbmi)) *lf = left_mbmi->interp_filters;

  if (af->as_filters.x_filter != INTERP_INVALID)
    is_horiz_eq = af->as_filters.x_filter == lf->as_filters.x_filter;
  if (af->as_filters.y_filter != INTERP_INVALID)
    is_vert_eq = af->as_filters.y_filter == lf->as_filters.y_filter;

  INTERP_PRED_TYPE pred_filter_type = (is_vert_eq << 1) + is_horiz_eq;
  const int mi_row = xd->mi_row;
  const int mi_col = xd->mi_col;
  int pred_filter_enable =
      cpi->sf.interp_sf.cb_pred_filter_search
          ? (((mi_row + mi_col) >> bsl) +
             get_chessboard_index(cm->current_frame.frame_number)) &
                0x1
          : 0;
  pred_filter_enable &= is_horiz_eq || is_vert_eq;
  // pred_filter_search = 0: pred_filter is disabled
  // pred_filter_search = 1: pred_filter is enabled and only horz pred matching
  // pred_filter_search = 2: pred_filter is enabled and only vert pred matching
  // pred_filter_search = 3: pred_filter is enabled and
  //                         both vert, horz pred matching
  return pred_filter_enable * pred_filter_type;
}

static DUAL_FILTER_TYPE find_best_interp_rd_facade(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int64_t *const rd, RD_STATS *rd_stats_y,
    RD_STATS *rd_stats, int *const switchable_rate,
    const BUFFER_SET *dst_bufs[2], const int switchable_ctx[2],
    const int skip_pred, uint16_t allow_interp_mask, int is_w4_or_h4) {
  int tmp_skip_pred = skip_pred;
  DUAL_FILTER_TYPE best_filt_type = REG_REG;

  // If no filter are set to be evaluated, return from function
  if (allow_interp_mask == 0x0) return best_filt_type;
  // For block width or height is 4, skip the pred evaluation of SHARP_SHARP
  tmp_skip_pred = is_w4_or_h4
                      ? cpi->interp_search_flags.default_interp_skip_flags
                      : skip_pred;

  // Loop over the all filter types and evaluate for only allowed filter types
  for (int filt_type = SHARP_SHARP; filt_type >= REG_REG; --filt_type) {
    const int is_filter_allowed =
        get_interp_filter_allowed_mask(allow_interp_mask, filt_type);
    if (is_filter_allowed)
      if (interpolation_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                                  rd_stats_y, rd_stats, switchable_rate,
                                  dst_bufs, filt_type, switchable_ctx,
                                  tmp_skip_pred))
        best_filt_type = filt_type;
    tmp_skip_pred = skip_pred;
  }
  return best_filt_type;
}

static INLINE void pred_dual_interp_filter_rd(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int64_t *const rd, RD_STATS *rd_stats_y,
    RD_STATS *rd_stats, int *const switchable_rate,
    const BUFFER_SET *dst_bufs[2], const int switchable_ctx[2],
    const int skip_pred, INTERP_PRED_TYPE pred_filt_type, int_interpfilters *af,
    int_interpfilters *lf) {
  (void)lf;
  assert(pred_filt_type > INTERP_HORZ_NEQ_VERT_NEQ);
  assert(pred_filt_type < INTERP_PRED_TYPE_ALL);
  uint16_t allowed_interp_mask = 0;

  if (pred_filt_type == INTERP_HORZ_EQ_VERT_NEQ) {
    // pred_filter_search = 1: Only horizontal filter is matching
    allowed_interp_mask =
        av1_interp_dual_filt_mask[pred_filt_type - 1][af->as_filters.x_filter];
  } else if (pred_filt_type == INTERP_HORZ_NEQ_VERT_EQ) {
    // pred_filter_search = 2: Only vertical filter is matching
    allowed_interp_mask =
        av1_interp_dual_filt_mask[pred_filt_type - 1][af->as_filters.y_filter];
  } else {
    // pred_filter_search = 3: Both horizontal and vertical filter are matching
    int filt_type =
        af->as_filters.x_filter + af->as_filters.y_filter * SWITCHABLE_FILTERS;
    set_interp_filter_allowed_mask(&allowed_interp_mask, filt_type);
  }
  // REG_REG is already been evaluated in the beginning
  reset_interp_filter_allowed_mask(&allowed_interp_mask, REG_REG);
  find_best_interp_rd_facade(x, cpi, tile_data, bsize, orig_dst, rd, rd_stats_y,
                             rd_stats, switchable_rate, dst_bufs,
                             switchable_ctx, skip_pred, allowed_interp_mask, 0);
}
// Evaluate dual filter type
// a) Using above, left block interp filter
// b) Find the best horizontal filter and
//    then evaluate corresponding vertical filters.
static INLINE void fast_dual_interp_filter_rd(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int64_t *const rd, RD_STATS *rd_stats_y,
    RD_STATS *rd_stats, int *const switchable_rate,
    const BUFFER_SET *dst_bufs[2], const int switchable_ctx[2],
    const int skip_hor, const int skip_ver) {
  const InterpSearchFlags *interp_search_flags = &cpi->interp_search_flags;
  MACROBLOCKD *const xd = &x->e_mbd;
  MB_MODE_INFO *const mbmi = xd->mi[0];
  INTERP_PRED_TYPE pred_filter_type = INTERP_HORZ_NEQ_VERT_NEQ;
  int_interpfilters af = av1_broadcast_interp_filter(INTERP_INVALID);
  int_interpfilters lf = af;

  if (!have_newmv_in_inter_mode(mbmi->mode)) {
    pred_filter_type = is_pred_filter_search_allowed(cpi, xd, bsize, &af, &lf);
  }

  if (pred_filter_type) {
    pred_dual_interp_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                               rd_stats_y, rd_stats, switchable_rate, dst_bufs,
                               switchable_ctx, (skip_hor & skip_ver),
                               pred_filter_type, &af, &lf);
  } else {
    const int bw = block_size_wide[bsize];
    const int bh = block_size_high[bsize];
    int best_dual_mode = 0;
    int skip_pred =
        bw <= 4 ? interp_search_flags->default_interp_skip_flags : skip_hor;
    // TODO(any): Make use of find_best_interp_rd_facade()
    // if speed impact is negligible
    for (int i = (SWITCHABLE_FILTERS - 1); i >= 1; --i) {
      if (interpolation_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                                  rd_stats_y, rd_stats, switchable_rate,
                                  dst_bufs, i, switchable_ctx, skip_pred)) {
        best_dual_mode = i;
      }
      skip_pred = skip_hor;
    }
    // From best of horizontal EIGHTTAP_REGULAR modes, check vertical modes
    skip_pred =
        bh <= 4 ? interp_search_flags->default_interp_skip_flags : skip_ver;
    for (int i = (best_dual_mode + (SWITCHABLE_FILTERS * 2));
         i >= (best_dual_mode + SWITCHABLE_FILTERS); i -= SWITCHABLE_FILTERS) {
      interpolation_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                              rd_stats_y, rd_stats, switchable_rate, dst_bufs,
                              i, switchable_ctx, skip_pred);
      skip_pred = skip_ver;
    }
  }
}

// Find the best interp filter if dual_interp_filter = 0
static INLINE void find_best_non_dual_interp_filter(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const orig_dst, int64_t *const rd, RD_STATS *rd_stats_y,
    RD_STATS *rd_stats, int *const switchable_rate,
    const BUFFER_SET *dst_bufs[2], const int switchable_ctx[2],
    const int skip_ver, const int skip_hor) {
  const InterpSearchFlags *interp_search_flags = &cpi->interp_search_flags;
  int8_t i;
  MACROBLOCKD *const xd = &x->e_mbd;
  MB_MODE_INFO *const mbmi = xd->mi[0];

  uint16_t interp_filter_search_mask =
      interp_search_flags->interp_filter_search_mask;

  if (cpi->sf.interp_sf.adaptive_interp_filter_search == 2) {
    const FRAME_UPDATE_TYPE update_type =
        get_frame_update_type(&cpi->ppi->gf_group, cpi->gf_frame_index);
    const int ctx0 = av1_get_pred_context_switchable_interp(xd, 0);
    const int ctx1 = av1_get_pred_context_switchable_interp(xd, 1);
    int use_actual_frame_probs = 1;
    const int *switchable_interp_p0;
    const int *switchable_interp_p1;
#if CONFIG_FPMT_TEST
    use_actual_frame_probs =
        (cpi->ppi->fpmt_unit_test_cfg == PARALLEL_SIMULATION_ENCODE) ? 0 : 1;
    if (!use_actual_frame_probs) {
      switchable_interp_p0 = (int *)cpi->ppi->temp_frame_probs
                                 .switchable_interp_probs[update_type][ctx0];
      switchable_interp_p1 = (int *)cpi->ppi->temp_frame_probs
                                 .switchable_interp_probs[update_type][ctx1];
    }
#endif
    if (use_actual_frame_probs) {
      switchable_interp_p0 =
          cpi->ppi->frame_probs.switchable_interp_probs[update_type][ctx0];
      switchable_interp_p1 =
          cpi->ppi->frame_probs.switchable_interp_probs[update_type][ctx1];
    }
    static const int thr[7] = { 0, 8, 8, 8, 8, 0, 8 };
    const int thresh = thr[update_type];
    for (i = 0; i < SWITCHABLE_FILTERS; i++) {
      // For non-dual case, the 2 dir's prob should be identical.
      assert(switchable_interp_p0[i] == switchable_interp_p1[i]);
      if (switchable_interp_p0[i] < thresh &&
          switchable_interp_p1[i] < thresh) {
        DUAL_FILTER_TYPE filt_type = i + SWITCHABLE_FILTERS * i;
        reset_interp_filter_allowed_mask(&interp_filter_search_mask, filt_type);
      }
    }
  }

  // Regular filter evaluation should have been done and hence the same should
  // be the winner
  assert(x->e_mbd.mi[0]->interp_filters.as_int == filter_sets[0].as_int);
  if ((skip_hor & skip_ver) != interp_search_flags->default_interp_skip_flags) {
    INTERP_PRED_TYPE pred_filter_type = INTERP_HORZ_NEQ_VERT_NEQ;
    int_interpfilters af = av1_broadcast_interp_filter(INTERP_INVALID);
    int_interpfilters lf = af;

    pred_filter_type = is_pred_filter_search_allowed(cpi, xd, bsize, &af, &lf);
    if (pred_filter_type) {
      assert(af.as_filters.x_filter != INTERP_INVALID);
      int filter_idx = SWITCHABLE * af.as_filters.x_filter;
      // This assert tells that (filter_x == filter_y) for non-dual filter case
      assert(filter_sets[filter_idx].as_filters.x_filter ==
             filter_sets[filter_idx].as_filters.y_filter);
      if (cpi->sf.interp_sf.adaptive_interp_filter_search &&
          !(get_interp_filter_allowed_mask(interp_filter_search_mask,
                                           filter_idx))) {
        return;
      }
      if (filter_idx) {
        interpolation_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                                rd_stats_y, rd_stats, switchable_rate, dst_bufs,
                                filter_idx, switchable_ctx,
                                (skip_hor & skip_ver));
      }
      return;
    }
  }
  // Reuse regular filter's modeled rd data for sharp filter for following
  // cases
  // 1) When bsize is 4x4
  // 2) When block width is 4 (i.e. 4x8/4x16 blocks) and MV in vertical
  // direction is full-pel
  // 3) When block height is 4 (i.e. 8x4/16x4 blocks) and MV in horizontal
  // direction is full-pel
  // TODO(any): Optimize cases 2 and 3 further if luma MV in relavant direction
  // alone is full-pel

  if ((bsize == BLOCK_4X4) ||
      (block_size_wide[bsize] == 4 &&
       skip_ver == interp_search_flags->default_interp_skip_flags) ||
      (block_size_high[bsize] == 4 &&
       skip_hor == interp_search_flags->default_interp_skip_flags)) {
    int skip_pred = skip_hor & skip_ver;
    uint16_t allowed_interp_mask = 0;

    // REG_REG filter type is evaluated beforehand, hence skip it
    set_interp_filter_allowed_mask(&allowed_interp_mask, SHARP_SHARP);
    set_interp_filter_allowed_mask(&allowed_interp_mask, SMOOTH_SMOOTH);
    if (cpi->sf.interp_sf.adaptive_interp_filter_search)
      allowed_interp_mask &= interp_filter_search_mask;

    find_best_interp_rd_facade(x, cpi, tile_data, bsize, orig_dst, rd,
                               rd_stats_y, rd_stats, switchable_rate, dst_bufs,
                               switchable_ctx, skip_pred, allowed_interp_mask,
                               1);
  } else {
    int skip_pred = (skip_hor & skip_ver);
    for (i = (SWITCHABLE_FILTERS + 1); i < DUAL_FILTER_SET_SIZE;
         i += (SWITCHABLE_FILTERS + 1)) {
      // This assert tells that (filter_x == filter_y) for non-dual filter case
      assert(filter_sets[i].as_filters.x_filter ==
             filter_sets[i].as_filters.y_filter);
      if (cpi->sf.interp_sf.adaptive_interp_filter_search &&
          !(get_interp_filter_allowed_mask(interp_filter_search_mask, i))) {
        continue;
      }
      interpolation_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                              rd_stats_y, rd_stats, switchable_rate, dst_bufs,
                              i, switchable_ctx, skip_pred);
      // In first iteration, smooth filter is evaluated. If smooth filter
      // (which is less sharper) is the winner among regular and smooth filters,
      // sharp filter evaluation is skipped
      // TODO(any): Refine this gating based on modelled rd only (i.e., by not
      // accounting switchable filter rate)
      if (cpi->sf.interp_sf.skip_sharp_interp_filter_search &&
          skip_pred != interp_search_flags->default_interp_skip_flags) {
        if (mbmi->interp_filters.as_int == filter_sets[SMOOTH_SMOOTH].as_int)
          break;
      }
    }
  }
}

static INLINE void calc_interp_skip_pred_flag(MACROBLOCK *const x,
                                              const AV1_COMP *const cpi,
                                              int *skip_hor, int *skip_ver) {
  const AV1_COMMON *cm = &cpi->common;
  MACROBLOCKD *const xd = &x->e_mbd;
  MB_MODE_INFO *const mbmi = xd->mi[0];
  const int num_planes = av1_num_planes(cm);
  const int is_compound = has_second_ref(mbmi);
  assert(is_intrabc_block(mbmi) == 0);
  for (int ref = 0; ref < 1 + is_compound; ++ref) {
    const struct scale_factors *const sf =
        get_ref_scale_factors_const(cm, mbmi->ref_frame[ref]);
    // TODO(any): Refine skip flag calculation considering scaling
    if (av1_is_scaled(sf)) {
      *skip_hor = 0;
      *skip_ver = 0;
      break;
    }
    const MV mv = mbmi->mv[ref].as_mv;
    int skip_hor_plane = 0;
    int skip_ver_plane = 0;
    for (int plane_idx = 0; plane_idx < AOMMAX(1, (num_planes - 1));
         ++plane_idx) {
      struct macroblockd_plane *const pd = &xd->plane[plane_idx];
      const int bw = pd->width;
      const int bh = pd->height;
      const MV mv_q4 = clamp_mv_to_umv_border_sb(
          xd, &mv, bw, bh, pd->subsampling_x, pd->subsampling_y);
      const int sub_x = (mv_q4.col & SUBPEL_MASK) << SCALE_EXTRA_BITS;
      const int sub_y = (mv_q4.row & SUBPEL_MASK) << SCALE_EXTRA_BITS;
      skip_hor_plane |= ((sub_x == 0) << plane_idx);
      skip_ver_plane |= ((sub_y == 0) << plane_idx);
    }
    *skip_hor &= skip_hor_plane;
    *skip_ver &= skip_ver_plane;
    // It is not valid that "luma MV is sub-pel, whereas chroma MV is not"
    assert(*skip_hor != 2);
    assert(*skip_ver != 2);
  }
  // When compond prediction type is compound segment wedge, luma MC and chroma
  // MC need to go hand in hand as mask generated during luma MC is reuired for
  // chroma MC. If skip_hor = 0 and skip_ver = 1, mask used for chroma MC during
  // vertical filter decision may be incorrect as temporary MC evaluation
  // overwrites the mask. Make skip_ver as 0 for this case so that mask is
  // populated during luma MC
  if (is_compound && mbmi->compound_idx == 1 &&
      mbmi->interinter_comp.type == COMPOUND_DIFFWTD) {
    assert(mbmi->comp_group_idx == 1);
    if (*skip_hor == 0 && *skip_ver == 1) *skip_ver = 0;
  }
}

/*!\brief AV1 interpolation filter search
 *
 * \ingroup inter_mode_search
 *
 * \param[in]     cpi               Top-level encoder structure.
 * \param[in]     tile_data         Pointer to struct holding adaptive
 *                                  data/contexts/models for the tile during
 *                                  encoding.
 * \param[in]     x                 Pointer to struc holding all the data for
 *                                  the current macroblock.
 * \param[in]     bsize             Current block size.
 * \param[in]     tmp_dst           A temporary prediction buffer to hold a
 *                                  computed prediction.
 * \param[in,out] orig_dst          A prediction buffer to hold a computed
 *                                  prediction. This will eventually hold the
 *                                  final prediction, and the tmp_dst info will
 *                                  be copied here.
 * \param[in,out] rd                The RD cost associated with the selected
 *                                  interpolation filter parameters.
 * \param[in,out] switchable_rate   The rate associated with using a SWITCHABLE
 *                                  filter mode.
 * \param[in,out] skip_build_pred   Indicates whether or not to build the inter
 *                                  predictor. If this is 0, the inter predictor
 *                                  has already been built and thus we can avoid
 *                                  repeating computation.
 * \param[in]     args              HandleInterModeArgs struct holding
 *                                  miscellaneous arguments for inter mode
 *                                  search. See the documentation for this
 *                                  struct for a description of each member.
 * \param[in]     ref_best_rd       Best RD found so far for this block.
 *                                  It is used for early termination of this
 *                                  search if the RD exceeds this value.
 *
 * \return Returns INT64_MAX if the filter parameters are invalid and the
 * current motion mode being tested should be skipped. It returns 0 if the
 * parameter search is a success.
 */
int64_t av1_interpolation_filter_search(
    MACROBLOCK *const x, const AV1_COMP *const cpi,
    const TileDataEnc *tile_data, BLOCK_SIZE bsize,
    const BUFFER_SET *const tmp_dst, const BUFFER_SET *const orig_dst,
    int64_t *const rd, int *const switchable_rate, int *skip_build_pred,
    HandleInterModeArgs *args, int64_t ref_best_rd) {
  const AV1_COMMON *cm = &cpi->common;
  const InterpSearchFlags *interp_search_flags = &cpi->interp_search_flags;
  const int num_planes = av1_num_planes(cm);
  MACROBLOCKD *const xd = &x->e_mbd;
  MB_MODE_INFO *const mbmi = xd->mi[0];
  const int need_search = av1_is_interp_needed(xd);
  const int ref_frame = xd->mi[0]->ref_frame[0];
  RD_STATS rd_stats_luma, rd_stats;

  // Initialization of rd_stats structures with default values
  av1_init_rd_stats(&rd_stats_luma);
  av1_init_rd_stats(&rd_stats);

  int match_found_idx = -1;
  const InterpFilter assign_filter = cm->features.interp_filter;

  match_found_idx = av1_find_interp_filter_match(
      mbmi, cpi, assign_filter, need_search, args->interp_filter_stats,
      args->interp_filter_stats_idx);

  if (match_found_idx != -1) {
    *rd = args->interp_filter_stats[match_found_idx].rd;
    x->pred_sse[ref_frame] =
        args->interp_filter_stats[match_found_idx].pred_sse;
    *skip_build_pred = 0;
    return 0;
  }

  int switchable_ctx[2];
  switchable_ctx[0] = av1_get_pred_context_switchable_interp(xd, 0);
  switchable_ctx[1] = av1_get_pred_context_switchable_interp(xd, 1);
  *switchable_rate =
      get_switchable_rate(x, mbmi->interp_filters, switchable_ctx,
                          cm->seq_params->enable_dual_filter);

  // Do MC evaluation for default filter_type.
  // Luma MC
  interp_model_rd_eval(x, cpi, bsize, orig_dst, AOM_PLANE_Y, AOM_PLANE_Y,
                       &rd_stats_luma, *skip_build_pred);

#if CONFIG_COLLECT_RD_STATS == 3
  RD_STATS rd_stats_y;
  av1_pick_recursive_tx_size_type_yrd(cpi, x, &rd_stats_y, bsize, INT64_MAX);
  PrintPredictionUnitStats(cpi, tile_data, x, &rd_stats_y, bsize);
#endif  // CONFIG_COLLECT_RD_STATS == 3
  // Chroma MC
  if (num_planes > 1) {
    interp_model_rd_eval(x, cpi, bsize, orig_dst, AOM_PLANE_U, AOM_PLANE_V,
                         &rd_stats, *skip_build_pred);
  }
  *skip_build_pred = 1;

  av1_merge_rd_stats(&rd_stats, &rd_stats_luma);

  assert(rd_stats.rate >= 0);

  *rd = RDCOST(x->rdmult, *switchable_rate + rd_stats.rate, rd_stats.dist);
  x->pred_sse[ref_frame] = (unsigned int)(rd_stats_luma.sse >> 4);

  if (assign_filter != SWITCHABLE || match_found_idx != -1) {
    return 0;
  }
  if (!need_search) {
    int_interpfilters filters = av1_broadcast_interp_filter(EIGHTTAP_REGULAR);
    assert(mbmi->interp_filters.as_int == filters.as_int);
    (void)filters;
    return 0;
  }
  if (args->modelled_rd != NULL) {
    if (has_second_ref(mbmi)) {
      const int ref_mv_idx = mbmi->ref_mv_idx;
      MV_REFERENCE_FRAME *refs = mbmi->ref_frame;
      const int mode0 = compound_ref0_mode(mbmi->mode);
      const int mode1 = compound_ref1_mode(mbmi->mode);
      const int64_t mrd = AOMMIN(args->modelled_rd[mode0][ref_mv_idx][refs[0]],
                                 args->modelled_rd[mode1][ref_mv_idx][refs[1]]);
      if ((*rd >> 1) > mrd && ref_best_rd < INT64_MAX) {
        return INT64_MAX;
      }
    }
  }

  x->recalc_luma_mc_data = 0;
  // skip_flag=xx (in binary form)
  // Setting 0th flag corresonds to skipping luma MC and setting 1st bt
  // corresponds to skipping chroma MC  skip_flag=0 corresponds to "Don't skip
  // luma and chroma MC"  Skip flag=1 corresponds to "Skip Luma MC only"
  // Skip_flag=2 is not a valid case
  // skip_flag=3 corresponds to "Skip both luma and chroma MC"
  int skip_hor = interp_search_flags->default_interp_skip_flags;
  int skip_ver = interp_search_flags->default_interp_skip_flags;
  calc_interp_skip_pred_flag(x, cpi, &skip_hor, &skip_ver);

  // do interp_filter search
  restore_dst_buf(xd, *tmp_dst, num_planes);
  const BUFFER_SET *dst_bufs[2] = { tmp_dst, orig_dst };
  // Evaluate dual interp filters
  if (cm->seq_params->enable_dual_filter) {
    if (cpi->sf.interp_sf.use_fast_interpolation_filter_search) {
      fast_dual_interp_filter_rd(x, cpi, tile_data, bsize, orig_dst, rd,
                                 &rd_stats_luma, &rd_stats, switchable_rate,
                                 dst_bufs, switchable_ctx, skip_hor, skip_ver);
    } else {
      // Use full interpolation filter search
      uint16_t allowed_interp_mask = ALLOW_ALL_INTERP_FILT_MASK;
      // REG_REG filter type is evaluated beforehand, so loop is repeated over
      // REG_SMOOTH to SHARP_SHARP for full interpolation filter search
      reset_interp_filter_allowed_mask(&allowed_interp_mask, REG_REG);
      find_best_interp_rd_facade(x, cpi, tile_data, bsize, orig_dst, rd,
                                 &rd_stats_luma, &rd_stats, switchable_rate,
                                 dst_bufs, switchable_ctx,
                                 (skip_hor & skip_ver), allowed_interp_mask, 0);
    }
  } else {
    // Evaluate non-dual interp filters
    find_best_non_dual_interp_filter(
        x, cpi, tile_data, bsize, orig_dst, rd, &rd_stats_luma, &rd_stats,
        switchable_rate, dst_bufs, switchable_ctx, skip_ver, skip_hor);
  }
  swap_dst_buf(xd, dst_bufs, num_planes);
  // Recompute final MC data if required
  if (x->recalc_luma_mc_data == 1) {
    // Recomputing final luma MC data is required only if the same was skipped
    // in either of the directions  Condition below is necessary, but not
    // sufficient
    assert((skip_hor == 1) || (skip_ver == 1));
    const int mi_row = xd->mi_row;
    const int mi_col = xd->mi_col;
    av1_enc_build_inter_predictor(cm, xd, mi_row, mi_col, orig_dst, bsize,
                                  AOM_PLANE_Y, AOM_PLANE_Y);
  }
  x->pred_sse[ref_frame] = (unsigned int)(rd_stats_luma.sse >> 4);

  // save search results
  if (cpi->sf.interp_sf.use_interp_filter) {
    assert(match_found_idx == -1);
    args->interp_filter_stats_idx = save_interp_filter_search_stat(
        mbmi, *rd, x->pred_sse[ref_frame], args->interp_filter_stats,
        args->interp_filter_stats_idx);
  }
  return 0;
}
