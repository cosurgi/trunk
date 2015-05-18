#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size_1d   = 10
halfSize1 = [size_1d,size_1d,0.1]
halfSize2 = halfSize1
size1     = [x * 2 for x in halfSize1]
size2     = [x * 2 for x in halfSize2]

# wavepacket parameters
k0_x         = 3
k0_y         = 2
gaussWidth_x = 1.0
gaussWidth_y = 2.0
potentialCoefficient= [0.5,0.5,0.5]


O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles()],
		[Law2_QMIGeom_QMIPhysHarmonicParticles()]
	),
	SchrodingerKosloffPropagator(),
]

body0           = QMBody()
body0.shape     = QMGeometry(extents=halfSize1,color=[1,1,1],partsScale=10)
body0.material  = QMParticleHarmonic(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient)
body0.state     = QMPacketGaussianWave(x0=[0,2,0],t0=0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],size=size1,gridSize=[2**6,2**6]) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.blockedDOFs=''

body1           = QMBody()
body1.shape     = QMGeometry(extents=halfSize2,color=[0.6,0.8,0.6],partsScale=10)
body1.material  = QMParticleHarmonic(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient)
body1.state     = QMPacketGaussianWave(x0=[2,0,0],t0=0,k0=[-k0_x,-k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],size=size2,gridSize=[2**6,2**6]) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body1)
O.bodies[nid].state.blockedDOFs=''

O.dt=.1

O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Two Gaussian packets connected via harmonic potential in 2D")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

