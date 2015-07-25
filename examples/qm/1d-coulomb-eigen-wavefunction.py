#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size1d    = 60
halfSize  = [size1d,0.1,0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB
GRIDSIZE  = 2**11

# potential parameters
potentialCenter      = [ -size1d+(2.0*size1d/GRIDSIZE)*(1.0*GRIDSIZE/2)+(1.0*size1d/GRIDSIZE) ,0  ,0  ]
potentialHalfSize    = Vector3(size1d,3,3)
potentialCoefficient = [-1,0,0]
potentialMaximum     = -100;

hydrogenEigenFunc_n   = 2
hydrogenEigenFunc_odd = 1

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
	SchrodingerKosloffPropagator(steps=-1 ), # auto
	SchrodingerAnalyticPropagator(),
]

stepRenderStripes=["default stripes","hidden","frame","stripes","mesh"]
stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
displayOptionsPot= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']}

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.8,0.8,0.8],displayOptions=[
     QMDisplayOptions(renderWireLight=False,partsScale=10,renderSe3=(Vector3(0,0,2), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True
                      ,renderFFTScale=(4,1,1)
                      ,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
analyticBody.material  = QMParameters(dim=dimensions,hbar=1)
coulombPacketArg      = {'energyLevel':[hydrogenEigenFunc_n,hydrogenEigenFunc_odd,0],'x0':potentialCenter,'gridSize':[GRIDSIZE]}
analyticBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic()     # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(renderWireLight=False,partsScale=10,renderSe3=(Vector3(0,0,2), Quaternion((1,0,0),0)))
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True
                      ,renderFFTScale=(4,1,1)
                      ,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0)))
])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,step=[0.2,0.1,0.1],displayOptions=[
     QMDisplayOptions(stepRender=stepRenderHide,partsScale=-10,**displayOptionsPot)
    ,QMDisplayOptions(stepRender=stepRenderHide,renderWireLight=False,renderFFT=True
                      ,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),pi))
                      ,renderFFTScale=(4,1,0.02)
                      ,**displayOptionsPot)
])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient,potentialMaximum=potentialMaximum)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.001
O.dt=.2

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.Controller()
	qt.controller.setWindowTitle("1D eigenwavefunction in Coulomb potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,10) # median=False, suggestedRadius = 5
	Gl1_QMGeometry().analyticUsesStepOfDiscrete=True
	Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False

except ImportError:
	pass

#O.run(20000)

