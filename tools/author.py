from cpp_out import AccessDecl, MemberDecl, AccessGetImpl, AccessSetImpl

def WriteDeclPOD(output_fn, struct_info):
  # for member_type, member_name in struct_info.members.items():
    # if member_type not in types and not member_type.endswith('_TYPE'): #TODO: enum support?
      # raise Exception('Unknown user type: ' + member_type)
      
  output_fn('struct {}'.format(struct_info.name))
  output_fn('{')
  output_fn('public:')
  output_fn('{}();'.format(struct_info.name))
  for field_name, field_info in struct_info.members.items():
    AccessDecl(output_fn, struct_info, field_info)
    
  output_fn('')
  output_fn('private:')
  output_fn(' '.join(['NETWORK_TYPE _m_type = {', 'NETWORK_TYPE::'+struct_info.type, '};']))
  for field_name, field_info in struct_info.members.items():
    MemberDecl(output_fn, struct_info, field_info)
  output_fn('};')

def WriteImplPOD(output_fn, struct_info):
  for field_name, field_info in struct_info.members.items():
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
  output_fn('')
  
def TypeAccessDecl(output_fn):
  output_fn('NETWORK_TYPE read_type(const void* buffer, size_t bytes_available);')
  output_fn('')

def TypeAccessImpl(output_fn):
  output_fn("""NETWORK_TYPE read_type(const void* buffer, size_t bytes_available) {
  if (bytes_available < sizeof(NETWORK_TYPE)) return NETWORK_TYPE::UNKNOWN;

  NETWORK_TYPE* ref = (NETWORK_TYPE*)buffer;
  return *ref;
}
  """)
  
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
  output_fn('{')
  output_fn('  char* members = (char*)this+sizeof(_m_type);')
  output_fn('  memset(members, 0, sizeof({})-sizeof(_m_type));'.format(struct_info.name))
  output_fn('}')
  