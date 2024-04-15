// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: simple.proto

#ifndef PROTOBUF_INCLUDED_simple_2eproto
#define PROTOBUF_INCLUDED_simple_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_simple_2eproto 

namespace protobuf_simple_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_simple_2eproto
class SimpleMessage;
class SimpleMessageDefaultTypeInternal;
extern SimpleMessageDefaultTypeInternal _SimpleMessage_default_instance_;
namespace google {
namespace protobuf {
template<> ::SimpleMessage* Arena::CreateMaybeMessage<::SimpleMessage>(Arena*);
}  // namespace protobuf
}  // namespace google

// ===================================================================

class SimpleMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:SimpleMessage) */ {
 public:
  SimpleMessage();
  virtual ~SimpleMessage();

  SimpleMessage(const SimpleMessage& from);

  inline SimpleMessage& operator=(const SimpleMessage& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  SimpleMessage(SimpleMessage&& from) noexcept
    : SimpleMessage() {
    *this = ::std::move(from);
  }

  inline SimpleMessage& operator=(SimpleMessage&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SimpleMessage& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const SimpleMessage* internal_default_instance() {
    return reinterpret_cast<const SimpleMessage*>(
               &_SimpleMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(SimpleMessage* other);
  friend void swap(SimpleMessage& a, SimpleMessage& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline SimpleMessage* New() const final {
    return CreateMaybeMessage<SimpleMessage>(NULL);
  }

  SimpleMessage* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<SimpleMessage>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const SimpleMessage& from);
  void MergeFrom(const SimpleMessage& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(SimpleMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int32 lucky_number = 1;
  bool has_lucky_number() const;
  void clear_lucky_number();
  static const int kLuckyNumberFieldNumber = 1;
  ::google::protobuf::int32 lucky_number() const;
  void set_lucky_number(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:SimpleMessage)
 private:
  void set_has_lucky_number();
  void clear_has_lucky_number();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  ::google::protobuf::int32 lucky_number_;
  friend struct ::protobuf_simple_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// SimpleMessage

// required int32 lucky_number = 1;
inline bool SimpleMessage::has_lucky_number() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SimpleMessage::set_has_lucky_number() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SimpleMessage::clear_has_lucky_number() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SimpleMessage::clear_lucky_number() {
  lucky_number_ = 0;
  clear_has_lucky_number();
}
inline ::google::protobuf::int32 SimpleMessage::lucky_number() const {
  // @@protoc_insertion_point(field_get:SimpleMessage.lucky_number)
  return lucky_number_;
}
inline void SimpleMessage::set_lucky_number(::google::protobuf::int32 value) {
  set_has_lucky_number();
  lucky_number_ = value;
  // @@protoc_insertion_point(field_set:SimpleMessage.lucky_number)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_simple_2eproto