import os

AddOption('--sfmlDir',
  default='#contrib',
  help='Extracted local directory for SFML')
AddOption('--projects',
  action='store_true',
  help='Create visual studio project file')
AddOption('--lib',
  action='store_true',
  help='Create a library instead of executable')
  
# Create required directories
dirs = ['build', 'projects']
for dir in dirs:
  try:
   os.makedirs(dir)
  except OSError:
    pass

#Build Environment
env = Environment(TARGET_ARCH='i386')

#Enable windows specific CXXFLAGS
if env['PLATFORM'] == 'win32':
  env['CXXCOM'] = env['CXXCOM'].replace("$CXXFLAGS", "$CXXWINFLAGS $CXXFLAGS")
  
#use CXXWINFLAGS
env.Append(CXXWINFLAGS=['/EHsc'])

#Paths
env['LIBPATH'] = [os.path.join(GetOption('sfmlDir'), 'lib')]
env.Append(CPPPATH=[Dir('#include').abspath])

outdir='build'

allFiles = SConscript('src/SConscript', exports='env', variant_dir=outdir, duplicate=0)

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