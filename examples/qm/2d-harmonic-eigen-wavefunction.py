#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size1d   = 10
halfSize  = [size1d,size1d*1.5,0.1] # must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# potential parameters
potentialCenter   = [ 0  ,0  ,0  ]
potentialHalfSize = halfSize
potentialCoefficient= [0.5,0.5,0.5]

harmonicOrder_x   = 0
harmonicOrder_y   = 1


O.engines=[
	StateDispatcher([
		St1_QMPacketHarmonicEigenFunc(),
		St1_QMStPotentialHarmonic(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic()],
		[Law2_QMIGeom_QMIPhysHarmonic()]
	),
	SchrodingerAnalyticPropagator(),
	SchrodingerKosloffPropagator(steps=-1 ), # auto
	PyRunner(iterPeriod=1,command='myAddPlotData()')
]

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

displayOptions1        = { 'partsScale':10
                          ,'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5
                          ,'renderWireLight':True
                          }
displayOptions2        = { 'partsScale':10
                          ,'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5
                          ,'renderWireLight':True
                          }
displayOptionsPot      = { 'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                          ,'renderWireLight':True
                          }

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**displayOptions1)])
analyticBody.material  = QMParameters(dim=dimensions,hbar=1)
harmonicPacketArg      = {'energyLevel':[harmonicOrder_x, harmonicOrder_y, 0],'gridSize':[2**6,2**7]}
analyticBody.state     = QMPacketHarmonicEigenFunc(**harmonicPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic()       # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions2)])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketHarmonicEigenFunc(se3=(Vector3(0,0,0), Quaternion((1,0,0),0)),**harmonicPacketArg)
### interesting, funny ###numericalBody.state     = QMPacketHarmonicEigenFunc(se3=(Vector3(-2,-1,0), Quaternion((1,0,0),0)),**harmonicPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()        # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],displayOptions=[QMDisplayOptions(partsScale=-10,**displayOptionsPot)])
potentialBody.material  = QMParametersHarmonic(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialHarmonic(se3=[potentialCenter,Quaternion((1,0,0),0)])
id_H=O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.02

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'
############################################
##### now the part pertaining to plots #####
############################################

from yade import plot
## we will have 2 plots:
## 1. t as function of i (joke test function)
## 2. i as function of t on left y-axis ('|||' makes the separation) and z_sph, v_sph (as green circles connected with line) and z_sph_half again as function of t
plot.plots={'t':('error')}

def myAddPlotData():
	symId=0
	numId=1
	O.bodies[symId].state.update()
	psiDiff=((O.bodies[symId].state)-(O.bodies[numId].state))	
	plot.addData(t=O.time,error=(psiDiff|psiDiff).real)
plot.liveInterval=.2
plot.plot(subPlots=False)


try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("2D eigenwavefunction in harmonic potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
	Gl1_QMGeometry().analyticUsesStepOfDiscrete=False
	O.bodies[id_H].shape.displayOptions[0].step=[2.0,2.0,2.0]
except ImportError:
	pass

#O.run(20000)

