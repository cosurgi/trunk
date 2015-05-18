#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 3
size_1d   = 5 
halfSize  = [size_1d,size_1d*1.2,size_1d]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x         = 1
k0_y         = 0
k0_z         = 0
gaussWidth_x = 0.7
gaussWidth_y = 0.7
gaussWidth_z = 0.7
potentialCoefficient= [2,2,2]

# potential parameters
potentialCenter   = [ 0, 0 ,0  ]
potentialHalfSize = halfSize # FIXME: size ??
potentialValue    = 0.0

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStateHarmonic(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic()],
		[Law2_QMIGeom_QMIPhysHarmonic()]
	),
	SchrodingerKosloffPropagator(),
]

displayOptions   = { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
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
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[k0_x,k0_y,k0_z],'a0':[gaussWidth_x,gaussWidth_y,gaussWidth_z],'size':size,'gridSize':[16,16,16]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.blockedDOFs='xyzXYZ' # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],**displayOptions)
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.blockedDOFs=''      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],**displayOptionsPot)
potentialBody.material  = QMParametersHarmonic(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStateHarmonic(se3=[potentialCenter,Quaternion((1,0,0),0)])
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
	qt.controller.setWindowTitle("Gaussian packet in 3D harmonic potential")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

