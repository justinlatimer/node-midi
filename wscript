import sys

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  if sys.platform == 'darwin':
    conf.env.append_value('LINKFLAGS', ['-framework','CoreMidi','-framework','CoreAudio','-framework','CoreFoundation'])
  else:
    conf.env.append_value('LINKFLAGS', ['-lasound', '-lpthread'])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  if sys.platform == 'darwin':
    obj.cxxflags.append("-D__MACOSX_CORE__")
  else:
    obj.cxxflags.append("-D__LINUX_ALSASEQ__")
  obj.target = "midi"
  obj.source = "src/node-midi.cpp"
