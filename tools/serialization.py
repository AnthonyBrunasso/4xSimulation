import sys
sys.path.append('../data')
import store
import type_generator
import network_definitions

# Output type enumerations
import type_decl
import author
def NetworkEnum():
  NETWORK = "NETWORK"
  
  for i, struct in enumerate(type_decl.structs.values()):
    typename = store.add_gametype(NETWORK, struct.name, i+1)
    struct.type = typename
    
  type_generator.write_file('../include/game_types.h')
  
if __name__ == "__main__":
  NetworkEnum()

  # Output POD structures
  with open('../include/network_types.h', 'w') as f:
    writer = lambda text: f.write(text+'\n')

    author.WriteDeclHeaders(writer)
    author.TypeAccessDecl(writer)

    for name, struct in type_decl.structs.items():
      author.WriteDeclPOD(writer, struct)

    writer('')
    for name, struct in type_decl.structs.items():
      author.SerializerDecl(writer, struct)
    
  with open('../src/network_types.cpp', 'w') as f:
    writer = lambda text: f.write(text+'\n')
    
    author.WriteImplHeaders(writer, 'network_types.h')
    
    author.TypeAccessImpl(writer)
    for name, struct in type_decl.structs.items():
      author.WriteImplPOD(writer, struct)
