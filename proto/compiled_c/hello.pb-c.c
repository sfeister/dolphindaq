/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: hello.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "hello.pb-c.h"
void   dolphindaq__hello__init
                     (Dolphindaq__Hello         *message)
{
  static const Dolphindaq__Hello init_value = DOLPHINDAQ__HELLO__INIT;
  *message = init_value;
}
size_t dolphindaq__hello__get_packed_size
                     (const Dolphindaq__Hello *message)
{
  assert(message->base.descriptor == &dolphindaq__hello__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t dolphindaq__hello__pack
                     (const Dolphindaq__Hello *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &dolphindaq__hello__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t dolphindaq__hello__pack_to_buffer
                     (const Dolphindaq__Hello *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &dolphindaq__hello__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Dolphindaq__Hello *
       dolphindaq__hello__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Dolphindaq__Hello *)
     protobuf_c_message_unpack (&dolphindaq__hello__descriptor,
                                allocator, len, data);
}
void   dolphindaq__hello__free_unpacked
                     (Dolphindaq__Hello *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &dolphindaq__hello__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCEnumValue dolphindaq__hello__device_type__enum_values_by_number[3] =
{
  { "OTHER", "DOLPHINDAQ__HELLO__DEVICE_TYPE__OTHER", 0 },
  { "DIODE", "DOLPHINDAQ__HELLO__DEVICE_TYPE__DIODE", 1 },
  { "LINECAM", "DOLPHINDAQ__HELLO__DEVICE_TYPE__LINECAM", 2 },
};
static const ProtobufCIntRange dolphindaq__hello__device_type__value_ranges[] = {
{0, 0},{0, 3}
};
static const ProtobufCEnumValueIndex dolphindaq__hello__device_type__enum_values_by_name[3] =
{
  { "DIODE", 1 },
  { "LINECAM", 2 },
  { "OTHER", 0 },
};
const ProtobufCEnumDescriptor dolphindaq__hello__device_type__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "dolphindaq.Hello.DeviceType",
  "DeviceType",
  "Dolphindaq__Hello__DeviceType",
  "dolphindaq",
  3,
  dolphindaq__hello__device_type__enum_values_by_number,
  3,
  dolphindaq__hello__device_type__enum_values_by_name,
  1,
  dolphindaq__hello__device_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
static const ProtobufCFieldDescriptor dolphindaq__hello__field_descriptors[3] =
{
  {
    "unique_name",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Dolphindaq__Hello, unique_name),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "unique_id",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(Dolphindaq__Hello, unique_id),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "device_type",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(Dolphindaq__Hello, device_type),
    &dolphindaq__hello__device_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned dolphindaq__hello__field_indices_by_name[] = {
  2,   /* field[2] = device_type */
  1,   /* field[1] = unique_id */
  0,   /* field[0] = unique_name */
};
static const ProtobufCIntRange dolphindaq__hello__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor dolphindaq__hello__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "dolphindaq.Hello",
  "Hello",
  "Dolphindaq__Hello",
  "dolphindaq",
  sizeof(Dolphindaq__Hello),
  3,
  dolphindaq__hello__field_descriptors,
  dolphindaq__hello__field_indices_by_name,
  1,  dolphindaq__hello__number_ranges,
  (ProtobufCMessageInit) dolphindaq__hello__init,
  NULL,NULL,NULL    /* reserved[123] */
};
