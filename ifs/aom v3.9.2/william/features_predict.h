#ifndef AOM_FEATURES_PREDICT_H
#define AOM_FEATURES_PREDICT_H

#include "av1/common/enums.h"
#include "av1/common/blockd.h"
#include "features_collect.h"

// Deve usar o modelo para predição?
#define WILLIAM_PREDICT_FEATURES 0

// Deve usar o modelo para fazer o skip?
#define WILLIAM_PREDICT_SKIP 0

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
);

#endif  // AOM_FEATURES_PREDICT_H
