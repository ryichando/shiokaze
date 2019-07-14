#!/usr/bin/env python
#
#	resources/flip/wscript
#
#	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
#	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 13, 2017.
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
import mitsuba
from mitsuba.core import *
from mitsuba.render import SceneHandler
from mitsuba.render import RenderQueue, RenderJob
import multiprocessing
from struct import *
import sys
import os
import time
import math

# Start up the scheduling system with one worker per local core
scheduler = Scheduler.getInstance()
for i in range(0, multiprocessing.cpu_count()):
	scheduler.registerWorker(LocalWorker(i, 'wrk%i' % i))
scheduler.start()

end = int(sys.argv[1])
name = sys.argv[2]
LiquidColor_Red = float(sys.argv[3])
LiquidColor_Green = float(sys.argv[4])
LiquidColor_Blue = float(sys.argv[5])
SampleCount = sys.argv[6]
target_x = float(sys.argv[7])
target_y = float(sys.argv[8])
target_z = float(sys.argv[9])
origin_x = float(sys.argv[10])
origin_y = float(sys.argv[11])
origin_z = float(sys.argv[12])

img_path = '../'+name+'_img'
if not os.path.exists(img_path):
	os.system('mkdir '+img_path)

for frame in range(1,end+1):
	#
	png_path = img_path+'/'+str(frame)+'_'+name+'.png'
	if( not os.path.exists(png_path)):
		#
		# Get a reference to the thread's file resolver
		fileResolver = Thread.getThread().getFileResolver()
		#
		# Interval for compling movie
		interval = 10
		#
		# Path to mesh files
		mesh_file = '../mesh/'+str(frame)+'_mesh.ply'
		if name == 'transparent':
			mesh_file = '../mesh/'+str(frame)+'_mesh_enclosed.serialized'

		particle_file = '../mesh/'+str(frame)+'_particles.dat'
		#
		while( not os.path.exists(mesh_file)):
			time.sleep(1)
		#
		while( not os.path.exists(particle_file)):
			time.sleep(1)
		#
		print 'Starting frame', frame
		#
		# Scene parameters
		paramMap = StringMap()
		paramMap['target_x'] = str(target_x)
		paramMap['target_y'] = str(target_y)
		paramMap['target_z'] = str(target_z)
		paramMap['origin_x'] = str(origin_x)
		paramMap['origin_y'] = str(origin_y)
		paramMap['origin_z'] = str(origin_z)
		paramMap['up'] = '0, 1, 0'
		paramMap['sample_count'] = SampleCount
		paramMap['liquid_color'] = str(LiquidColor_Red)+', '+str(LiquidColor_Green)+', '+str(LiquidColor_Blue)
		#
		paramMap['mesh_filename'] = mesh_file
		paramMap['solid_filename'] = '../mesh/static_solids/levelset_solid.ply'
		scene = SceneHandler.loadScene(fileResolver.resolve(name+'.xml'),paramMap)
		#
		# Append ballistic particles if exists
		if os.path.exists(particle_file):
			print 'Loading particles...'
			pmgr = PluginManager.getInstance()
			f = open(particle_file,'rb')
			(number,) = unpack('I',f.read(4))
			print 'Adding '+str(number)+' particles...'
			for i in range(0,number):
				(x,y,z,r,) = unpack('ffff',f.read(16))
				ParticleColor = Spectrum()
				ParticleColor.fromSRGB(LiquidColor_Red,LiquidColor_Green,LiquidColor_Blue)
				if name == 'mesh':
					scene.addChild(pmgr.create(
									   {'type' : 'sphere',
									   'center' : Point(x,y,z),
									   'radius' : r,
									   'ref' : {
									   'type' : 'plastic',
									   'diffuseReflectance' : ParticleColor,
									   'intIOR' : 1.5
									   }
									   }))
				elif name == 'transparent':
					scene.addChild(pmgr.create(
									   {'type' : 'sphere',
									   'center' : Point(x,y,z),
									   'radius' : r,
									   'ref' : {
									   'type' : 'dielectric',
									   'intIOR' : 1.5
									   }
									   }))
		#
		# Create a queue for tracking render jobs
		queue = RenderQueue()
		scene.setDestinationFile(img_path+'/'+str(frame)+'_'+name+'.exr')
		#
		# Create a render job and insert it into the queue
		job = RenderJob('myRenderJob',scene,queue)
		job.start()
		#
		# Wait for all jobs to finish and release resources
		queue.waitLeft(0)
		queue.join()
		#
		# Print some statistics about the rendering process
		print(Statistics.getInstance().getStats())
		#
		# Tonemapping
		exr_path = img_path+'/'+str(frame)+'_'+name+'.exr'
		os.system('mtsutil tonemap -g 3 '+exr_path)
		#
		# Sometimes compile movie
		if frame % interval == interval-1:
			video_path = img_path+'/'+name+'.mp4'
			os.system('rm -rf '+video_path)
			os.system('avconv -r 60 -i '+img_path+'/%d_'+name+'.png -b:v 120000k -c:v libx264 -pix_fmt yuv420p -loglevel panic -filter:v lutyuv="y=gammaval(1.3)" '+video_path)