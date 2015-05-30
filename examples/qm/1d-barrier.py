#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size_1d   = 15
halfSize  = [size_1d,0.1,0.1]          # FIXME: halfSize  = [size_1d]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x       = 10
gaussWidth = 0.1

# potential parameters
potentialCenter1  = [ 2.0,0  ,0  ]
potentialCenter2  = [-2.0,0  ,0  ]
potentialHalfSize = [1.0,0.1,0.1]
potentialValue    = 23000/2

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

stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
stepRenderStripes=["default stripes","hidden","frame","stripes","mesh"]
## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(stepRender=stepRenderHide)])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[k0_x,0,0],'a0':[gaussWidth,0,0],'size':size,'gridSize':[2**11]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.blockedDOFs='xyzXYZ' # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(stepRender=stepRenderHide)])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.blockedDOFs=''      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody1 = QMBody()
potentialBody1.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[QMDisplayOptions(partsScale=-potentialValue,stepRender=stepRenderStripes)])
potentialBody1.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody1.state     = QMStPotentialBarrier(se3=[potentialCenter1,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody1)

potentialBody2 = QMBody()
potentialBody2.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[QMDisplayOptions(partsScale=-potentialValue,stepRender=stepRenderStripes)])
potentialBody2.material  = QMParametersBarrier(dim=dimensions,hbar=1,height=potentialValue)
potentialBody2.state     = QMStPotentialBarrier(se3=[potentialCenter2,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody2)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.001

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.Renderer().blinkHighlight=False
	Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False
	qt.View()
	qt.controller.setWindowTitle("Packet inside a 1D well")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

