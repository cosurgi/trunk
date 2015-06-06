#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 3
size1d   = 10
halfSize  = [size1d,size1d*1.5,size1d]

## This is a simple test:
## - a freely moving particle according to Schrodinger equation is calculated using Tal-Ezer Kosloff 1984 method
## - it is compared with the same movement, but calculated analytically
## The error between numerical and analytical solution is plot on the graph

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	SchrodingerKosloffPropagator(),
	SchrodingerAnalyticPropagator()
]


stepRenderFrame   =["default frame","hidden","frame","stripes","mesh"]
displayOptions    = { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                     ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                     ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                     ,'renderMaxTime':0.5
                     ,'threshold3D':0.00001}
## Two particles are created - the analytical one, and the numerical one. They
## do not interact, they are two separate calculations in fact.

## The analytical one:
analyticBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(stepRender=stepRenderFrame,**displayOptions)])
# it's too simple now. Later we will have quarks (up, down, etc.), leptons and bosons as a material.
# So no material for now.
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
#gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[2.5,0,0],'a0':[0.5,0.5,0.5],'gridSize':[128]*dimensions}
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[0.4,2,0],'a0':[1.5,2,1.5],'gridSize':[24,42,24]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic() # is propagated as analytical solution - no calculations involved

## The numerical one:
numericalBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
numericalBody.groupMask = 1
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(stepRender=stepRenderFrame,**displayOptions)])
numericalBody.material  = analyticBody.material
# Initialize the discrete wavefunction using the analytical gaussPacket created earlier.
# The wavefunction shape can be anything - as long as it is normalized, in this case the Gauss shape is used.
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()    # is being propagated by SchrodingerKosloffPropagator

## Define timestep for the calculations
O.dt=.07

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
#O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("3D free propagating packet")
	#qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

