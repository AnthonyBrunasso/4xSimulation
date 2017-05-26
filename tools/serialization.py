import sys
sys.path.append('../data')
import store
import type_generator
import network_definitions
from optparse import OptionParser

# Output type enumerations
import type_decl
import author
def NetworkEnum():
  NETWORK = "NETWORK"
  
  for i, struct in enumerate(type_decl.structs):
    typename = store.add_gametype(NETWORK, struct.name, i+1)
    struct.type = typename
    
  type_generator.write_file('../include/game_types.h')
  
if __name__ == "__main__":
  parser = OptionParser()
  parser.add_option("-n", "--network_types", action='store_true', default=False, help="Generate legacy networking types")

  (options, args) = parser.parse_args()

  NetworkEnum()

  if options.network_types:
      # Output POD structures
      with open('../include/network_types.h', 'w') as f:
        writer = lambda text: f.write(text+'\n')

        author.WriteDeclHeaders(writer)
        author.TypeAccessDecl(writer)
        author.MiscDecl(writer)
        author.ForwardDecl(writer, type_decl.structs)
        
        for struct in type_decl.structs:
          author.WriteDeclPOD(writer, struct)

        writer('')
        for struct in type_decl.structs:
          author.SerializerDecl(writer, struct)
        author.MessageSizeTemplate(writer, type_decl.structs)
      
      with open('../src/network_types.cpp', 'w') as f:
        writer = lambda text: f.write(text+'\n')
        
        author.WriteImplHeaders(writer, 'network_types.h')
        
        author.TypeAccessImpl(writer)
        for struct in type_decl.structs:
          author.ChecksumMemberOffsetImpl(writer, struct)
        author.ChecksumImpl(writer, type_decl.structs)
        for struct in type_decl.structs:
          author.WriteImplPOD(writer, struct)
      
