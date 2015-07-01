#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size1d   = 50
halfSize  = [size1d,0.1,0.1]           # FIXME: halfSize  = [size1d]
GRIDSIZE = 2**11

# wavepacket parameters
k0_x       = 0.7
gaussWidth = 0.5

# potential parameters
potentialCenter   = [ 0 ,0  ,0  ]
potentialHalfSize = Vector3(size1d,3,3)
potentialCoefficient= [-20,0,0]
potentialMaximum    = 20;

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
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
	SchrodingerKosloffPropagator(steps=-1),
]

stepRenderStripes=["default stripes","hidden","frame","stripes","mesh"]
stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
displayOptions   = { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default nodes', 'hidden', 'nodes', 'bars', 'points', 'wire', 'surface']}
displayOptionsFFT= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default bars', 'hidden', 'nodes', 'bars', 'points', 'wire', 'surface']
                    ,'partReal':['default nodes', 'hidden', 'nodes', 'bars', 'points', 'wire', 'surface']}


## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[-3,0,0],'t0':0,'k0':[k0_x,0,0],'a0':[gaussWidth,0,0],'gridSize':[GRIDSIZE]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.setAnalytic() # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(renderWireLight=False,renderSe3=(Vector3(0,0,0), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[
     QMDisplayOptions(stepRender=stepRenderHide,partsScale=1,**displayOptions)
#    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)),**displayOptionsFFT)
])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient,potentialMaximum=potentialMaximum)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
#O.dt=.001
#O.dt=.003
##O.dt=.005 # FIXME - noise explosion, why? (because steps=50)
O.dt=.01 # works with steps=100
#O.dt=.01 # works a few cycles with steps=150
#O.dt=.012 #is not working, even for steps=150
#O.dt=.02 # is not working, even for steps=300
#O.dt=.1

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("1D gaussian packet in Coulomb potential")
	qt.View()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

