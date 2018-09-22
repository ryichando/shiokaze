#!/usr/bin/env python
#
#	wscript
#
#	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
#	Created by Ryoichi Ando <rand@nii.ac.jp> on 30 Jan, 2017. All rights reserved.
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy of
#	this software and associated documentation files (the "Software"), to deal in
#	the Software without restriction, including without limitation the rights to use,
#	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
#	Software, and to permit persons to whom the Software is furnished to do so,
#	subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in all copies
#	or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
#	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
#	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
#	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
#	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
import sys, os, multiprocessing, platform, glob, filecmp
#
if 'DISPLAY' in os.environ:
	use_opengl = True
else:
	use_opengl = False
#
num_cores = multiprocessing.cpu_count()
rows, columns = os.popen('stty size', 'r').read().split()
greet_total = min(int(columns),60)
#
def options(opt):
	#
	opt.greeting = greeting
	opt.message = message
	#
	opt.greeting('Shiokaze Framework')
	opt.message('A research-oriented fluid solver for computer graphics.')
	opt.message('Designed and developd by Ryoichi Ando <rand@nii.ac.jp>')
	#
	opt.load('compiler_c')
	opt.load('compiler_cxx')
	#
	greet_end()
#
def common_configure(conf,target):
	#
	if target == 'debug':
		conf.setenv('debug')
	#
	conf.env.CXXFLAGS = ['-std=c++14']
	#
	if target == 'debug':
		suffix = '_debug'
		conf.define('SHKZ_BUILD_TARGET','Debug')
		conf.define('SHKZ_DEBUG',1)
		conf.env.CXXFLAGS.append('-g')
		conf.env.CXXFLAGS.append('-O0')
		conf.env.LINKFLAGS.append('-g')
	elif target == 'release':
		suffix = ''
		conf.define('SHKZ_BUILD_TARGET','Release')
		conf.define('SHKZ_RELEASE',1)
		conf.env.CXXFLAGS.append('-O3')
	#
	conf.env.suffix = suffix
	#
	conf.load('compiler_c')
	conf.load('compiler_cxx')
	#
	conf.define('SHKZ_SUFFIX',suffix)
	conf.define('NUM_THREAD',num_cores)
	#
	if use_opengl:
		conf.define('USE_OPENGL',1)

def configure(conf):
	#
	common_configure(conf,'release')
	common_configure(conf,'debug')
#
def get_target_name(bld,name):
	if type(name) is list:
		return ['shiokaze_'+x+bld.env.suffix for x in name]
	else:
		return 'shiokaze_'+name+bld.env.suffix
#
def build(bld):
	#
	bld.get_target_name = get_target_name
	bld.use_opengl = use_opengl
	#
	bld.root_path = bld.path.abspath()
	bld.env.LIBPATH  = ['/usr/local/lib']
	bld.env.INCLUDES = ['/usr/local/include',
						bld.root_path+'/include',
						bld.root_path+'/local/include']
	#
	if use_opengl:
		if sys.platform == 'darwin':
			bld.LIB_OPENGL = ['glfw']
			bld.FRAMEWORK_OPENGL = ['OpenGL','GLUT']
		elif sys.platform == 'linux2':
			bld.LIB_OPENGL = ['glfw','GL','GLU','glut']
			bld.FRAMEWORK_OPENGL = []
	else:
		if sys.platform == 'darwin':
			bld.LIB_OPENGL = []
			bld.FRAMEWORK_OPENGL = []
		elif sys.platform == 'linux2':
			bld.LIB_OPENGL = []
			bld.FRAMEWORK_OPENGL = []
	#
	MODULES = glob.glob(bld.root_path+'/src/*/')
	RESOURCES = []
	LOCAL = []
	#
	if os.path.exists(bld.root_path+'/local/wscript'):
		LOCAL.append(bld.root_path+'/local')
	for dir in glob.glob(bld.root_path+'/resources/*/'):
		if os.path.exists(dir+'wscript'):
			RESOURCES.append(dir)
	#
	bld.recurse(['launcher']+LOCAL+MODULES+RESOURCES)
#
def greeting(str):
	rows, columns = os.popen('stty size', 'r').read().split()
	count = len(str)
	streamer_count = (greet_total-count-2)/2
	for i in range(streamer_count):
		sys.stdout.write('-')
	sys.stdout.write(' ')
	sys.stdout.write(str)
	sys.stdout.write(' ')
	for i in range(greet_total-count-streamer_count-2):
		sys.stdout.write('-')
	sys.stdout.write('\n')
#
def message(str):
	print str
#
def greet_end():
	for i in range(greet_total):
		sys.stdout.write('-')
	sys.stdout.write('\n')
#
from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
class debug(BuildContext):
	cmd = 'debug'
	variant = 'debug'
for x in 'debug release'.split():
	for y in (BuildContext, CleanContext, InstallContext, UninstallContext):
		name = y.__name__.replace('Context','').lower()
		class tmp(y):
			cmd = name + '_' + x
			variant = x
#
