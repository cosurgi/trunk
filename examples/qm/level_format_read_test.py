#!/usr/bin/python
# -*- coding: utf-8 -*-

# sample size
dimensions= 1
startX    = -110 # -100  # -500
end__X    =  110 #  100  #  500
gridSize  =  2**13 # 2**11 # 2**16 # The grid size must be a power of 2 to allow FFT. Here 2**13=8192 is used.
                                   # FFTW is best with 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ, for e+f either 0 or 1

# energy info
zero_shift_left   = -55
energy_shift_val  = 0 # -0.06790711  ### ciekawy FIXME - gdy tu jest zero to się pojawiają błędy numeryczne, tutaj bym mógł to debugować.

import os
cwd = os.getcwd()
print "------------------------------------------"
print cwd
#print int(cwd[-2:])
print "------------------------------------------"

J_rot_value       = 69 ### int(cwd[-2:])

print "------------------------------------------"
print "J_rot_value="+str(J_rot_value)
print "------------------------------------------"


# NOTKA: QMPacketFromFile nie jest znormalizowany: 1.8897149739656927

# damping
dampMarginBandMin = 30
dampMarginBandMax = 50
dampFormulaSmooth = True    # True - uses exp() with smoothed edge, False - uses 'traditional' exp() cut-off with discontinuity in first derivative.
dampExponent      = 0.016

# wavepacket parameters
k0_x       = 0
gaussWidth = 3.0*0.95 /sqrt(2)
x0_center  = 12.5 + zero_shift_left

# mass
_39K_mass          = 38.963707          # http://physics.nist.gov/PhysRefData/Handbook/Tables/potassiumtable1.htm
_7Li_mass          =  7.016003          # http://physics.nist.gov/PhysRefData/Handbook/Tables/lithiumtable1.htm
electron_mass_in_u =  0.00054857990907  # http://physics.nist.gov/cuu/Constants/Table/allascii.txt
mass               =  1.0/(1/(_39K_mass/electron_mass_in_u)+1/(_7Li_mass/electron_mass_in_u))

# potential parameters
potentialCenter1   = [(startX+end__X)*0.5,0,0]
size1d    = end__X-startX
halfSize  = [size1d/2,0.1,0.1]          # must be three components, because yade is inherently 3D and uses Vector3r.
                                        # Remaining components will be used for AABB
potentialHalfSize  = halfSize
potentialFileName  = "./KLi_2sPi_J=0_do_J=69__v8_PLUS_stan_podstawowy.txt"
potentialColumnX   = 1
potentialColumnVal = J_rot_value + 2

wavePacketFilename    = "./wf_J69.dat"
wavePacketColumnX     = 1
wp_zero_shift_left    = zero_shift_left
wp________shift_val   = 0
wavePacketColumnVal   = 2
fileLevelFormat       = True
fileLevelVibrational  = 11
normalizeWaveFunction = True
expandRightDecay      = True
expandRightDecayPoints= 10000

print "------------------------------------------"
print "J_rot_value="+str(J_rot_value)
print "potentialColumnVal="+str(potentialColumnVal)
print "------------------------------------------"

angstron_to_AU          = 1.0/0.52917721067
# timestep
atomic_time_to_seconds  = 2.418884326509e-17  # http://physics.nist.gov/cuu/Constants/Table/allascii.txt
femto                   = 1e-15
atomic_time_in_femtosec = 1000/(atomic_time_to_seconds/femto)
O.dt=atomic_time_in_femtosec

# DRAWING scale
potentialDrawScale            = 5000  # 5000
energy_at_inifiniity_DRAWONLY = 0.096231526169661
potentialDrawminusZ           = -potentialDrawScale*energy_at_inifiniity_DRAWONLY
psiDrawScale                  = 20

# title of file and window
def titleText(dt,mass,J,width):
	return 'dt='+str(dt)+'_mass='+str(mass)+'_J='+str(J)+'_width='+str(width)

O.engines=[
	StateDispatcher([
		St1_QMPacketFromFile(),
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
            ,dampDebugPrint    = False
            ,threadNum         = 1
            ),
	PyRunner(iterPeriod=1,command='myAddPlotData()'),
	PyRunner(iterPeriod= 50,command='savePlotData()')
#       PyRunner(iterPeriod=20000,command='savePlotData()')
]

displayOptionsPot= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default stripes","hidden","frame","stripes","mesh"]}

displayOptionsDamp= { 'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default stripes","hidden","frame","stripes","mesh"]}

stepRenderHide   =["default hidden","hidden","frame","stripes","mesh"]
## Create:
# 1. analytical gauss packet - only use it to initialise the discrete packet
# 2. discrete packet
# 3. potential - vith values given from a text file

## 1: Analytical packet
originalWP = QMBody()
originalWP.groupMask = 2
originalWP.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide)])
originalWP.material  = QMParticle(dim=dimensions,hbar=1,m=mass)
filePacketArg        = {'filename':wavePacketFilename,'columnX':wavePacketColumnX,'multX':angstron_to_AU,'shiftX':wp_zero_shift_left,'shiftVal':wp________shift_val,'columnVal':wavePacketColumnVal,'gridSize':[gridSize],'fileLevelFormat':fileLevelFormat,'fileLevelVibrational':fileLevelVibrational,'normalizeWaveFunction':normalizeWaveFunction,'expandRightDecay':expandRightDecay,'expandRightDecayPoints':expandRightDecayPoints}
originalWP.state     = QMPacketFromFile(**filePacketArg)
nid=O.bodies.append(originalWP)        # do not append, it is used only to create the numerical one
O.bodies[nid].state.setAnalytic()        # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[
     QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide,renderWireLight=False)
    ,QMDisplayOptions(partsScale=psiDrawScale,stepRender=stepRenderHide,renderWireLight=False,renderFFT=True,renderSe3=(Vector3(0,0,40), Quaternion((1,0,0),0)))
])
numericalBody.material  = originalWP.material
numericalBody.state     = QMPacketFromFile(**filePacketArg)
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()          # is being propagated by SchrodingerKosloffPropagator

## 3: The potential loaded from file
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

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
#O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

############################################
##### now the part pertaining to plots #####
############################################

from yade import plot
plot.plots={'t':('re','im','abs','population')}

def myAddPlotData():
	symId=0
	numId=1
	O.bodies[symId].state.update()
	#psiDiff=((O.bodies[symId].state)-(O.bodies[numId].state))

# stara wersja
#	autocorr   = ((O.bodies[symId].state)|(O.bodies[numId].state))
#	population = ((O.bodies[numId].state)|(O.bodies[numId].state)).real
#	autocorr   = ((O.bodies[symId].state)|(O.bodies[numId].state))
#	population = ((O.bodies[numId].state)|(O.bodies[numId].state)).real

	copy_num   = O.bodies[numId].state.copy()
	zeroTheOutsideOfRange = True
	copy_num.zeroRange([dampMarginBandMin],[size1d-dampMarginBandMax],zeroTheOutsideOfRange,False)
	autocorr   = ((O.bodies[symId].state)|(       copy_num      ))
	population = ((       copy_num      )|(       copy_num      )).real

	plot.addData(t=O.time,re=autocorr.real,im=autocorr.imag,abs=abs(autocorr),population=population )

def savePlotData():
	plot.saveDataTxt(titleText(O.dt,mass,J_rot_value,gaussWidth)+'.txt')

plot.liveInterval=.3
plot.plot(subPlots=False)

try:
	from yade import qt
	qt.Controller()
	qt.Renderer().blinkHighlight=False
	qt.Renderer().bgColor=Vector3(0,0,0)
        qt.Renderer().extraDrawers=[GlExtra_QMEngine(drawDTable=True,dampColor=Vector3(1,1,1)
                    ,dampDisplayOptions=QMDisplayOptions(partsScale=10
                    ,renderSe3=(Vector3(0,0,0), Quaternion((1,0,0),0)),**displayOptionsDamp))]
	Gl1_QMGeometry().analyticUsesScaleOfDiscrete=False
	#Gl1_QMGeometry().analyticUsesStepOfDiscrete=False
	qt.View()
	#qt.controller.setWindowTitle("Packet inside a 1D well")
	qt.controller.setWindowTitle(titleText(O.dt,mass,J_rot_value,gaussWidth))
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5
except ImportError:
	pass

# print "------------------------------------------"
# print "J_rot_value="+str(J_rot_value)
# print "potentialColumnVal="+str(potentialColumnVal)
# print "------------------------------------------"
# 
# #O.run(260001,True)
# O.run(400001,True)
# #O.run(20,True)  # True - oznacza, że nie wychodzi do pythona.
# 
# exit()
# 
