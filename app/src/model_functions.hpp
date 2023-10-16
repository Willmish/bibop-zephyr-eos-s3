#ifndef BIBOP_ZEPHYR_EOS_S3_APP_MODEL_FUNCTIONS_H_
#define BIBOP_ZEPHYR_EOS_S3_APP_MODEL_FUNCTIONS_H_

#include "common.hpp"

/* Expose a C friendly interface for main functions. */
#ifdef __cplusplus
extern "C" {
#endif

    /* Initializes all data needed for the example. The name is important, and needs
     *   */
    void setup_model(void);

    /* Runs one iteration of data gathering and inference. This should be called
     *  * repeatedly from the application code. The name needs to be loop() for Arduino
     *   * compatibility.
     *    */
    Inferred loop_model(void);

#ifdef __cplusplus
}
#endif

#endif /* BIBOP_ZEPHYR_EOS_S3_APP_MODEL_FUNCTIONS_H_ */

