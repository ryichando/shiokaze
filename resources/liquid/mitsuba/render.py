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
SampleCount = sys.argv[2]
target_x = float(sys.argv[3])
target_y = float(sys.argv[4])
target_z = float(sys.argv[5])
origin_x = float(sys.argv[6])
origin_y = float(sys.argv[7])
origin_z = float(sys.argv[8])
xml_name = sys.argv[9]

img_path = '../'+xml_name+'_img'
if not os.path.exists(img_path):
	os.system('mkdir '+img_path)

for frame in range(1,end+1):
	#
	png_path = img_path+'/'+str(frame)+'_'+xml_name+'.png'
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
		if xml_name == 'transparent':
			mesh_file = '../mesh/'+str(frame)+'_mesh_enclosed.ply'
		#
		while( not os.path.exists(mesh_file)):
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
		#
		paramMap['mesh_filename'] = mesh_file
		paramMap['solid_filename'] = '../mesh/static_solids/levelset_solid.ply'
		scene = SceneHandler.loadScene(fileResolver.resolve(xml_name+'.xml'),paramMap)
		#
		# Create a queue for tracking render jobs
		queue = RenderQueue()
		exr_path = img_path+'/'+str(frame)+'_'+xml_name+'.exr'
		scene.setDestinationFile(exr_path)
		#
		# Create a render job and insert it into the queue
		job = RenderJob('myRenderJob', scene, queue)
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
		os.system('mtsutil tonemap -g 3 '+exr_path)
		#
		# Sometimes compile movie
		if frame % interval == interval-1:
			video_path = img_path+'/'+xml_name+'.mp4'
			os.system('rm -rf '+video_path)
			os.system('avconv -r 60 -i '+img_path+'/%d_'+xml_name+'.png -b:v 120000k -c:v libx264 -pix_fmt yuv420p -loglevel panic -filter:v lutyuv="y=gammaval(1.3)" '+video_path)

