#ifndef BIBOP_ZEPHYR_EOS_S3_APP_COMMON_H_
#define BIBOP_ZEPHYR_EOS_S3_APP_COMMON_H_

typedef struct
{
    float cycle_len;
    float t_start_sys;
    float t_sys_end;
    float t_sys_dicr;
    float t_dicr_end;
    float ratio;
} Features;

typedef struct
{
    float sbp;
    float dbp;
} Inferred;

#endif /* BIBOP_ZEPHYR_EOS_S3_APP_COMMON_H_ */

