#! [section]
-------------------  MAC Liquid 2D (Liquid)  -------------------
   Advection = "macadvection2"
   Array = "lineararray_core2"
   Gravity = 0,-9.8
   GridUtility = "gridutility2"
   GridVisualizer = "gridvisualizer2"
   MacStats = "macstats2"
   MacUtility = "macutility2"
   MacVisualizer = "macvisualizer2"
   Name = "waterdrop2"
   Projection = "macpressuresolver2"
   ResolutionScale = 1.0
   ResolutionX = 64
   ResolutionY = 32
   SurfaceTracker = "maclevelsetsurfacetracker2"
   TimeStepper = "timestepper"
   VolumeChangeTolRatio = 0.03
   VolumeCorrection = Yes
-------------  MAC Visualizer 2D (MacVisualizer)  --------------
#! [section]

#! [description]
-------------------  MAC Liquid 2D (Liquid)  -------------------  
   Advection (STRING): Advection module
   Array (STRING): Array core module
   Gravity (VEC2D): Gravity vector
   GridUtility (STRING): Grid utility module
   GridVisualizer (STRING): Grid visualizer module
   MacStats (STRING): MAC Statistics Analyzer
   MacUtility (STRING): MAC Utility Tools
   MacVisualizer (STRING): MAC visualizer module
   Name (STRING): Scene file name
   Projection (STRING): Projection module
   ResolutionScale (DOUBLE): Resolution doubling scale
   ResolutionX (UNSIGNED): Resolution towards X axis
   ResolutionY (UNSIGNED): Resolution towards Y axis
   SurfaceTracker (STRING): Moving level set tracking module
   TimeStepper (STRING): Time stepper module
   VolumeChangeTolRatio (DOUBLE): Volume change tolerance ratio
   VolumeCorrection (BOOL): Should perform volume correction
-------------  MAC Visualizer 2D (MacVisualizer)  --------------  
#! [description]