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
VolumeScale = sys.argv[3]
img_dirname = sys.argv[4]

img_path = '../'+img_dirname
if not os.path.exists(img_path):
	os.system('mkdir '+img_path)

for frame in range(1,end+1):
	#
	png_path = img_path+'/'+str(frame)+'_density.png'
	if( not os.path.exists(png_path)):
		#
		# Get a reference to the thread's file resolver
		fileResolver = Thread.getThread().getFileResolver()
		#
		# Interval for compling movie
		interval = 10
		#
		# Path to density files
		density_file = '../density/'+str(frame)+'_density.vol'
		#
		while( not os.path.exists(density_file)):
			time.sleep(1)
		#
		print 'Starting frame', frame
		#
		# Scene parameters
		paramMap = StringMap()
		paramMap['sample_count'] = SampleCount
		paramMap['volume_scale'] = VolumeScale
		paramMap['density_filename'] = density_file
		#
		scene = SceneHandler.loadScene(fileResolver.resolve("smoke.xml"),paramMap)
		#
		# Create a queue for tracking render jobs
		queue = RenderQueue()
		scene.setDestinationFile(img_path+'/'+str(frame)+'_density.exr')
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
		exr_path = img_path+'/'+str(frame)+'_density.exr'
		os.system('mtsutil tonemap -g 3 '+exr_path)
		#
		# Sometimes compile movie
		if frame % interval == interval-1:
			video_path = img_path+'/density.mp4'
			os.system('rm -rf '+video_path)
			os.system('avconv -r 60 -i '+img_path+'/%d_density.png -b:v 120000k -c:v libx264 -pix_fmt yuv420p -loglevel panic -filter:v lutyuv="y=gammaval(1.3)" '+video_path)

