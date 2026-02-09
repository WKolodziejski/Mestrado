#ifndef AOM_FEATURES_PREDICT_H
#define AOM_FEATURES_PREDICT_H

#include "model_obmc_ctb.h"
#include "av1/common/enums.h"
#include "av1/common/blockd.h"

// Deve usar o modelo para fazer o skip?
#define WILLIAM_PREDICT_SKIP 1

int will_discard(
    long long int rd_stats__rdcost,
    int args__ref_frame_cost,
    int mbmi__skip_txfm
    );

#endif  // AOM_FEATURES_PREDICT_H
