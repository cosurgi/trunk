#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size_1d   = 15
halfSize  = [size_1d,0.1,0.1]
size      = [x * 2 for x in halfSize]


## FIXME a1 = [0,1,2]
## FIXME a2 = [x * 3 for x in a1]
## FIXME or, if you need a1 to be done in place:
## FIXME a1[:] = [x*3 for x in a1] 
## FIXME import numpy
## FIXME a = numpy.array([0, 1, 2])
## FIXME print a * 3


# wavepacket parameters
k0_x       = 10
gaussWidth = 0.1

# potential parameters
potentialCenter1  = [ 2.0,0  ,0  ]
potentialCenter2  = [-2.0,0  ,0  ]
potentialHalfSize = [1.0,0.1,0.1]
potentialValue    = 1000.0


O.engines=[
	SpatialQuickSortCollider([
	#InsertionSortCollider([
		Bo1_QMGeometryDisplay_Aabb(),
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
# in DEM was: Ig2_Box_Sphere_ScGeom  → Constructs QMInteractionGeometry for Box+QMGeometryDisplay
		[Ig2_Box_QMGeometryDisplay_QMInteractionGeometry()],
# in DEM was: Ip2_FrictMat_FrictMat_FrictPhys()     → SKIP: no material parameters so far
		[Ip2_QMParameters_QMParameters_QMInteractionPhysics()],
# in DEM was: Law2_ScGeom_FrictPhys_CundallStrack() → SKIP: potential is handles inside SchrodingerKosloffPropagator
		[Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics()] 
	),
	SchrodingerKosloffPropagator(),
]

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometryDisplay(halfSize=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParameters()
gaussPacket            = FreeMovingGaussianWavePacket(dim=dimensions,x0=[0,0,0],t0=0,k0=[k0_x,0,0],m=1,a=gaussWidth,hbar=1)
analyticBody.state     = gaussPacket
#O.bodies.append(analyticBody)     # do not append, it is used only to create the numerical one

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometryDisplay(halfSize=halfSize,color=[1,1,1])
numericalBody.material  = QMParameters()
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMStateDiscrete(creator=gaussPacket,dim=dimensions,size=size,gridSize=[(2**11)])
O.bodies.append(numericalBody)

## 3: The box with potential
potentialBody1 = QMBody()
potentialBody1.shape     = Box(extents=potentialHalfSize ,wire=True)
potentialBody1.material  = QMParameters()
potentialBody1.state     = QMStateBarrier(se3=[potentialCenter1,Quaternion((1,0,0),0)],potentialValue=potentialValue)
O.bodies.append(potentialBody1)

potentialBody2 = QMBody()
potentialBody2.shape     = Box(extents=potentialHalfSize ,wire=True)
potentialBody2.material  = QMParameters()
potentialBody2.state     = QMStateBarrier(se3=[potentialCenter2,Quaternion((1,0,0),0)],potentialValue=potentialValue)
O.bodies.append(potentialBody2)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.001

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.View()
	qt.Controller()
	qt.controller.setWindowTitle("Packet inside a 1D well")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
except ImportError:
	pass

#O.run(20000)

