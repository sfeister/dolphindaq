# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: linecam.proto

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
  name='linecam.proto',
  package='dolphindaq.linecam',
  syntax='proto2',
  serialized_options=None,
  serialized_pb=_b('\n\rlinecam.proto\x12\x12\x64olphindaq.linecam\x1a\x1fgoogle/protobuf/timestamp.proto\"\x9b\x02\n\x08Settings\x12\x16\n\x0estart_shot_num\x18\x01 \x01(\x04\x12.\n\nstart_time\x18\x02 \x01(\x0b\x32\x1a.google.protobuf.Timestamp\x12\x0e\n\x06sensor\x18\x03 \x01(\t\x12\x10\n\x08\x65xposure\x18\x04 \x01(\x01\x12\x10\n\x08image_nx\x18\x05 \x01(\r\x12\x1a\n\x12metrics_batch_size\x18\x06 \x01(\r\x12\x0e\n\x06\x63lk_dt\x18\x07 \x01(\x01\x12\x14\n\x0ctimtick_secs\x18\x08 \x01(\x01\x12\x17\n\x0f\x63lk_dt_timticks\x18\t \x01(\r\x12\x18\n\x10\x65xposure_clk_dts\x18\n \x01(\r\x12\n\n\x02t1\x18\x0b \x01(\x01\x12\x12\n\nt1_clk_dts\x18\x0c \x01(\r\"^\n\x04\x44\x61ta\x12(\n\x05image\x18\x01 \x01(\x0b\x32\x19.dolphindaq.linecam.Image\x12,\n\x07metrics\x18\x02 \x01(\x0b\x32\x1b.dolphindaq.linecam.Metrics\"t\n\x05Image\x12\x10\n\x08shot_num\x18\x01 \x01(\x04\x12-\n\tshot_time\x18\x02 \x01(\x0b\x32\x1a.google.protobuf.Timestamp\x12\x1b\n\x13shot_time_alt_nanos\x18\x03 \x01(\x04\x12\r\n\x05yvals\x18\x04 \x01(\x0c\"\xfe\x01\n\x07Metrics\x12\x10\n\x08shot_num\x18\x01 \x03(\x04\x12-\n\tshot_time\x18\x02 \x03(\x0b\x32\x1a.google.protobuf.Timestamp\x12\x19\n\x11shot_time_seconds\x18\x03 \x03(\x03\x12\x17\n\x0fshot_time_nanos\x18\x04 \x03(\x05\x12\x1b\n\x13shot_time_alt_nanos\x18\x05 \x03(\x04\x12\x11\n\timage_sum\x18\x06 \x03(\x04\x12\x12\n\nimage_mean\x18\x07 \x03(\x01\x12\x11\n\timage_max\x18\x08 \x03(\x04\x12\x11\n\timage_min\x18\t \x03(\x04\x12\x14\n\x0cimage_custom\x18\n \x03(\x01')
  ,
  dependencies=[google_dot_protobuf_dot_timestamp__pb2.DESCRIPTOR,])




_SETTINGS = _descriptor.Descriptor(
  name='Settings',
  full_name='dolphindaq.linecam.Settings',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='start_shot_num', full_name='dolphindaq.linecam.Settings.start_shot_num', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='start_time', full_name='dolphindaq.linecam.Settings.start_time', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='sensor', full_name='dolphindaq.linecam.Settings.sensor', index=2,
      number=3, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='exposure', full_name='dolphindaq.linecam.Settings.exposure', index=3,
      number=4, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_nx', full_name='dolphindaq.linecam.Settings.image_nx', index=4,
      number=5, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='metrics_batch_size', full_name='dolphindaq.linecam.Settings.metrics_batch_size', index=5,
      number=6, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='clk_dt', full_name='dolphindaq.linecam.Settings.clk_dt', index=6,
      number=7, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='timtick_secs', full_name='dolphindaq.linecam.Settings.timtick_secs', index=7,
      number=8, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='clk_dt_timticks', full_name='dolphindaq.linecam.Settings.clk_dt_timticks', index=8,
      number=9, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='exposure_clk_dts', full_name='dolphindaq.linecam.Settings.exposure_clk_dts', index=9,
      number=10, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='t1', full_name='dolphindaq.linecam.Settings.t1', index=10,
      number=11, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='t1_clk_dts', full_name='dolphindaq.linecam.Settings.t1_clk_dts', index=11,
      number=12, type=13, cpp_type=3, label=1,
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
  serialized_start=71,
  serialized_end=354,
)


_DATA = _descriptor.Descriptor(
  name='Data',
  full_name='dolphindaq.linecam.Data',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='image', full_name='dolphindaq.linecam.Data.image', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='metrics', full_name='dolphindaq.linecam.Data.metrics', index=1,
      number=2, type=11, cpp_type=10, label=1,
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
  serialized_start=356,
  serialized_end=450,
)


_IMAGE = _descriptor.Descriptor(
  name='Image',
  full_name='dolphindaq.linecam.Image',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='shot_num', full_name='dolphindaq.linecam.Image.shot_num', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time', full_name='dolphindaq.linecam.Image.shot_time', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_alt_nanos', full_name='dolphindaq.linecam.Image.shot_time_alt_nanos', index=2,
      number=3, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='yvals', full_name='dolphindaq.linecam.Image.yvals', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
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
  serialized_start=452,
  serialized_end=568,
)


_METRICS = _descriptor.Descriptor(
  name='Metrics',
  full_name='dolphindaq.linecam.Metrics',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='shot_num', full_name='dolphindaq.linecam.Metrics.shot_num', index=0,
      number=1, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time', full_name='dolphindaq.linecam.Metrics.shot_time', index=1,
      number=2, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_seconds', full_name='dolphindaq.linecam.Metrics.shot_time_seconds', index=2,
      number=3, type=3, cpp_type=2, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_nanos', full_name='dolphindaq.linecam.Metrics.shot_time_nanos', index=3,
      number=4, type=5, cpp_type=1, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='shot_time_alt_nanos', full_name='dolphindaq.linecam.Metrics.shot_time_alt_nanos', index=4,
      number=5, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_sum', full_name='dolphindaq.linecam.Metrics.image_sum', index=5,
      number=6, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_mean', full_name='dolphindaq.linecam.Metrics.image_mean', index=6,
      number=7, type=1, cpp_type=5, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_max', full_name='dolphindaq.linecam.Metrics.image_max', index=7,
      number=8, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_min', full_name='dolphindaq.linecam.Metrics.image_min', index=8,
      number=9, type=4, cpp_type=4, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='image_custom', full_name='dolphindaq.linecam.Metrics.image_custom', index=9,
      number=10, type=1, cpp_type=5, label=3,
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
  serialized_start=571,
  serialized_end=825,
)

_SETTINGS.fields_by_name['start_time'].message_type = google_dot_protobuf_dot_timestamp__pb2._TIMESTAMP
_DATA.fields_by_name['image'].message_type = _IMAGE
_DATA.fields_by_name['metrics'].message_type = _METRICS
_IMAGE.fields_by_name['shot_time'].message_type = google_dot_protobuf_dot_timestamp__pb2._TIMESTAMP
_METRICS.fields_by_name['shot_time'].message_type = google_dot_protobuf_dot_timestamp__pb2._TIMESTAMP
DESCRIPTOR.message_types_by_name['Settings'] = _SETTINGS
DESCRIPTOR.message_types_by_name['Data'] = _DATA
DESCRIPTOR.message_types_by_name['Image'] = _IMAGE
DESCRIPTOR.message_types_by_name['Metrics'] = _METRICS
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

Settings = _reflection.GeneratedProtocolMessageType('Settings', (_message.Message,), dict(
  DESCRIPTOR = _SETTINGS,
  __module__ = 'linecam_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.linecam.Settings)
  ))
_sym_db.RegisterMessage(Settings)

Data = _reflection.GeneratedProtocolMessageType('Data', (_message.Message,), dict(
  DESCRIPTOR = _DATA,
  __module__ = 'linecam_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.linecam.Data)
  ))
_sym_db.RegisterMessage(Data)

Image = _reflection.GeneratedProtocolMessageType('Image', (_message.Message,), dict(
  DESCRIPTOR = _IMAGE,
  __module__ = 'linecam_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.linecam.Image)
  ))
_sym_db.RegisterMessage(Image)

Metrics = _reflection.GeneratedProtocolMessageType('Metrics', (_message.Message,), dict(
  DESCRIPTOR = _METRICS,
  __module__ = 'linecam_pb2'
  # @@protoc_insertion_point(class_scope:dolphindaq.linecam.Metrics)
  ))
_sym_db.RegisterMessage(Metrics)


# @@protoc_insertion_point(module_scope)
