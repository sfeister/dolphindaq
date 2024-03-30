/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.1 */

#ifndef PB_GOOGLE_PROTOBUF_TIMESTAMP_PB_H_INCLUDED
#define PB_GOOGLE_PROTOBUF_TIMESTAMP_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _google_protobuf_Timestamp {
    int64_t seconds;
    int32_t nanos;
} google_protobuf_Timestamp;


/* Initializer values for message structs */
#define google_protobuf_Timestamp_init_default   {0, 0}
#define google_protobuf_Timestamp_init_zero      {0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define google_protobuf_Timestamp_seconds_tag    1
#define google_protobuf_Timestamp_nanos_tag      2

/* Struct field encoding specification for nanopb */
#define google_protobuf_Timestamp_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    seconds,           1) \
X(a, STATIC,   SINGULAR, INT32,    nanos,             2)
#define google_protobuf_Timestamp_CALLBACK NULL
#define google_protobuf_Timestamp_DEFAULT NULL

extern const pb_msgdesc_t google_protobuf_Timestamp_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define google_protobuf_Timestamp_fields &google_protobuf_Timestamp_msg

/* Maximum encoded size of messages (where known) */
#define google_protobuf_Timestamp_size           22

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
