#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size1d   = 10
halfSize  = [size1d,size1d*1.5,0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

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
	SchrodingerKosloffPropagator(steps=-1),
	SchrodingerAnalyticPropagator()
	,PyRunner(iterPeriod=1,command='myAddPlotData()')
]

## Two particles are created - the analytical one, and the numerical one. They
## do not interact, they are two separate calculations in fact.

## The analytical one:

analyticBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(partsScale=40,renderWireLight=True)])
# it's too simple now. Later we will have quarks (up, down, etc.), leptons and bosons as a material.
# So no material for now.
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
#gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[0.4,2,0],'a0':[3,2,0],'gridSize':[128,64]}    # ← previously used arguments, were nice too!
gaussPacketArg         = {'x0':[0,0,0],'t0':0,'k0':[0.4,2,0],'a0':[1.5,2,0],'gridSize':[64,128]} # ← arguments I tried when introducing 3D
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic() # is propagated as analytical solution - no calculations involved

## The numerical one:
numericalBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
numericalBody.groupMask = 1
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(partsScale=40,renderWireLight=True)])
numericalBody.material  = analyticBody.material
# Initialize the discrete wavefunction using the analytical gaussPacket created earlier.
# The wavefunction shape can be anything - as long as it is normalized, in this case the Gauss shape is used.
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()   # is being propagated by SchrodingerKosloffPropagator

## Define timestep for the calculations
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
	qt.controller.setWindowTitle("2D free propagating packet")
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

