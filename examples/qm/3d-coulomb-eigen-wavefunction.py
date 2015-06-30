#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 3
size1d   = 5
halfSize  = [size1d,size1d*1.2,size1d*1.3]           # FIXME: halfSize  = [size1d,size1d*1.5]

# potential parameters
potentialCenter   = [ 0  ,0  ,0  ]
potentialHalfSize = halfSize
potentialCoefficient= [0.5,0.5,0.5]

coulombOrder_x   = 0
coulombOrder_y   = 1
coulombOrder_z   = 0


O.engines=[
	StateDispatcher([
		St1_QMPacketHydrogenEigenFunc(),
		St1_QMStPotentialCoulomb(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb()],
		[Law2_QMIGeom_QMIPhysCoulomb()]
	),
	SchrodingerAnalyticPropagator(),
	SchrodingerKosloffPropagator(),
]

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

displayOptions   = { 'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default frame","hidden","frame","stripes","mesh"]
		    ,'partsSquared':1
		    ,'threshold3D':0.01}

displayOptionsPot= { 'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default frame","hidden","frame","stripes","mesh"]
		    ,'threshold3D':10}

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**displayOptions)])
analyticBody.material  = QMParameters(dim=dimensions,hbar=1)
coulombPacketArg      = {'energyLevel':[coulombOrder_x, coulombOrder_y, coulombOrder_z],'gridSize':[16,16,16]}
analyticBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic()      # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions)])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],displayOptions=[QMDisplayOptions(**displayOptionsPot)])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
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
	qt.controller.setWindowTitle("3D eigenwavefunction in Coulomb potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

