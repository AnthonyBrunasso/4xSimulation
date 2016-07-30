def AccessDecl(output_fn, struct_info, field_info):
  output_fn('void set_{}(const {}&);'.format(field_info.name, field_info.access_type))
  output_fn('{} get_{}() const;'.format(field_info.access_type, field_info.name))

def MemberDecl(output_fn, struct_info, field_info):
  if field_info.storage_count > 1:
    output_fn('{} m_{}[{}];'.format(field_info.storage_type, field_info.name, field_info.storage_count))
  else:
    output_fn('{} m_{};'.format(field_info.storage_type, field_info.name))
    
def AccessGetImpl(output_fn, struct_info, field_info):
  output_fn('{} {}::get_{}() const'.format(field_info.access_type, struct_info.name, field_info.name))
  output_fn('{')
  if field_info.accessor is None:
    output_fn('  return m_{};'.format(field_info.name))
  else:
    field_info.accessor.getter(lambda f: output_fn('  '+f), struct_info, field_info)
  output_fn('}')
  
def AccessSetImpl(output_fn, struct_info, field_info):
  output_fn('void {}::set_{}(const {}& {})'.format(struct_info.name, field_info.name, field_info.access_type, field_info.name))
  output_fn('{')
  if field_info.accessor is None:
    output_fn('  m_{} = {};'.format(field_info.name, field_info.name))
  else:
    field_info.accessor.setter(lambda f: output_fn('  '+f), struct_info, field_info)
  output_fn('}')

def OffsetOfMember(output_fn, struct_info, field_info):
  output_fn('offsetof({}, m_{}),'.format(struct_info.name, field_info.name))