
structs = {}

class StructInfo:
  def __init__(self, name):
    self.members = {}
    self.name = name
    self.type = None

class FieldInfo:
  def __init__(self, parent, storage_type, access_type, field_name, count):
    self.parent = parent
    self.storage_type = storage_type
    self.access_type = access_type
    self.name = field_name
    self.storage_count = count
    self.accessor = None
    
class AccessorInfo:
  def __init__(self, getter=None, setter=None):
    self.getter = getter
    self.setter = setter

class TypeInfo:
  def __init__(self, type_name, access_type=None, storage_type=None, storage_count=1, getter=None, setter=None):
    self.name = type_name
    if access_type is not None:
      self.access_type = access_type
    else:
      self.access_type = type_name
    if storage_type is not None:
      self.storage_type = storage_type
    else:
      self.storage_type = type_name
      
    self.storage_count = storage_count
    self.getter = getter
    self.setter = setter

def Struct(name, members={}):
  global structs
  if not name in structs:
    structs[name] = StructInfo(name)
  struct_info = structs[name]
  for type, name in members:
    Field(struct_info, type, name) 
  return struct_info
  
def CustomField(parent, storage_type, access_type, field_name, count=1):
  parent.members[field_name] = FieldInfo(parent, storage_type, access_type, field_name, count)
  return parent.members[field_name]
  
def Field(parent, type, field_name):
  field = FieldInfo(parent, type.storage_type, type.access_type, field_name, type.storage_count)
  if type.getter and type.setter:
    Accessor(field, type.getter, type.setter)
  parent.members[field_name] = field
  return field
  
def Accessor(parent, getter, setter):
  parent.accessor = AccessorInfo(getter, setter)
