#ifndef BIBOP_ZEPHYR_EOS_S3_APP_BP_PROCESSING_H_
#define BIBOP_ZEPHYR_EOS_S3_APP_BP_PROCESSING_H_

/* Expose a C friendly interface for main functions. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float cycle_len;
    float t_start_sys;
    float t_sys_end;
    float t_sys_dicr;
    float t_dicr_end;
    float ratio;
} Features;

Features preprocess_data();

#ifdef __cplusplus
}
#endif

#endif /* BIBOP_ZEPHYR_EOS_S3_APP_MODEL_FUNCTIONS_H_*/

