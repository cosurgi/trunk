#!/usr/bin/python
# -*- coding: utf-8 -*-

# PICK NUMBER OF DIMENSIONS (1,2 or 3):
dimensions= 2
size_1d   = 12
halfSize  = [size_1d,size_1d,0.1]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x       = 2
k0_y       = 4
gaussWidth_x = 2.0
gaussWidth_y = 1.5

# potential parameters
potentialCenter1  = [ 9,0  ,0  ]
potentialCenter2  = [-9,0  ,0  ]
potentialHalfSize = [1.0,10.0,1.0]

potentialCenter1a = [0  , 9,0  ]
potentialCenter2a = [0  ,-9,0  ]
potentialHalfSizeA= [ 7.8,1.0,1.0]
potentialValue    = 100.0

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
analyticBody.shape     = QMGeometry(halfSize=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParameters()
gaussPacket            = QMPacketGaussianWave(dim=dimensions,x0=[0,0,0],t0=0,k0=[k0_x,k0_y,0],m=1,a0=[gaussWidth_x,gaussWidth_y,0],hbar=1)
analyticBody.state     = gaussPacket
#O.bodies.append(analyticBody)     # do not append, it is used only to create the numerical one

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(halfSize=halfSize,color=[1,1,1],partsScale=10)
					# partAbsolute=['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
					# partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
					# partReal=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
numericalBody.material  = QMParameters()
numericalBody.state     = QMStateDiscrete(creator=gaussPacket,dim=dimensions,size=size,gridSize=[(2**7)]*dimensions)
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

potentialBody3 = QMBody()
potentialBody3.shape     = Box(extents=potentialHalfSizeA ,wire=True)
potentialBody3.material  = QMParameters()
potentialBody3.state     = QMStateBarrier(se3=[potentialCenter1a,Quaternion((1,0,0),0)],potentialValue=potentialValue)
O.bodies.append(potentialBody3)

potentialBody4 = QMBody()
potentialBody4.shape     = Box(extents=potentialHalfSizeA ,wire=True)
potentialBody4.material  = QMParameters()
potentialBody4.state     = QMStateBarrier(se3=[potentialCenter2a,Quaternion((1,0,0),0)],potentialValue=potentialValue)
O.bodies.append(potentialBody4)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.1

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("2D potential well")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

