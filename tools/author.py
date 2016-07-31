from cpp_out import AccessDecl, MemberDecl, AccessGetImpl, AccessSetImpl, OffsetOfMember

def WriteDeclPOD(output_fn, struct_info):      
  output_fn('struct {}'.format(struct_info.name))
  output_fn('{')
  output_fn('public:')
  output_fn('{}();'.format(struct_info.name))
  for field_info in struct_info.members:
    AccessDecl(output_fn, struct_info, field_info)
    
  output_fn('')
  output_fn('static void MemberOffsets();')
  output_fn('')
  output_fn('private:')
  output_fn(' '.join(['NETWORK_TYPE _m_type = {', 'NETWORK_TYPE::'+struct_info.type, '};']))
  for field_info in struct_info.members:
    MemberDecl(output_fn, struct_info, field_info)
  output_fn('};')

def WriteImplPOD(output_fn, struct_info):
  for field_info in struct_info.members:
    AccessGetImpl(output_fn, struct_info, field_info)
    AccessSetImpl(output_fn, struct_info, field_info)
  SerializerImpl(output_fn, struct_info)
  
def WriteDeclHeaders(output_fn):
  output_fn('#pragma once')
  output_fn('#include <cstdint>')
  output_fn('#include <string>')
  output_fn('#include <cassert>')
  output_fn('#include <algorithm>')
  output_fn('#include <Vector3.hpp>')
  output_fn('\n')
  output_fn('#include "game_types.h"')
  output_fn('\n')
  
def WriteImplHeaders(output_fn, header):
  output_fn('#include "{}"'.format(header))
  output_fn('#include <cstring>')
  output_fn('#include <algorithm>')
  output_fn('#include <cstddef>')
  output_fn('')
  
def TypeAccessDecl(output_fn):
  output_fn('NETWORK_TYPE read_type(const void* buffer, size_t bytes_available);')
  output_fn('')
  
def MiscDecl(output_fn):
  output_fn('size_t get_checksum();')
  output_fn('constexpr size_t largest_message();')
  output_fn('')

def ForwardDecl(output_fn, structs):
  for struct in structs:
    output_fn('struct {};'.format(struct.name))
    
def TypeAccessImpl(output_fn):
  output_fn('std::vector<uint64_t> s_allOffsets;')
  output_fn('')
  output_fn("""NETWORK_TYPE read_type(const void* buffer, size_t bytes_available) {
  if (bytes_available < sizeof(NETWORK_TYPE)) return NETWORK_TYPE::UNKNOWN;

  NETWORK_TYPE* ref = (NETWORK_TYPE*)buffer;
  return *ref;
}
  """)
  output_fn("""template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
  """)
  
def ChecksumMemberOffsetImpl(output_fn, struct_info):
  output_fn('void {}::MemberOffsets()'.format(struct_info.name))
  output_fn('{')
  output_fn('  std::vector<uint64_t> offsets = {')
  for field_info in struct_info.members:
    OffsetOfMember(lambda s: output_fn('    '+s), struct_info, field_info)
  output_fn('  };')
  output_fn('  for (auto i : offsets) {')
  output_fn('    s_allOffsets.push_back(i);')
  output_fn('  }')
  output_fn('}')

def ChecksumImpl(output_fn, structs):
  output_fn('size_t checksum()')
  output_fn('{')
  for struct in structs:
    output_fn('  {}::MemberOffsets();'.format(struct.name))
  output_fn('  size_t seed = 0;')
  output_fn('  for (auto i : s_allOffsets) {')
  output_fn('    hash_combine(seed, i * 2654435761);')
  output_fn('  }')
  output_fn('  return seed;')
  output_fn('}')
  output_fn('size_t get_checksum()')
  output_fn('{')
  output_fn('  static size_t check = checksum();')
  output_fn('  return check;')
  output_fn('}')

def MessageSizeTemplate(output_fn, structs):
  output_fn("""template <typename... Args>
struct MaxStructSize;

template <typename First, typename... Args>
struct MaxStructSize<First, Args...>
{
  constexpr static size_t bytes()
  {
    return std::max(sizeof(First), MaxStructSize<Args...>::bytes());
  }
};

template <>
struct MaxStructSize<>
{
  constexpr static size_t bytes()
  {
    return 0;
  }
};

template <typename... Args>
constexpr size_t bytes()
{
  return MaxStructSize<Args...>::bytes();
}""")
  output_fn('constexpr size_t largest_message() {')
  output_fn('  return bytes<int')
  for struct in structs:
    output_fn('    ,{}'.format(struct.name))
  output_fn('  >();')
  output_fn('}')
  
def SerializerDecl(output_fn, struct_info):
  output_fn('size_t serialize(void* buffer, size_t buffer_size, const {}& net_msg);'.format(struct_info.name))
  output_fn('size_t deserialize(const void* buffer, size_t bytes_available, {}& out_msg);'.format(struct_info.name))
  
def SerializerImpl(output_fn, struct_info):
  output_fn('size_t serialize(void* buffer, size_t buffer_size, const {}& net_msg)'.format(struct_info.name))
  output_fn('{')
  output_fn('  if (buffer_size < sizeof(net_msg)) return 0;')
  output_fn('  memcpy(buffer, &net_msg, sizeof(net_msg));')
  output_fn('  return sizeof(net_msg);')
  output_fn('}')
  
  output_fn('size_t deserialize(const void* buffer, size_t bytes_available, {}& out_msg)'.format(struct_info.name))
  output_fn('{')
  output_fn('  if (bytes_available < sizeof(out_msg)) return 0;')
  output_fn('  memcpy(&out_msg, buffer, sizeof(out_msg));')
  output_fn('  return sizeof(out_msg);')
  output_fn('}')
  
  output_fn('{}::{}()'.format(struct_info.name, struct_info.name))
  index = 0
  for field_info in struct_info.members:
    if index:
      chr = ','
    else:
      chr = ':'
    index += 1
    output_fn('{} m_{}{}'.format(chr, field_info.name, field_info.initial_value))
  output_fn('{')
  output_fn('}')
  
