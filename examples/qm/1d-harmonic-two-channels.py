#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size1d   = 20
halfSize  = [size1d,0.1,0.1] # must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# wavepacket parameters
k0_x       = 6
gaussWidth = 0.5

# potential parameters
potentialCenter   = [ 0.0,0  ,0  ]
potentialHalfSize = Vector3(size1d,3,3)
potentialCoefficient = [0.5,0.5,0.5]
potentialCoefficient2= [0.8,0.8,0.8]
potentialCoefficientby2= [1.0,1.0,1.0]

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialHarmonic(),
	])
	,SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	])
	,InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic()],
		[Law2_QMIGeom_QMIPhysHarmonic()]
	)
	,SchrodingerAnalyticPropagator()
	,SchrodingerKosloffPropagator(steps=-1, useGroupMaskBool=True, useGroupThisMask=4)
	,SchrodingerKosloffPropagator(steps=-1, useGroupMaskBool=True, useGroupThisMask=8)
	,PyRunner(iterPeriod=1,command='myAddPlotData()')
]

stepRenderStripes=["default stripes","hidden","frame","stripes","mesh"]
stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
displayOptions   = { 'partAbsolute':['default nodes', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']}


## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
#analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],displayOptions=[
     QMDisplayOptions(renderWireLight=False,renderSe3=(Vector3(0,0,2), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[4,0,0],'t0':0,'k0':[k0_x,0,0],'a0':[gaussWidth,0,0],'gridSize':[2**10],'harmonic':[1,1,1],'w0':potentialCoefficientby2}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic() # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.groupMask = 4
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(renderWireLight=False,renderSe3=(Vector3(0,0,2), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential:
potentialBody = QMBody()
potentialBody.groupMask = 4
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[
     QMDisplayOptions(stepRender=stepRenderHide,partsScale=-10,**displayOptions)
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),pi)),**displayOptions)
])
potentialBody.material  = QMParametersHarmonic(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialHarmonic(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## 4: The numerical one: (another channel)
numericalBody2 = QMBody()
numericalBody2.groupMask = 8
numericalBody2.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(renderWireLight=False,renderSe3=(Vector3(0,0,3), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
numericalBody2.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
gaussPacketArg2        = {'x0':[4,0,0],'t0':0,'k0':[-k0_x*1.5,0,0],'a0':[gaussWidth,0,0],'gridSize':[1024],'harmonic':[1,1,1],'w0':potentialCoefficientby2}
numericalBody2.state     = QMPacketGaussianWave(**gaussPacketArg2)
nid=O.bodies.append(numericalBody2)
O.bodies[nid].state.setNumeric()      # is being propagated by SchrodingerKosloffPropagator

## 5: The box with potential: (another channel)
potentialBody2 = QMBody()
potentialBody2.groupMask = 8
potentialBody2.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[
     QMDisplayOptions(stepRender=stepRenderHide,partsScale=-10,**displayOptions)
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),pi)),**displayOptions)
])
potentialBody2.material  = QMParametersHarmonic(dim=dimensions,hbar=1,coefficient=potentialCoefficient2)
potentialBody2.state     = QMStPotentialHarmonic(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody2)


## Define timestep for the calculations
O.dt=.01

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
	qt.View()
	qt.Controller()
	qt.controller.setWindowTitle("1D gaussian packet in harmonic potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
except ImportError:
	pass

#O.run(20000)

