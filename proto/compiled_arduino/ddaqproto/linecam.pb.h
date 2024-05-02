/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.1 */

#ifndef PB_DOLPHINDAQ_LINECAM_LINECAM_PB_H_INCLUDED
#define PB_DOLPHINDAQ_LINECAM_LINECAM_PB_H_INCLUDED
#include <pb.h>
#include "timestamp.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _dolphindaq_linecam_Metrics {
    pb_callback_t shot_num;
    pb_callback_t shot_time;
    pb_callback_t shot_time_seconds;
    pb_callback_t shot_time_nanos;
    pb_callback_t shot_time_alt_nanos;
    pb_callback_t image_sum;
    pb_callback_t image_mean;
    pb_callback_t image_max;
    pb_callback_t image_min;
    pb_callback_t image_custom;
} dolphindaq_linecam_Metrics;

typedef struct _dolphindaq_linecam_Image {
    bool has_shot_num;
    uint64_t shot_num;
    bool has_shot_time;
    google_protobuf_Timestamp shot_time;
    bool has_shot_time_alt_nanos;
    uint64_t shot_time_alt_nanos;
    pb_callback_t yvals;
} dolphindaq_linecam_Image;

typedef struct _dolphindaq_linecam_Settings {
    bool has_start_shot_num;
    uint64_t start_shot_num;
    bool has_start_time;
    google_protobuf_Timestamp start_time;
    pb_callback_t sensor;
    bool has_exposure;
    double exposure;
    bool has_image_nx;
    uint32_t image_nx;
    bool has_metrics_batch_size;
    uint32_t metrics_batch_size;
    bool has_clk_dt;
    double clk_dt;
    bool has_timtick_secs;
    double timtick_secs;
    bool has_clk_dt_timticks;
    uint32_t clk_dt_timticks;
    bool has_exposure_clk_dts;
    uint32_t exposure_clk_dts;
} dolphindaq_linecam_Settings;

typedef struct _dolphindaq_linecam_Data {
    bool has_image;
    dolphindaq_linecam_Image image;
    bool has_metrics;
    dolphindaq_linecam_Metrics metrics;
} dolphindaq_linecam_Data;


/* Initializer values for message structs */
#define dolphindaq_linecam_Settings_init_default {false, 0, false, google_protobuf_Timestamp_init_default, {{NULL}, NULL}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define dolphindaq_linecam_Data_init_default     {false, dolphindaq_linecam_Image_init_default, false, dolphindaq_linecam_Metrics_init_default}
#define dolphindaq_linecam_Image_init_default    {false, 0, false, google_protobuf_Timestamp_init_default, false, 0, {{NULL}, NULL}}
#define dolphindaq_linecam_Metrics_init_default  {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}
#define dolphindaq_linecam_Settings_init_zero    {false, 0, false, google_protobuf_Timestamp_init_zero, {{NULL}, NULL}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define dolphindaq_linecam_Data_init_zero        {false, dolphindaq_linecam_Image_init_zero, false, dolphindaq_linecam_Metrics_init_zero}
#define dolphindaq_linecam_Image_init_zero       {false, 0, false, google_protobuf_Timestamp_init_zero, false, 0, {{NULL}, NULL}}
#define dolphindaq_linecam_Metrics_init_zero     {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define dolphindaq_linecam_Metrics_shot_num_tag  1
#define dolphindaq_linecam_Metrics_shot_time_tag 2
#define dolphindaq_linecam_Metrics_shot_time_seconds_tag 3
#define dolphindaq_linecam_Metrics_shot_time_nanos_tag 4
#define dolphindaq_linecam_Metrics_shot_time_alt_nanos_tag 5
#define dolphindaq_linecam_Metrics_image_sum_tag 6
#define dolphindaq_linecam_Metrics_image_mean_tag 7
#define dolphindaq_linecam_Metrics_image_max_tag 8
#define dolphindaq_linecam_Metrics_image_min_tag 9
#define dolphindaq_linecam_Metrics_image_custom_tag 10
#define dolphindaq_linecam_Image_shot_num_tag    1
#define dolphindaq_linecam_Image_shot_time_tag   2
#define dolphindaq_linecam_Image_shot_time_alt_nanos_tag 3
#define dolphindaq_linecam_Image_yvals_tag       4
#define dolphindaq_linecam_Settings_start_shot_num_tag 1
#define dolphindaq_linecam_Settings_start_time_tag 2
#define dolphindaq_linecam_Settings_sensor_tag   3
#define dolphindaq_linecam_Settings_exposure_tag 4
#define dolphindaq_linecam_Settings_image_nx_tag 5
#define dolphindaq_linecam_Settings_metrics_batch_size_tag 6
#define dolphindaq_linecam_Settings_clk_dt_tag   7
#define dolphindaq_linecam_Settings_timtick_secs_tag 8
#define dolphindaq_linecam_Settings_clk_dt_timticks_tag 9
#define dolphindaq_linecam_Settings_exposure_clk_dts_tag 10
#define dolphindaq_linecam_Data_image_tag        1
#define dolphindaq_linecam_Data_metrics_tag      2

/* Struct field encoding specification for nanopb */
#define dolphindaq_linecam_Settings_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UINT64,   start_shot_num,    1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  start_time,        2) \
X(a, CALLBACK, OPTIONAL, STRING,   sensor,            3) \
X(a, STATIC,   OPTIONAL, DOUBLE,   exposure,          4) \
X(a, STATIC,   OPTIONAL, UINT32,   image_nx,          5) \
X(a, STATIC,   OPTIONAL, UINT32,   metrics_batch_size,   6) \
X(a, STATIC,   OPTIONAL, DOUBLE,   clk_dt,            7) \
X(a, STATIC,   OPTIONAL, DOUBLE,   timtick_secs,      8) \
X(a, STATIC,   OPTIONAL, UINT32,   clk_dt_timticks,   9) \
X(a, STATIC,   OPTIONAL, UINT32,   exposure_clk_dts,  10)
#define dolphindaq_linecam_Settings_CALLBACK pb_default_field_callback
#define dolphindaq_linecam_Settings_DEFAULT NULL
#define dolphindaq_linecam_Settings_start_time_MSGTYPE google_protobuf_Timestamp

#define dolphindaq_linecam_Data_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  image,             1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  metrics,           2)
#define dolphindaq_linecam_Data_CALLBACK NULL
#define dolphindaq_linecam_Data_DEFAULT NULL
#define dolphindaq_linecam_Data_image_MSGTYPE dolphindaq_linecam_Image
#define dolphindaq_linecam_Data_metrics_MSGTYPE dolphindaq_linecam_Metrics

#define dolphindaq_linecam_Image_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UINT64,   shot_num,          1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  shot_time,         2) \
X(a, STATIC,   OPTIONAL, UINT64,   shot_time_alt_nanos,   3) \
X(a, CALLBACK, OPTIONAL, BYTES,    yvals,             4)
#define dolphindaq_linecam_Image_CALLBACK pb_default_field_callback
#define dolphindaq_linecam_Image_DEFAULT NULL
#define dolphindaq_linecam_Image_shot_time_MSGTYPE google_protobuf_Timestamp

#define dolphindaq_linecam_Metrics_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, UINT64,   shot_num,          1) \
X(a, CALLBACK, REPEATED, MESSAGE,  shot_time,         2) \
X(a, CALLBACK, REPEATED, INT64,    shot_time_seconds,   3) \
X(a, CALLBACK, REPEATED, INT32,    shot_time_nanos,   4) \
X(a, CALLBACK, REPEATED, UINT64,   shot_time_alt_nanos,   5) \
X(a, CALLBACK, REPEATED, UINT64,   image_sum,         6) \
X(a, CALLBACK, REPEATED, DOUBLE,   image_mean,        7) \
X(a, CALLBACK, REPEATED, UINT64,   image_max,         8) \
X(a, CALLBACK, REPEATED, UINT64,   image_min,         9) \
X(a, CALLBACK, REPEATED, DOUBLE,   image_custom,     10)
#define dolphindaq_linecam_Metrics_CALLBACK pb_default_field_callback
#define dolphindaq_linecam_Metrics_DEFAULT NULL
#define dolphindaq_linecam_Metrics_shot_time_MSGTYPE google_protobuf_Timestamp

extern const pb_msgdesc_t dolphindaq_linecam_Settings_msg;
extern const pb_msgdesc_t dolphindaq_linecam_Data_msg;
extern const pb_msgdesc_t dolphindaq_linecam_Image_msg;
extern const pb_msgdesc_t dolphindaq_linecam_Metrics_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define dolphindaq_linecam_Settings_fields &dolphindaq_linecam_Settings_msg
#define dolphindaq_linecam_Data_fields &dolphindaq_linecam_Data_msg
#define dolphindaq_linecam_Image_fields &dolphindaq_linecam_Image_msg
#define dolphindaq_linecam_Metrics_fields &dolphindaq_linecam_Metrics_msg

/* Maximum encoded size of messages (where known) */
/* dolphindaq_linecam_Settings_size depends on runtime parameters */
/* dolphindaq_linecam_Data_size depends on runtime parameters */
/* dolphindaq_linecam_Image_size depends on runtime parameters */
/* dolphindaq_linecam_Metrics_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif