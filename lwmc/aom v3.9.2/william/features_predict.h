#ifndef AOM_FEATURES_PREDICT_H
#define AOM_FEATURES_PREDICT_H

#include "warp.h"

// Deve usar o modelo para fazer o skip?
#define WILLIAM_PREDICT_SKIP 0

int will_discard(
    long long int rd_stats__rdcost,
    long long int best_est_rd,
    int cpi__rd__rdmult,
    long long int rd_stats__dist,
    char *mbmi__mode,
    int rd_stats__rate,
    int rate_mv,
    int rd_stats_y__rate,
    double cpi__ppi__twopass__firstpass_info__total_stats__MVrv,
    int rate2_nocoeff,
    int xd__width,
    unsigned int x__source_variance,
    double cpi__ppi__twopass__firstpass_info__total_stats__MVr,
    long long cpi__ppi__tf_info__frame_diff__1__sse,
    int xd__plane__2__height,
    int x__errorperbit,
    int cpi__ppi__p_rc__last_boosted_qindex,
    unsigned char cpi__gf_frame_index,
    long long int ref_best_rd,
    int cpi__common__current_frame__refresh_frame_flags,
    int x__plane__0__eobs,
    unsigned int args__best_pred_sse,
    int mbmi__num_proj_ref,
    double cpi__ppi__twopass__firstpass_info__total_stats__new_mv_count,
    int mbmi__ref_mv_idx,
    long long cpi__ppi__tpl_data__tpl_frame__tpl_stats_ptr__recrf_sse,
    int x__qindex,
    double cpi__ppi__tpl_sb_rdmult_scaling_factors,
    int mbmi__skip_txfm,
    char *mbmi__interintra_mode,
    int x__rdmult,
    int cpi__ppi__p_rc__rolling_actual_bits
    );

#endif  // AOM_FEATURES_PREDICT_H
