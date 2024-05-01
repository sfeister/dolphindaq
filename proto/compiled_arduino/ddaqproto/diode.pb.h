/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.1 */

#ifndef PB_DOLPHINDAQ_DIODE_DIODE_PB_H_INCLUDED
#define PB_DOLPHINDAQ_DIODE_DIODE_PB_H_INCLUDED
#include <pb.h>
#include "timestamp.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _dolphindaq_diode_Metrics {
    pb_callback_t shot_num;
    pb_callback_t shot_time;
    pb_callback_t shot_time_seconds;
    pb_callback_t shot_time_nanos;
    pb_callback_t shot_time_alt_nanos;
    pb_callback_t trace_sum;
    pb_callback_t trace_mean;
    pb_callback_t trace_max;
    pb_callback_t trace_min;
    pb_callback_t trace_custom;
    pb_callback_t bg_mean;
    pb_callback_t signal_mean;
    pb_callback_t signal_max;
    pb_callback_t reduced_integ;
    pb_callback_t reduced_mean;
    pb_callback_t reduced_max;
} dolphindaq_diode_Metrics;

typedef struct _dolphindaq_diode_Settings {
    bool has_start_shot_num;
    uint64_t start_shot_num;
    bool has_start_time;
    google_protobuf_Timestamp start_time;
    pb_callback_t trace_tvals;
    bool has_trace_dt;
    double trace_dt;
    bool has_trace_nt;
    uint32_t trace_nt;
    bool has_trace_ymin;
    double trace_ymin;
    bool has_trace_ymax;
    double trace_ymax;
    bool has_metrics_batch_size;
    uint32_t metrics_batch_size;
    bool has_dt;
    double dt;
    bool has_t1;
    double t1;
    bool has_t2;
    double t2;
    bool has_t3;
    double t3;
    bool has_t4;
    double t4;
    bool has_timtick_secs;
    double timtick_secs;
    bool has_dt_timticks;
    uint32_t dt_timticks;
    bool has_t1_dts;
    uint32_t t1_dts;
    bool has_t2_dts;
    uint32_t t2_dts;
    bool has_t3_dts;
    uint32_t t3_dts;
    bool has_t4_dts;
    uint32_t t4_dts;
} dolphindaq_diode_Settings;

typedef struct _dolphindaq_diode_Trace {
    bool has_shot_num;
    uint64_t shot_num;
    bool has_shot_time;
    google_protobuf_Timestamp shot_time;
    pb_callback_t yvals;
} dolphindaq_diode_Trace;

typedef struct _dolphindaq_diode_Data {
    bool has_trace;
    dolphindaq_diode_Trace trace;
    bool has_metrics;
    dolphindaq_diode_Metrics metrics;
} dolphindaq_diode_Data;


/* Initializer values for message structs */
#define dolphindaq_diode_Settings_init_default   {false, 0, false, google_protobuf_Timestamp_init_default, {{NULL}, NULL}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define dolphindaq_diode_Data_init_default       {false, dolphindaq_diode_Trace_init_default, false, dolphindaq_diode_Metrics_init_default}
#define dolphindaq_diode_Trace_init_default      {false, 0, false, google_protobuf_Timestamp_init_default, {{NULL}, NULL}}
#define dolphindaq_diode_Metrics_init_default    {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}
#define dolphindaq_diode_Settings_init_zero      {false, 0, false, google_protobuf_Timestamp_init_zero, {{NULL}, NULL}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define dolphindaq_diode_Data_init_zero          {false, dolphindaq_diode_Trace_init_zero, false, dolphindaq_diode_Metrics_init_zero}
#define dolphindaq_diode_Trace_init_zero         {false, 0, false, google_protobuf_Timestamp_init_zero, {{NULL}, NULL}}
#define dolphindaq_diode_Metrics_init_zero       {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define dolphindaq_diode_Metrics_shot_num_tag    1
#define dolphindaq_diode_Metrics_shot_time_tag   2
#define dolphindaq_diode_Metrics_shot_time_seconds_tag 3
#define dolphindaq_diode_Metrics_shot_time_nanos_tag 4
#define dolphindaq_diode_Metrics_shot_time_alt_nanos_tag 5
#define dolphindaq_diode_Metrics_trace_sum_tag   6
#define dolphindaq_diode_Metrics_trace_mean_tag  7
#define dolphindaq_diode_Metrics_trace_max_tag   8
#define dolphindaq_diode_Metrics_trace_min_tag   9
#define dolphindaq_diode_Metrics_trace_custom_tag 10
#define dolphindaq_diode_Metrics_bg_mean_tag     11
#define dolphindaq_diode_Metrics_signal_mean_tag 12
#define dolphindaq_diode_Metrics_signal_max_tag  13
#define dolphindaq_diode_Metrics_reduced_integ_tag 14
#define dolphindaq_diode_Metrics_reduced_mean_tag 15
#define dolphindaq_diode_Metrics_reduced_max_tag 16
#define dolphindaq_diode_Settings_start_shot_num_tag 1
#define dolphindaq_diode_Settings_start_time_tag 2
#define dolphindaq_diode_Settings_trace_tvals_tag 3
#define dolphindaq_diode_Settings_trace_dt_tag   4
#define dolphindaq_diode_Settings_trace_nt_tag   5
#define dolphindaq_diode_Settings_trace_ymin_tag 6
#define dolphindaq_diode_Settings_trace_ymax_tag 7
#define dolphindaq_diode_Settings_metrics_batch_size_tag 8
#define dolphindaq_diode_Settings_dt_tag         9
#define dolphindaq_diode_Settings_t1_tag         10
#define dolphindaq_diode_Settings_t2_tag         11
#define dolphindaq_diode_Settings_t3_tag         12
#define dolphindaq_diode_Settings_t4_tag         13
#define dolphindaq_diode_Settings_timtick_secs_tag 14
#define dolphindaq_diode_Settings_dt_timticks_tag 15
#define dolphindaq_diode_Settings_t1_dts_tag     16
#define dolphindaq_diode_Settings_t2_dts_tag     17
#define dolphindaq_diode_Settings_t3_dts_tag     18
#define dolphindaq_diode_Settings_t4_dts_tag     19
#define dolphindaq_diode_Trace_shot_num_tag      1
#define dolphindaq_diode_Trace_shot_time_tag     2
#define dolphindaq_diode_Trace_yvals_tag         3
#define dolphindaq_diode_Data_trace_tag          1
#define dolphindaq_diode_Data_metrics_tag        2

/* Struct field encoding specification for nanopb */
#define dolphindaq_diode_Settings_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UINT64,   start_shot_num,    1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  start_time,        2) \
X(a, CALLBACK, REPEATED, DOUBLE,   trace_tvals,       3) \
X(a, STATIC,   OPTIONAL, DOUBLE,   trace_dt,          4) \
X(a, STATIC,   OPTIONAL, UINT32,   trace_nt,          5) \
X(a, STATIC,   OPTIONAL, DOUBLE,   trace_ymin,        6) \
X(a, STATIC,   OPTIONAL, DOUBLE,   trace_ymax,        7) \
X(a, STATIC,   OPTIONAL, UINT32,   metrics_batch_size,   8) \
X(a, STATIC,   OPTIONAL, DOUBLE,   dt,                9) \
X(a, STATIC,   OPTIONAL, DOUBLE,   t1,               10) \
X(a, STATIC,   OPTIONAL, DOUBLE,   t2,               11) \
X(a, STATIC,   OPTIONAL, DOUBLE,   t3,               12) \
X(a, STATIC,   OPTIONAL, DOUBLE,   t4,               13) \
X(a, STATIC,   OPTIONAL, DOUBLE,   timtick_secs,     14) \
X(a, STATIC,   OPTIONAL, UINT32,   dt_timticks,      15) \
X(a, STATIC,   OPTIONAL, UINT32,   t1_dts,           16) \
X(a, STATIC,   OPTIONAL, UINT32,   t2_dts,           17) \
X(a, STATIC,   OPTIONAL, UINT32,   t3_dts,           18) \
X(a, STATIC,   OPTIONAL, UINT32,   t4_dts,           19)
#define dolphindaq_diode_Settings_CALLBACK pb_default_field_callback
#define dolphindaq_diode_Settings_DEFAULT NULL
#define dolphindaq_diode_Settings_start_time_MSGTYPE google_protobuf_Timestamp

#define dolphindaq_diode_Data_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  trace,             1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  metrics,           2)
#define dolphindaq_diode_Data_CALLBACK NULL
#define dolphindaq_diode_Data_DEFAULT NULL
#define dolphindaq_diode_Data_trace_MSGTYPE dolphindaq_diode_Trace
#define dolphindaq_diode_Data_metrics_MSGTYPE dolphindaq_diode_Metrics

#define dolphindaq_diode_Trace_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UINT64,   shot_num,          1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  shot_time,         2) \
X(a, CALLBACK, OPTIONAL, BYTES,    yvals,             3)
#define dolphindaq_diode_Trace_CALLBACK pb_default_field_callback
#define dolphindaq_diode_Trace_DEFAULT NULL
#define dolphindaq_diode_Trace_shot_time_MSGTYPE google_protobuf_Timestamp

#define dolphindaq_diode_Metrics_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, UINT64,   shot_num,          1) \
X(a, CALLBACK, REPEATED, MESSAGE,  shot_time,         2) \
X(a, CALLBACK, REPEATED, INT64,    shot_time_seconds,   3) \
X(a, CALLBACK, REPEATED, INT32,    shot_time_nanos,   4) \
X(a, CALLBACK, REPEATED, UINT64,   shot_time_alt_nanos,   5) \
X(a, CALLBACK, REPEATED, UINT64,   trace_sum,         6) \
X(a, CALLBACK, REPEATED, DOUBLE,   trace_mean,        7) \
X(a, CALLBACK, REPEATED, UINT64,   trace_max,         8) \
X(a, CALLBACK, REPEATED, UINT64,   trace_min,         9) \
X(a, CALLBACK, REPEATED, DOUBLE,   trace_custom,     10) \
X(a, CALLBACK, REPEATED, DOUBLE,   bg_mean,          11) \
X(a, CALLBACK, REPEATED, DOUBLE,   signal_mean,      12) \
X(a, CALLBACK, REPEATED, DOUBLE,   signal_max,       13) \
X(a, CALLBACK, REPEATED, DOUBLE,   reduced_integ,    14) \
X(a, CALLBACK, REPEATED, DOUBLE,   reduced_mean,     15) \
X(a, CALLBACK, REPEATED, DOUBLE,   reduced_max,      16)
#define dolphindaq_diode_Metrics_CALLBACK pb_default_field_callback
#define dolphindaq_diode_Metrics_DEFAULT NULL
#define dolphindaq_diode_Metrics_shot_time_MSGTYPE google_protobuf_Timestamp

extern const pb_msgdesc_t dolphindaq_diode_Settings_msg;
extern const pb_msgdesc_t dolphindaq_diode_Data_msg;
extern const pb_msgdesc_t dolphindaq_diode_Trace_msg;
extern const pb_msgdesc_t dolphindaq_diode_Metrics_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define dolphindaq_diode_Settings_fields &dolphindaq_diode_Settings_msg
#define dolphindaq_diode_Data_fields &dolphindaq_diode_Data_msg
#define dolphindaq_diode_Trace_fields &dolphindaq_diode_Trace_msg
#define dolphindaq_diode_Metrics_fields &dolphindaq_diode_Metrics_msg

/* Maximum encoded size of messages (where known) */
/* dolphindaq_diode_Settings_size depends on runtime parameters */
/* dolphindaq_diode_Data_size depends on runtime parameters */
/* dolphindaq_diode_Trace_size depends on runtime parameters */
/* dolphindaq_diode_Metrics_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
