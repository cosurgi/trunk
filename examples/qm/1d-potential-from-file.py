#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
startX    = -55 # -100  # -500
end__X    =  55 #  100  #  500

zero_shift_left  = -45
energy_shift_val = -0.067908708333333

dampMarginBandMin = 2
dampMarginBandMax = 10
dampFormulaSmooth = False   # True - uses exp() with smoothed edge, False - uses 'traditional' exp() cut-off with discontinuity in first derivative.
dampExponent      = 4

size1d    = end__X-startX
halfSize  = [size1d/2,0.1,0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB
# The grid size must be a power of 2 to allow FFT. Here 2**13=8192 is used.
gridSize  = 2**12 # 2**11 # 2**16

# wavepacket parameters
k0_x       = 0
gaussWidth = 0.95 #/ 2.0
x0_center  = 6.15 + zero_shift_left
mass       = 1604.391494

# potential parameters
potentialCenter1  = [(startX+end__X)*0.5,0,0]
potentialHalfSize = halfSize
potentialFileName = "LiH_pot.txt"
potentialColumnX  = 19
potentialColumnVal= 20

# draw scale (was potentialValue)
potentialDrawScale  = 15000  # 5000
potentialDrawminusZ = 0 # -340
psiDrawScale        = 30

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialFromFile(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile()],
		[Law2_QMIGeom_QMIPhysFromFile()]
	),
	#SchrodingerAnalyticPropagator(),
	SchrodingerKosloffPropagator(
             FIXMEatomowe_MASS = mass
            ,dampMarginBandMin = dampMarginBandMin
            ,dampMarginBandMax = dampMarginBandMax
            ,dampFormulaSmooth = dampFormulaSmooth
            ,dampExponent      = dampExponent
            ,dampDebugPrint    = True
            ,threadNum         = 1
            ),
	PyRunner(iterPeriod=1,command='myAddPlotData()'),
	PyRunner(iterPeriod=10,command='savePlotData()')
]

displayOptionsPot= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default stripes","hidden","frame","stripes","mesh"]}

stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential - vith values given from a text file

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide)])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=mass)
gaussPacketArg         = {'x0':[x0_center,0,0],'t0':0,'k0':[k0_x,0,0],'a0':[gaussWidth,0,0],'gridSize':[gridSize]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
O.bodies[nid].state.setAnalytic()        # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide,renderWireLight=False)
    ,QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,40), Quaternion((1,0,0),0)))
])
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()          # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody1 = QMBody()
potentialBody1.shape     = QMGeometry(extents=potentialHalfSize,displayOptions=[
                                         QMDisplayOptions(partsScale=potentialDrawScale
                                        ,renderSe3=(Vector3(0,0,potentialDrawminusZ), Quaternion((1,0,0),0))
                                        ,**displayOptionsPot)])
potentialBody1.material  = QMParametersFromFile(
                                          dim=dimensions
                                         ,hbar=1
                                         ,filename=potentialFileName
                                         ,columnX=potentialColumnX
                                         ,shiftX=zero_shift_left
                                         ,shiftVal=energy_shift_val
                                         ,columnVal=potentialColumnVal)
potentialBody1.state     = QMStPotentialFromFile(se3=[potentialCenter1,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody1)

## Define timestep for the calculations
#O.dt=.000001
#O.dt=.001
#O.dt=.2
#O.dt=1
O.dt=6

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

############################################
##### now the part pertaining to plots #####
############################################

from yade import plot
plot.plots={'t':('re','im','abs','population')} #,'pim')}

def myAddPlotData():
	symId=0
	numId=1
	O.bodies[symId].state.update()
	#psiDiff=((O.bodies[symId].state)-(O.bodies[numId].state))
	autocorr   = ((O.bodies[symId].state)|(O.bodies[numId].state))
	population = ((O.bodies[numId].state)|(O.bodies[numId].state))
	plot.addData(t=O.time,re=autocorr.real,im=autocorr.imag,abs=abs(autocorr),population=population.real ) #, pim=population.imag )

def savePlotData():
	plot.saveDataTxt('dt='+str(O.dt)+'_gaussWidth='+str(gaussWidth)+'_cols_'+str(potentialColumnX)+'_'+str(potentialColumnVal)+'.txt')

plot.liveInterval=.2
plot.plot(subPlots=False)

try:
	from yade import qt
	qt.Controller()
	qt.Renderer().blinkHighlight=False
	Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False
	#Gl1_QMGeometry().analyticUsesStepOfDiscrete=False
	qt.View()
	#qt.controller.setWindowTitle("Packet inside a 1D well")
	qt.controller.setWindowTitle('dt='+str(O.dt)+'_gaussWidth='+str(gaussWidth)+' col_x='+str(potentialColumnX)+' col_y='+str(potentialColumnVal))
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

#O.run(20000)

