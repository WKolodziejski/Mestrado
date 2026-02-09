#include "features_predict.h"
#include "model_obmc_ctb.h"

// CTB
int will_discard(
    long long int rd_stats__rdcost,
    int args__ref_frame_cost,
    int mbmi__skip_txfm
) {
  float floatFeatures[] = {
    rd_stats__rdcost,
    args__ref_frame_cost,
    mbmi__skip_txfm
  };

  return ApplyCatboostModelWrapper(floatFeatures, 3);
}

int comp(int in, int comp) { return in == comp; }