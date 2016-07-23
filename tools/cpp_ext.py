def GetVector3i(output_fn, struct_info, field_info):
  output_fn('  return std::move(sf::Vector3i(m_{}[0], m_{}[1], m_{}[2]));'.format(field_info.name, field_info.name, field_info.name))
  
def SetVector3i(output_fn, struct_info, field_info):
  output_fn('m_{}[0] = {}.x;'.format(field_info.name, field_info.name))
  output_fn('m_{}[1] = {}.y;'.format(field_info.name, field_info.name))
  output_fn('m_{}[2] = {}.z;'.format(field_info.name, field_info.name))
  
def GetString(output_fn, struct_info, field_info):
  output_fn('return std::move(std::string(m_{}));'.format(field_info.name))

def SetString(output_fn, struct_info, field_info):
  output_fn('size_t len = std::min<size_t>({}-1, {}.size());'.format(field_info.storage_count, field_info.name))
  output_fn('memcpy(m_{}, {}.c_str(), len);'.format(field_info.name, field_info.name, field_info.name))
  output_fn('m_{}[len] = 0;'.format(field_info.name, field_info.storage_count))

def GetBool(output_fn, struct_info, field_info):
  output_fn('return m_{}?true:false;'.format(field_info.name))
  
def SetBool(output_fn, struct_info, field_info):
  output_fn('m_{} = {}?1:0;'.format(field_info.name, field_info.name))

