#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size1d   = 5
halfSize1 = [size1d,size1d,0.1]
halfSize2 = halfSize1
GRIDSIZE  = [16,16]

# wavepacket parameters
k0_x         = 0
k0_y         = 0
gaussWidth_x = 1.5
gaussWidth_y = 1.5
potentialCoefficient1= [-2.0,0,0]
potentialCoefficient2= [ 2.0,0,0]


O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles()],
		[Law2_QMIGeom_QMIPhysCoulombParticles()]
	),
	SchrodingerKosloffPropagator(),
]

displayOptions1        = { 'partsScale':70,'partsSquared':1
                          ,'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5}
displayOptions2        = { 'partsScale':70,'partsSquared':1
                          ,'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5}

body0           = QMBody()
body0.shape     = QMGeometry(extents=halfSize1,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions1)])
body0.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient1)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
body0.state     = QMPacketGaussianWave(x0=[-1,0,0],t0=0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setNumeric()

body1           = QMBody()
body1.shape     = QMGeometry(extents=halfSize2,color=[0.6,0.6,0.0],displayOptions=[QMDisplayOptions(**displayOptions2)])
body1.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient2)
body1.state     = QMPacketGaussianWave(x0=[1,0,0],t0=0,k0=[-k0_x,-k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body1)
O.bodies[nid].state.setNumeric()

O.dt=.1

O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Two Gaussian packets connected via Coulomb potential in 2D")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

