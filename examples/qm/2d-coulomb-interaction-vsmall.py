#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions = 2
size1d     = 250
SMALL      = 1
GRIDSIZE   = [128/SMALL,80/SMALL]
halfSize   = [size1d,size1d*(1.0*GRIDSIZE[1]/GRIDSIZE[0]),0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# wavepacket_1 parameters
potentialCoefficient_1 = [-1.0,0,0]
k0_1                   = [ 0.2,0.0,0.0]
gaussWidth_1           = [ 10  ,10 ,0  ]
t0_1                   = 300
x0_1                   = [0,-20,0]

# wavepacket_2 parameters
potentialCoefficient_2 = [ 1.0,0,0]
k0_2                   = [-0.2,0.0,0.0]
gaussWidth_2           = [ 10  ,10 ,0  ]
t0_2                   = 300
x0_2                   = [0,-30,0]

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles()],
		[Law2_QMIGeom_QMIPhysCoulombParticles()]
	),
	SchrodingerKosloffPropagator(FIXMEatomowe_MASS=1.0,steps=-1,virialCheck=False,printIter=20,doCopyTable=True,threadNum=16),
]

partsScale            = 200000
partsScale_normalized = 4000

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

def append_dict(d,arg):
    import copy
    ret = copy.deepcopy(d)
    ret.update(arg)
    return ret

def placeDraw(di,i,j):
    return append_dict(di,{'renderSe3':(Vector3(halfSize[0]*2.2*i,halfSize[1]*2.2*j,0), Quaternion((1,0,0),0))})

body0           = QMBody()
body0.shape     = QMGeometry(extents=halfSize,color=[0.1,1.0,0.9],displayOptions=[
                       QMDisplayOptions(**          displayOptions_probabilitySurface       )
                     , QMDisplayOptions(**placeDraw(displayOptions_probabilitySurface, 0, 1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReal      ,-1,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalImag      ,-1, 0))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReImFFT   ,-1, 1))
                     ])
body0.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient_1)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
body0.state     = QMPacketGaussianWave(x0=x0_1,t0=t0_1,k0=k0_1,a0=gaussWidth_1,gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setNumeric()

body1           = QMBody()
body1.shape     = QMGeometry(extents=halfSize,color=[0.9,0.1,1.0],displayOptions=[
                       QMDisplayOptions(**          displayOptions_probabilitySurface       )
                     , QMDisplayOptions(**placeDraw(displayOptions_probabilitySurface, 0,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReal      , 1,-1))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalImag      , 1, 0))
                     , QMDisplayOptions(**placeDraw(displayOptions_marginalReImFFT   , 1, 1))
                     ])
body1.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient_2)
body1.state     = QMPacketGaussianWave(x0=x0_2,t0=t0_2,k0=k0_2,a0=gaussWidth_2,gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body1)
O.bodies[nid].state.setNumeric()

O.dt=50

O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle(sys.argv[0])
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,size1d*1.5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

