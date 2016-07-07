import os

AddOption('--sfmlDir',
  default='#contrib',
  help='Extracted local directory for SFML')
AddOption('--projects',
  action='store_true',
  help='Create visual studio project file')
AddOption('--shared_lib',
  action='store_true',
  help='Create a shared library')
AddOption('--pdb',
  action='store_true',
  help='Create a debugging symbols')
AddOption('--variant',
  choices=["debug","release"],
  default='debug',
  action='store',
  help='debug/release variant')
AddOption('--arch',
  choices=['x86', 'x86_64'],
  default='x86_64',
  help='output architecture for binaries')
AddOption('--pybinddir',
  default=Dir('#../py4xsim/4x').abspath,
  help='Install directory for python bindings')

# Create required directories
dirs = ['build', 'projects']
for dir in dirs:
  try:
   os.makedirs(dir)
  except OSError:
    pass

#Build Environment
env = Environment(TARGET_ARCH=GetOption('arch'))
print(env['MSVC_VERSION'])

#Enable windows specific CXXFLAGS
if env['PLATFORM'] == 'win32':
  env['CXXCOM'] = env['CXXCOM'].replace("$CXXFLAGS", "$WINFLAGS $CXXFLAGS")
  env['SHCXXCOM'] = env['SHCXXCOM'].replace("$SHCXXFLAGS", "$WINFLAGS $SHCXXFLAGS")
#windows specific compiler flags
env.Append(WINFLAGS=['/EHsc'])
env['PDB'] = GetOption('pdb')

if (GetOption('variant') == 'debug'):
  env.Append(WINFLAGS=['/MDd'])
else:
  env.Append(WINFLAGS=['/MD', '/O1'])
  
#Paths
env['LIBPATH'] = [os.path.join(GetOption('sfmlDir'), 'lib')]
env.Append(CPPPATH=[Dir('#include').abspath])

outdir='build'

allFiles = SConscript('src/SConscript', exports='env', variant_dir=outdir, duplicate=0)

Alias('install', env.AlwaysBuild(env.Install(GetOption('pybinddir'), Dir('#include').abspath)))

#project files
if GetOption('projects'):
  allIncludes = []
  for cppPath in env['CPPPATH']:
    for root, dirs, files in os.walk(cppPath):
      filesWithPath = map(lambda f: os.path.join(root, f), files)
      allIncludes.extend(filesWithPath)
      
  relativeSrcPaths = map(lambda f: f.srcnode().path, allFiles)
  relativeToProjectDir = map(lambda f: os.path.relpath(f, 'projects'), relativeSrcPaths)
  env.MSVSProject(target = 'projects/Simulation' + env['MSVSPROJECTSUFFIX'],
    srcs=relativeToProjectDir, 
    incs=allIncludes, 
    variant='default')