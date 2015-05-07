#!/usr/bin/python
# -*- coding: utf-8 -*-

# PICK NUMBER OF DIMENSIONS (1,2 or 3):
dimensions= 2
size_1d   = 10
halfSize  = [size_1d,size_1d*1.2,0.1]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x         = 3
k0_y         = 2
gaussWidth_x = 1.0
gaussWidth_y = 2.0

# potential parameters
potentialCenter   = [ 0, 0 ,0  ]
potentialHalfSize = halfSize
potentialValue    = 0.0

O.engines=[
	SpatialQuickSortCollider([
	#InsertionSortCollider([
		Bo1_QMGeometry_Aabb(),
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_Box_QMGeometry_QMPotGeometry()],
		[Ip2_QMParameters_QMParameters_QMInteractionPhysics()],
		[Law2_QMPotGeometry_QMInteractionPhysics_QMInteractionPhysics()] 
	),
	SchrodingerKosloffPropagator(),
]


## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(halfSize=halfSize,color=[0.6,0.6,0.6],partsScale=10)
analyticBody.material  = QMParameters()
gaussPacket            = QMPacketGaussianWave(dim=dimensions,x0=[0,2,0],t0=0,k0=[k0_x,k0_y,0],m=1,a0=[gaussWidth_x,gaussWidth_y,0],hbar=1)
analyticBody.state     = gaussPacket
#O.bodies.append(analyticBody)     # do not append, it is used only to create the numerical one

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(halfSize=halfSize,color=[1,1,1],partsScale=10)
numericalBody.material  = QMParameters()
#numericalBody.state     = QMStateDiscrete(creator=gaussPacket,dim=dimensions,size=size,gridSize=[(2**7),(2**6)])
numericalBody.state     = QMStateDiscrete(creator=gaussPacket,dim=dimensions,size=size,gridSize=[2**6,2**7])
O.bodies.append(numericalBody)

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = Box(extents=potentialHalfSize ,wire=True)
potentialBody.material  = QMParameters()
potentialBody.state     = QMStateBarrier(se3=[potentialCenter,Quaternion((1,0,0),0)],potentialValue=potentialValue,potentialType=1)
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.1

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Gaussian packet in 2D harmonic potential")
	qt.Renderer().blinkHighlight=False
	Gl1_QMGeometry().step=[0.2,0.2,0.2]
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

