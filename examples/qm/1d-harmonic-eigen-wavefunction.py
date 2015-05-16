#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size_1d   = 100
halfSize  = [size_1d,0.1,0.1]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x       = 2
gaussWidth = 1

# potential parameters
potentialCenter   = [ 0.0,0  ,0  ]
potentialHalfSize = Vector3(size_1d,3,3)
harmonicOrder     = 15


O.engines=[
	StateDispatcher([
		St1_QMPacketHarmonicEigenFunc(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
# in DEM was: Ig2_Box_Sphere_ScGeom  → Constructs QMPotGeometry for Box+QMGeometry
		[Ig2_Box_QMGeometry_QMPotGeometry()],
# in DEM was: Ip2_FrictMat_FrictMat_FrictPhys()     → SKIP: no material parameters so far
		[Ip2_QMParameters_QMParameters_QMPotPhysics()],
# in DEM was: Law2_ScGeom_FrictPhys_CundallStrack() → SKIP: potential is handles inside SchrodingerKosloffPropagator
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
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],step=[0.03,0.1,0.1])
analyticBody.material  = QMParameters(dim=dimensions,hbar=1)
harmonicPacketArg      = {'energyLevel':[harmonicOrder,0,0],'size':size,'gridSize':[2**13]}
analyticBody.state     = QMPacketHarmonicEigenFunc(**harmonicPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.blockedDOFs='xyzXYZ' # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketHarmonicEigenFunc(**harmonicPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.blockedDOFs=''      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = Box(extents=potentialHalfSize ,wire=True)
potentialBody.material  = QMParameters()
potentialBody.state     = QMStateBarrier(se3=[potentialCenter,Quaternion((1,0,0),0)],potentialType=1)
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.002

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.View()
	qt.Controller()
	qt.controller.setWindowTitle("1D eigenwavefunction in harmonic potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
except ImportError:
	pass

#O.run(20000)

