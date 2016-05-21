# This script copies headers and the dynamic library to the game directory

import os
import shutil

# Simulations library and include directory
lib_dir = '../'
inc_dir = '../include'

lib_ext = ['.dylib', '.so', '.dll']
header_ext = ['.h', '.hpp', '.inl']

# Game directory where everything will be copied to
game_dir = '../../Game'

if __name__ == '__main__':
  header_target = os.path.join(game_dir, 'include', 'simulation')
  lib_target = os.path.join(game_dir, 'lib')

  for lib in [each for each in os.listdir(lib_dir) if each.endswith(tuple(lib_ext))]:
    lib_src = os.path.join(lib_dir,lib)
    dest = os.path.join(game_dir, 'lib')
    print("Copying: " + lib_src + " to: " + dest)
    shutil.copy(lib_src, dest)
    
  for header in [each for each in os.listdir(inc_dir) if each.endswith(tuple(header_ext))]:
    header_src = os.path.join(inc_dir, header)
    dest = os.path.join(game_dir, 'include', 'simulation')
    if not os.path.exists(dest):
      os.makedirs(dest)
    print("Copying: " + header_src + " to: " + dest)
    shutil.copy(header_src, dest)

