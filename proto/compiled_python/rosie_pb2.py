# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: rosie.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import timestamp_pb2 as google_dot_protobuf_dot_timestamp__pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='rosie.proto',
  package='dolphindaq.rosie',
  syntax='proto2',
  serialized_options=None,
  serialized_pb=_b('\n\x0brosie.proto\x12\x10\x64olphindaq.rosie\x1a\x1fgoogle/protobuf/timestamp.proto\"n\n\x08Settings\x12\x16\n\x0estart_shot_num\x18\x01 \x01(\x04\x12.\n\nstart_time\x18\x02 \x01(\x0b\x32\x1a.google.protobuf.Timestamp\x12\x1a\n\x12metrics_batch_size\x18\x03 \x01(\r\"2\n\x04\x44\x61ta\x12*\n\x07metrics\x18\x01 \x01(\x0b\x32\x19.dolphindaq.rosie.Metrics\"\x9b\x01\n\x07Metrics\x12\x10\n\x08shot_num\x18\x01 \x03(\x04\x12-\n\tshot_time\x18\x02 \x03(\x0b\x32\x1a.google.protobuf.Timestamp\x12\x19\n\x11shot_time_seconds\x18\x03 \x03(\x03\x12\x17\n\x0fshot_time_nanos\x18\x04 \x03(\x05\x12\x1b\n\x13shot_time_alt_nanos\x18\x05 \x03(\x04')
  ,
  dependencies=[google_dot_protobuf_dot_timestamp__pb2.DESCRIPTOR,])




_SETTINGS = _descriptor.Descriptor(
  name='Settings',
  full_name='dolphindaq.rosie.Settings',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='start_shot_num', full_name='dolphindaq.rosie.Settings.start_shot_num', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='start_time', full_name='dolphindaq.rosie.Settings.start_time', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='metrics_batch_size', full_name='dolphindaq.rosie.Settings.metrics_batch_size', index=2,
      number=3, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=66,
  serialized_end=176,
)


_DATA = _descriptor.Descriptor(
  name='Data',
  full_name='dolphindaq.rosie.Data',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='metrics', full_name='dolphindaq.rosie.Data.metrics', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=178,
  serialized_end=228,
)


_METRICS = _descriptor.Descriptor(
  name='Metrics',
  full_name='dolphindaq.rosie.Metrics',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='shot_num', full_name='dolphindaq.rosie.Metrics.shot_num', index=0,
      number=1, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time', full_name='dolphindaq.rosie.Metrics.shot_time', index=1,
      number=2, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_seconds', full_name='dolphindaq.rosie.Metrics.shot_time_seconds', index=2,
      number=3, type=3, cpp_type=2, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_nanos', full_name='dolphindaq.rosie.Metrics.shot_time_nanos', index=3,
      number=4, type=5, cpp_type=1, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_alt_nanos', full_name='dolphindaq.rosie.Metrics.shot_time_alt_nanos', index=4,
      number=5, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=231,
  serialized_end=386,
)

_SETTINGS.fields_by_name['start_time'].message_type = google_dot_protobuf_dot_timestamp__pb2._TIMESTAMP
_DATA.fields_by_name['metrics'].message_type = _METRICS
_METRICS.fields_by_name['shot_time'].message_type = google_dot_protobuf_dot_timestamp__pb2._TIMESTAMP
DESCRIPTOR.message_types_by_name['Settings'] = _SETTINGS
DESCRIPTOR.message_types_by_name['Data'] = _DATA
DESCRIPTOR.message_types_by_name['Metrics'] = _METRICS
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

Settings = _reflection.GeneratedProtocolMessageType('Settings', (_message.Message,), dict(
  DESCRIPTOR = _SETTINGS,
  __module__ = 'rosie_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.rosie.Settings)
  ))
_sym_db.RegisterMessage(Settings)

Data = _reflection.GeneratedProtocolMessageType('Data', (_message.Message,), dict(
  DESCRIPTOR = _DATA,
  __module__ = 'rosie_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.rosie.Data)
  ))
_sym_db.RegisterMessage(Data)

Metrics = _reflection.GeneratedProtocolMessageType('Metrics', (_message.Message,), dict(
  DESCRIPTOR = _METRICS,
  __module__ = 'rosie_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.rosie.Metrics)
  ))
_sym_db.RegisterMessage(Metrics)


# @@protoc_insertion_point(module_scope)
