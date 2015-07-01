#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size1d   = 30
halfSize  = [size1d,size1d,0.1]

# wavepacket parameters
k0_x         = 0.0
k0_y         = 0.0
gaussWidth_x = 0.5
gaussWidth_y = 0.5
potentialCoefficient= [-20,0,0]
potentialMaximum    = 20;
GRIDSIZE = [2**8 ,2**8 ]

# potential parameters
potentialCenter   = [ 0, 0 ,0  ]
potentialHalfSize = halfSize # size ??

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
	SchrodingerKosloffPropagator(),
]

displayOptions   = { 'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                    }
displayOptionsPot= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                    }

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[-3,5,0],'t0':0,'k0':[k0_x,k0_y,0],'a0':[gaussWidth_x,gaussWidth_y,0],'gridSize':GRIDSIZE}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.setAnalytic()        # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(partsScale=10,renderWireLight=True,**displayOptions)])
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()          # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],wire=True,displayOptions=[QMDisplayOptions(partsScale=1,**displayOptionsPot)])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.01

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Gaussian packet in 2D Coulomb potential")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

