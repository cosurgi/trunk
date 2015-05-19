#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size_1d   = 12
halfSize  = [size_1d,size_1d,0.1]           # FIXME: halfSize  = [size_1d,size_1d]  ← że nie ma trzeciej składowej, ale Box::extents robi problemy
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
potentialHalfSizeA= [ 7.99,1.0,1.0]        # FIXME: nie może być [8,1,1] bo coś nie działa, tylko co???
potentialValue    = 100

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialBarrier(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier()],
		[Law2_QMIGeom_QMIPhysBarrier()]
	),
	SchrodingerKosloffPropagator(),
]

displayOptionsPot= { 'partAbsolute':['default nodes', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]}

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[k0_x,k0_y,0],'a0':[gaussWidth_x,gaussWidth_y,0],'size':size,'gridSize':[2**6,2**7]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.blockedDOFs='xyzXYZ' # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],partsScale=10)
					# partAbsolute=['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
					# partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
					# partReal=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(se3=[[0,0,0],Quaternion((1,0,0),0)],**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.blockedDOFs=''      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody1 = QMBody()
potentialBody1.shape     = QMGeometry(extents=potentialHalfSize,color=[0.3,0.5,0.3],partsScale=-potentialValue,**displayOptionsPot)
potentialBody1.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody1.state     = QMStPotentialBarrier(se3=[potentialCenter1,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody1)

potentialBody2 = QMBody()
potentialBody2.shape     = QMGeometry(extents=potentialHalfSize,color=[0.3,0.5,0.3],partsScale=-potentialValue,**displayOptionsPot)
potentialBody2.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody2.state     = QMStPotentialBarrier(se3=[potentialCenter2,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody2)

potentialBody3 = QMBody()
potentialBody3.shape     = QMGeometry(extents=potentialHalfSizeA,color=[0.3,0.5,0.3],partsScale=-potentialValue,**displayOptionsPot)
potentialBody3.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody3.state     = QMStPotentialBarrier(se3=[potentialCenter1a,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody3)

potentialBody4 = QMBody()
potentialBody4.shape     = QMGeometry(extents=potentialHalfSizeA,color=[0.3,0.5,0.3],partsScale=-potentialValue,**displayOptionsPot)
potentialBody4.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody4.state     = QMStPotentialBarrier(se3=[potentialCenter2a,Quaternion((1,0,0),0)])
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
	qt.Renderer().blinkHighlight=False
	#Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False
	qt.View()
	qt.controller.setWindowTitle("2D potential well")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

