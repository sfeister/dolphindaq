/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: simple.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "simple.pb-c.h"
void   simple_message__init
                     (SimpleMessage         *message)
{
  static const SimpleMessage init_value = SIMPLE_MESSAGE__INIT;
  *message = init_value;
}
size_t simple_message__get_packed_size
                     (const SimpleMessage *message)
{
  assert(message->base.descriptor == &simple_message__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t simple_message__pack
                     (const SimpleMessage *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &simple_message__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t simple_message__pack_to_buffer
                     (const SimpleMessage *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &simple_message__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
SimpleMessage *
       simple_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (SimpleMessage *)
     protobuf_c_message_unpack (&simple_message__descriptor,
                                allocator, len, data);
}
void   simple_message__free_unpacked
                     (SimpleMessage *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &simple_message__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor simple_message__field_descriptors[1] =
{
  {
    "lucky_number",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(SimpleMessage, lucky_number),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned simple_message__field_indices_by_name[] = {
  0,   /* field[0] = lucky_number */
};
static const ProtobufCIntRange simple_message__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor simple_message__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "SimpleMessage",
  "SimpleMessage",
  "SimpleMessage",
  "",
  sizeof(SimpleMessage),
  1,
  simple_message__field_descriptors,
  simple_message__field_indices_by_name,
  1,  simple_message__number_ranges,
  (ProtobufCMessageInit) simple_message__init,
  NULL,NULL,NULL    /* reserved[123] */
};
