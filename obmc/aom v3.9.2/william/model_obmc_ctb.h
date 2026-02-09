#ifndef AOM_MODEL_OBMC_CTB_H
#define AOM_MODEL_OBMC_CTB_H

#ifdef __cplusplus
extern "C" {
#endif

double ApplyCatboostModelWrapper(
    const float* floatFeatures, int floatFeaturesSize
);

#ifdef __cplusplus
}
#endif

#endif  // AOM_MODEL_OBMC_CTB_H
