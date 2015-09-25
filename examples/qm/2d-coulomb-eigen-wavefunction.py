#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
#size1d    =  8000         # bardzo wysokie n
#GRIDSIZE  = [2**15,2**15] # bardzo wysokie n
SC        = 1
size1d    = 120*SC
GRIDSIZE  = [SC*2**8,SC*2**8]
halfSize  = [size1d,size1d,0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# potential parameters
SH0= -size1d+(2.0*size1d/GRIDSIZE[0])*(1.0*GRIDSIZE[0]/2)+(1.0*size1d/GRIDSIZE[0])
SH1= -size1d+(2.0*size1d/GRIDSIZE[1])*(1.0*GRIDSIZE[1]/2)+(1.0*size1d/GRIDSIZE[1])
potentialCenter      = [ SH0,SH1 ,0  ]
potentialHalfSize    = halfSize
potentialCoefficient = [-1,0,0] # FIXMEatomowe
potentialMaximum     = -10000; # negative puts ZERO at center, positive - puts this value.

hydrogenEigenFunc_n   = 3
hydrogenEigenFunc_l   = 1


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
	SchrodingerKosloffPropagator(FIXMEatomowe_MASS=1.0,steps=-1,virialCheck=False,threadNum=16), # auto
	SchrodingerAnalyticPropagator(),
	PyRunner(iterPeriod=1,command='myAddPlotData()')
]

## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential barrier - as a box with given potential

displayOptions1        = { 'renderWireLight':True,'partsScale':250
                          ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
#		    ,'partsSquared':1
                          ,'renderMaxTime':0.5
                          }
displayOptions2        = { 'renderWireLight':False,'partsScale':250
                          ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
#		    ,'partsSquared':1
                          ,'renderMaxTime':0.5
                          }
displayOptionsPot      = { 'renderWireLight':True,'partsScale':250
                          ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                          }

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**displayOptions1)])
analyticBody.material  = QMParameters(dim=dimensions,hbar=1)
coulombPacketArg      = {'m1':1,'m2_is_infinity':True,'energyLevel':[hydrogenEigenFunc_n, hydrogenEigenFunc_l, 0],'x0':potentialCenter,'gridSize':GRIDSIZE}
analyticBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic()       # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions2)])
numericalBody.material  = analyticBody.material
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()        # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],displayOptions=[QMDisplayOptions(**displayOptionsPot)])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient,potentialMaximum=potentialMaximum)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
id_H=O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
#O.dt=10
O.dt=2

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
	qt.controller.setWindowTitle("2D eigenwavefunction in Coulomb potential")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
	Gl1_QMGeometry().analyticUsesStepOfDiscrete=True
	Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False
	O.bodies[id_H].shape.displayOptions[0].step=[2.0,2.0,2.0]
except ImportError:
	pass

#O.run(20000)

