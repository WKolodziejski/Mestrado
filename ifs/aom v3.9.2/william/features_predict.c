#include "features_predict.h"
#include "features_xgb_low.c"
#include "features_xgb_high.c"

int comp(int in, int comp) { return in == comp; }

void build_input_low(
    double input[49],
    double tmp_rs,
    double min_rd,
    double switchable_rate,
    double x__cnt_zeromv,
    double x__plane__0__round_fp_QTX,
    double x__plane__0__quant_QTX,
    double cpi__tpl_rdmult_scaling_factors,
    double cpi__ppi__twopass__rolling_arf_group_actual_bits,
    double cpi__rd__r0,
    double cpi__twopass_frame__stats_in__intra_error,
    double cpi__twopass_frame__stats_in__pcnt_neutral,
    double cpi__twopass_frame__stats_in__MVr,
    double cpi__twopass_frame__stats_in__raw_error_stdev,
    BLOCK_SIZE bsize,
    int filter_idx,
    PREDICTION_MODE mode
) {
  input[0] = tmp_rs;
  input[1] = min_rd;
  input[2] = switchable_rate;
  input[3] = x__cnt_zeromv;
  input[4] = x__plane__0__round_fp_QTX;
  input[5] = x__plane__0__quant_QTX;
  input[6] = cpi__tpl_rdmult_scaling_factors;
  input[7] = cpi__ppi__twopass__rolling_arf_group_actual_bits;
  input[8] = cpi__rd__r0;
  input[9] = cpi__twopass_frame__stats_in__intra_error;
  input[10] = cpi__twopass_frame__stats_in__pcnt_neutral;
  input[11] = cpi__twopass_frame__stats_in__MVr;
  input[12] = cpi__twopass_frame__stats_in__raw_error_stdev;

  input[13] = comp(filter_idx, 4);            // filter_idx_4
  input[14] = comp(filter_idx, 5);            // filter_idx_5
  input[15] = comp(filter_idx, 6);            // filter_idx_6

  input[16] = comp(bsize, BLOCK_128X128);      // 128X128
  input[17] = comp(bsize, BLOCK_128X64);       // 128X64
  input[18] = comp(bsize, BLOCK_16X16);        // 16X16
  input[19] = comp(bsize, BLOCK_16X32);        // 16X32
  input[20] = comp(bsize, BLOCK_16X4);         // 16X4
  input[21] = comp(bsize, BLOCK_16X64);        // 16X64
  input[22] = comp(bsize, BLOCK_16X8);         // 16X8
  input[23] = comp(bsize, BLOCK_32X16);       // 32X16
  input[24] = comp(bsize, BLOCK_32X32);       // 32X32
  input[25] = comp(bsize, BLOCK_32X64);       // 32X64
  input[26] = comp(bsize, BLOCK_32X8);        // 32X8
  input[27] = comp(bsize, BLOCK_4X16);        // 4X16
  input[28] = comp(bsize, BLOCK_4X4);         // 4X4
  input[29] = comp(bsize, BLOCK_4X8);         // 4X8
  input[30] = comp(bsize, BLOCK_64X128);      // 64X128
  input[31] = comp(bsize, BLOCK_64X16);       // 64X16
  input[32] = comp(bsize, BLOCK_64X32);       // 64X32
  input[33] = comp(bsize, BLOCK_64X64);       // 64X64
  input[34] = comp(bsize, BLOCK_8X16);        // 8X16
  input[35] = comp(bsize, BLOCK_8X32);        // 8X32
  input[36] = comp(bsize, BLOCK_8X4);         // 8X4
  input[37] = comp(bsize, BLOCK_8X8);         // 8X8

  input[38] = comp(mode, GLOBALMV);           // mode_GLOBALMV
  input[39] = comp(mode, NEARESTMV);          // mode_NEARESTMV
  input[40] = comp(mode, NEAREST_NEARESTMV);  // mode_NEAREST_NEARESTMV
  input[41] = comp(mode, NEAREST_NEWMV);      // mode_NEAREST_NEWMV
  input[42] = comp(mode, NEARMV);             // mode_NEARMV
  input[43] = comp(mode, NEAR_NEARMV);        // mode_NEAR_NEARMV
  input[44] = comp(mode, NEAR_NEWMV);         // mode_NEAR_NEWMV
  input[45] = comp(mode, NEWMV);              // mode_NEWMV
  input[46] = comp(mode, NEW_NEARESTMV);      // mode_NEW_NEARESTMV
  input[47] = comp(mode, NEW_NEARMV);         // mode_NEW_NEARMV
  input[48] = comp(mode, NEW_NEWMV);          // mode_NEW_NEWMV
}

int is_bsize(int in, BLOCK_SIZE comp) { return in == comp; }

int is_filter_idx(int in, int comp) { return in == comp; }

int is_prediction_mode(int in, PREDICTION_MODE comp) { return in == comp; }

void build_input(double input[49], int tmp_rs, double min_rd,
                 int switchable_rate, int cnt_zeromv,
                 double tpl_rdmult_scaling_factors,
                 int rolling_arf_group_actual_bits, int rolling_actual_bits,
                 double r0, BLOCK_SIZE bsize, int filter_idx,
                 PREDICTION_MODE mode) {
  input[0] = tmp_rs;                      // tmp_rs
  input[1] = min_rd;                      // min_rd
  input[2] = switchable_rate;             // switchable_rate
  input[3] = cnt_zeromv;                  // x__cnt_zeromv
  input[4] = tpl_rdmult_scaling_factors;  // cpi__tpl_rdmult_scaling_factors
  input[5] =
      rolling_arf_group_actual_bits;  // cpi__ppi__twopass__rolling_arf_group_actual_bits
  input[6] = rolling_actual_bits;  // cpi__ppi__p_rc__rolling_actual_bits
  input[7] = r0;                   // cpi__rd__r0
  input[8] = is_bsize(bsize, BLOCK_128X128);  // 128X128
  input[9] = is_bsize(bsize, BLOCK_128X64);   // 128X64
  input[10] = is_bsize(bsize, BLOCK_16X16);   // 16X16
  input[11] = is_bsize(bsize, BLOCK_16X32);   // 16X32
  input[12] = is_bsize(bsize, BLOCK_16X4);    // 16X4
  input[13] = is_bsize(bsize, BLOCK_16X64);   // 16X64
  input[14] = is_bsize(bsize, BLOCK_16X8);    // 16X8
  input[15] = is_bsize(bsize, BLOCK_32X16);   // 32X16
  input[16] = is_bsize(bsize, BLOCK_32X32);   // 32X32
  input[17] = is_bsize(bsize, BLOCK_32X64);   // 32X64
  input[18] = is_bsize(bsize, BLOCK_32X8);    // 32X8
  input[19] = is_bsize(bsize, BLOCK_4X16);    // 4X16
  input[20] = is_bsize(bsize, BLOCK_4X4);     // 4X4
  input[21] = is_bsize(bsize, BLOCK_4X8);     // 4X8
  input[22] = is_bsize(bsize, BLOCK_64X128);  // 64X128
  input[23] = is_bsize(bsize, BLOCK_64X16);   // 64X16
  input[24] = is_bsize(bsize, BLOCK_64X32);   // 64X32
  input[25] = is_bsize(bsize, BLOCK_64X64);   // 64X64
  input[26] = is_bsize(bsize, BLOCK_8X16);    // 8X16
  input[27] = is_bsize(bsize, BLOCK_8X32);    // 8X32
  input[28] = is_bsize(bsize, BLOCK_8X4);     // 8X4
  input[29] = is_bsize(bsize, BLOCK_8X8);     // 8X8
  input[30] = is_filter_idx(filter_idx, 1);   // filter_idx_1
  input[31] = is_filter_idx(filter_idx, 2);   // filter_idx_2
  input[32] = is_filter_idx(filter_idx, 3);   // filter_idx_3
  input[33] = is_filter_idx(filter_idx, 4);   // filter_idx_4
  input[34] = is_filter_idx(filter_idx, 5);   // filter_idx_5
  input[35] = is_filter_idx(filter_idx, 6);   // filter_idx_6
  input[36] = is_filter_idx(filter_idx, 7);   // filter_idx_7
  input[37] = is_filter_idx(filter_idx, 8);   // filter_idx_8
  input[38] =
      is_prediction_mode(mode, GLOBALMV);  // x__e_mbd__mi__0__mode_GLOBALMV
  input[39] =
      is_prediction_mode(mode, NEARESTMV);  // x__e_mbd__mi__0__mode_NEARESTMV
  input[40] = is_prediction_mode(
      mode, NEAREST_NEARESTMV);  // x__e_mbd__mi__0__mode_NEAREST_NEARESTMV
  input[41] = is_prediction_mode(
      mode, NEAREST_NEWMV);  // x__e_mbd__mi__0__mode_NEAREST_NEWMV
  input[42] = is_prediction_mode(mode, NEARMV);  // x__e_mbd__mi__0__mode_NEARMV
  input[43] = is_prediction_mode(
      mode, NEAR_NEARMV);  // x__e_mbd__mi__0__mode_NEAR_NEARMV
  input[44] =
      is_prediction_mode(mode, NEAR_NEWMV);  // x__e_mbd__mi__0__mode_NEAR_NEWMV
  input[45] = is_prediction_mode(mode, NEWMV);  // x__e_mbd__mi__0__mode_NEWMV
  input[46] = is_prediction_mode(
      mode, NEW_NEARESTMV);  // x__e_mbd__mi__0__mode_NEW_NEARESTMV
  input[47] =
      is_prediction_mode(mode, NEW_NEARMV);  // x__e_mbd__mi__0__mode_NEW_NEARMV
  input[48] =
      is_prediction_mode(mode, NEW_NEWMV);  // x__e_mbd__mi__0__mode_NEW_NEWMV
}

int will_discard(double tmp_rs,
                 double min_rd,
                 double switchable_rate,
                 double x__cnt_zeromv,
                 double cpi__tpl_rdmult_scaling_factors,
                 double cpi__ppi__twopass__rolling_arf_group_actual_bits,
                 double cpi__ppi__p_rc__rolling_actual_bits,
                 double cpi__rd__r0,
                 double x__plane__0__round_fp_QTX,
                 double x__plane__0__quant_QTX,
                 double cpi__twopass_frame__stats_in__intra_error,
                 double cpi__twopass_frame__stats_in__pcnt_neutral,
                 double cpi__twopass_frame__stats_in__MVr,
                 double cpi__twopass_frame__stats_in__raw_error_stdev,
                 BLOCK_SIZE bsize,
                 int filter_idx,
                 PREDICTION_MODE mode
) {
  double output[2];  // Discard? [False, True]

  if (!(filter_idx == 4 || filter_idx == 5 || filter_idx == 6)) {
    double input[49];

    build_input(input, tmp_rs, min_rd, switchable_rate, x__cnt_zeromv,
                cpi__tpl_rdmult_scaling_factors, cpi__ppi__twopass__rolling_arf_group_actual_bits,
                cpi__ppi__p_rc__rolling_actual_bits, cpi__rd__r0, bsize, filter_idx, mode);

    score_high(input, output);

  } else {
    double input[68];

    build_input_low(input,
                    tmp_rs,
                    min_rd,
                    switchable_rate,
                    x__cnt_zeromv,
                    x__plane__0__round_fp_QTX,
                    x__plane__0__quant_QTX,
                    cpi__tpl_rdmult_scaling_factors,
                    cpi__ppi__twopass__rolling_arf_group_actual_bits,
                    cpi__rd__r0,
                    cpi__twopass_frame__stats_in__intra_error,
                    cpi__twopass_frame__stats_in__pcnt_neutral,
                    cpi__twopass_frame__stats_in__MVr,
                    cpi__twopass_frame__stats_in__raw_error_stdev,
                    bsize,
                    filter_idx,
                    mode);

    score_low(input, output);
  }

  double f = output[0];
  double t = output[1];

  return t > f;
}
