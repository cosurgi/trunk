#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size_1d   = 10
halfSize  = [size_1d,size_1d*1.5,0.1]
size      = [x * 2 for x in halfSize]

# potential parameters
potentialCenter   = [ 0  ,0  ,0  ]
potentialHalfSize = halfSize
harmonicOrder_x   = 0
harmonicOrder_y   = 1


O.engines=[
	SpatialQuickSortCollider([
	#InsertionSortCollider([
	#	Bo1_QMGeometry_Aabb(),
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
# in DEM was: Ig2_Box_Sphere_ScGeom  → Constructs QMPotGeometry for Box+QMGeometry
		[Ig2_Box_QMGeometry_QMPotGeometry()],
# in DEM was: Ip2_FrictMat_FrictMat_FrictPhys()     → SKIP: no material parameters so far
		[Ip2_QMParameters_QMParameters_QMPotPhysics()],
# in DEM was: Law2_ScGeom_FrictPhys_CundallStrack() → SKIP: potential is handled inside SchrodingerKosloffPropagator
		[Law2_QMPotGeometry_QMPotPhysics_QMPotPhysics()]
	),
	SchrodingerAnalyticPropagator(),
	SchrodingerKosloffPropagator(steps=-1 ), # auto
]

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],partsScale=10
                                         , partAbsolute=['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , partReal=['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , renderMaxTime=0.5)
analyticBody.material  = QMParameters()
harmonicPacket         = QMPacketHarmonicEigenFunc(dim=dimensions,energyLevel=[harmonicOrder_x, harmonicOrder_y, 0])
analyticBody.state     = harmonicPacket
O.bodies.append(analyticBody)     # do not append, it is used only to create the numerical one

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],partsScale=10
                                         , partAbsolute=['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , partReal=['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                         , renderMaxTime=0.5)
numericalBody.material  = QMParameters()
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMStateDiscrete(creator=harmonicPacket,dim=dimensions,size=size,gridSize=[2**6,2**7])
O.bodies.append(numericalBody)

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = Box(extents=potentialHalfSize ,wire=True)
potentialBody.material  = QMParameters()
potentialBody.state     = QMStateBarrier(se3=[potentialCenter,Quaternion((1,0,0),0)],potentialType=1)
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.02

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("2D eigenwavefunction in harmonic potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

