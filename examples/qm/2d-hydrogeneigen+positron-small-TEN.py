#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions = 2
SMALLER    = 2
size1d     = 800/SMALLER
GRIDSIZE   = [320/SMALLER,256/SMALLER] # GRIDSIZE   = [320,256]
halfSize   = [size1d,size1d*(1.0*GRIDSIZE[1]/GRIDSIZE[0]),0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# potential parameters
potentialCoefficient_proton = [ 1.0,0,0]    # unmovable proton
Pot_x_tmp = 100.0
Pot_y_tmp = 90.0
oczko_x = 2.0*size1d/GRIDSIZE[0]
oczko_y = 2.0*size1d/GRIDSIZE[0]
Pot_x   = -halfSize[0]+(oczko_x)*(1.0*GRIDSIZE[0]/2+int(1.0*Pot_x_tmp/oczko_x))+(oczko_x*0.5) ## wiesza się gdy dam -20    } InteractionLoop.cpp:120
Pot_y   = -halfSize[1]+(oczko_y)*(1.0*GRIDSIZE[1]/2+int(1.0*Pot_y_tmp/oczko_y))+(oczko_y*0.5) ##                  +15 ???  } bo wcale nie jest symetryczne!!
potentialCenter      = [ Pot_x, Pot_y ,0  ]
potentialHalfSize    = halfSize # size ??
potentialMaximum     = -100000000; # negative puts ZERO at center, positive - puts this value.

# wavepacket_1 parameters, incoming positron 
potentialCoefficient_positron = [ 1.0 ,0.0,0.0]
k0_positron                   = [-0.12,0.0,0.0]
gaussWidth_positron           = [ 60  ,60 ,0  ]
t0_positron                   = 4500/SMALLER
x0_positron                   = [0,-140+Pot_y,0]

# wavepacket_2 parameters, electron on stationary orbit
hydrogenEigenFunc_n_electron  = 8
hydrogenEigenFunc_l_electron  = 7
potentialCoefficient_electron = [-1.0,0,0]

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialCoulomb(),
		St1_QMPacketHydrogenEigenFunc(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[ Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles()
                 ,Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential()],
		[Law2_QMIGeom_QMIPhysCoulombParticles(),Law2_QMIGeom_QMIPhysCoulombParticleInPotential()]
	),
	SchrodingerKosloffPropagator(FIXMEatomowe_MASS=1.0,steps=-1,virialCheck=False,printIter=20,doCopyTable=True,threadNum=32),
	SchrodingerAnalyticPropagator(),
]

partsScale            = 4000000
partsScale_normalized = 40000

displayOptions_probabilitySurface = { 'partsScale':partsScale,'partsSquared':0
                                     ,'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'renderMaxTime':0.5}
displayOptions_marginalReal       = { 'partsScale':partsScale_normalized,'partsSquared':0
                                     ,'marginalDensityOnly':False,'marginalNormalize':True
                                     ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'renderMaxTime':0.5}
displayOptions_marginalImag       = { 'partsScale':partsScale_normalized,'partsSquared':0
                                     ,'marginalDensityOnly':False,'marginalNormalize':True
                                     ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'renderMaxTime':0.5}
displayOptions_marginalReImFFT    = { 'partsScale':partsScale_normalized,'partsSquared':0
                                     ,'marginalDensityOnly':False,'marginalNormalize':True
                                     ,'renderFFT':True
                                     ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'renderMaxTime':0.5}
displayOptionsPot                 = { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                                     }
displayOptions_marginalReal_wire  = { 'partsScale':partsScale_normalized,'partsSquared':0
                                     ,'marginalDensityOnly':False,'marginalNormalize':True
                                     ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                     ,'renderMaxTime':0.5}


def append_dict(d,arg):
    import copy
    ret = copy.deepcopy(d)
    ret.update(arg)
    return ret

def placeDraw(di,i,j):
    return append_dict(di,{'renderSe3':(Vector3(halfSize[0]*2.2*i,halfSize[1]*2.2*j,0), Quaternion((1,0,0),0))})

## 1(0): positron
body0           = QMBody()
body0.shape     = QMGeometry(extents=halfSize,color=[0.9,0.5,0.5],displayOptions=[
                       QMDisplayOptions(**          displayOptions_probabilitySurface       )
                     , QMDisplayOptions(**placeDraw(displayOptions_probabilitySurface, 0,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReal      , 1,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalImag      , 1, 0))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReImFFT   , 1, 1))
                     ])
body0.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient_positron)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
body0.state     = QMPacketGaussianWave(x0=x0_positron,t0=t0_positron,k0=k0_positron,a0=gaussWidth_positron,gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setNumeric()

## 2(1): electron
body1           = QMBody()
body1.shape     = QMGeometry(extents=halfSize,color=[0.5,0.5,0.9],displayOptions=[
                       QMDisplayOptions(**          displayOptions_probabilitySurface       )
                     , QMDisplayOptions(**placeDraw(displayOptions_probabilitySurface, 0, 1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReal      ,-1,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalImag      ,-1, 0))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReImFFT   ,-1, 1))
                     ])
body1.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient_electron)
coulombPacketArg      = {'m1':1,'m2_is_infinity':True,'energyLevel':[hydrogenEigenFunc_n_electron, hydrogenEigenFunc_l_electron, 0],'x0':potentialCenter,'gridSize':GRIDSIZE}
body1.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(body1)
O.bodies[nid].state.setNumeric()

## 3(2): unmovable proton potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],wire=True,displayOptions=[QMDisplayOptions(partsScale=1,**displayOptionsPot)])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient_proton)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## 4(3): Analytical packet
analyticBody = QMBody()
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**placeDraw(displayOptions_marginalReal_wire ,-1,-1))])
analyticBody.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=[-1.0,0.0,0.0])
analyticBody.state     = QMPacketHydrogenEigenFunc(t0=-270,**coulombPacketArg)
nid=O.bodies.append(analyticBody)
O.bodies[nid].state.setAnalytic()       # is propagated as analytical solution - no calculations involved

O.dt=200

#O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

## try:
## 	from yade import qt
## 	qt.Controller()
## 	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
## 	qt.controller.setWindowTitle(sys.argv[0])
## 	qt.Renderer().blinkHighlight=False
## 	qt.Renderer().light1Pos=Vector3( 1175,1130,500)
## 	qt.Renderer().light2Pos=Vector3(-1130, 575,230)
## 	qt.View()
## 	#qt.Renderer().light2Pos=Vector3(Pot_x,Pot_y,30)
## 	qt.views()[0].center(False,size1d*1.5) # median=False, suggestedRadius = 5
## 
## except ImportError:
## 	pass
## 

#for i in range(100):
#	O.step()
#	if(i%5==0):
#		O.save(str(sys.argv[0])+"_"+str(O.iter)+".yade.bz2")


O.dt=0.0000001

for i in range(81):
	O.step()
	O.dt=100
	if(i%5==0):
		O.save(str(sys.argv[0])+"_"+str(O.iter)+".yade.bz2")

