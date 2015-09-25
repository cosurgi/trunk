#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions = 2
size1d     = 300
GRIDSIZE   = [160,128]
halfSize   = [size1d,size1d*(1.0*GRIDSIZE[1]/GRIDSIZE[0]),0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB
halfSize1  = halfSize
halfSize2  = halfSize1


# potential parameters
potentialCoefficient_proton = [ 1.0,0,0]    # unmovable proton
Pot_x_tmp =   5.0
#Pot_y_tmp =  60.0
Pot_y_tmp =   0.0
oczko_x = 2.0*size1d/GRIDSIZE[0]
oczko_y = 2.0*size1d/GRIDSIZE[0]
Pot_x   = -halfSize[0]+(oczko_x)*(1.0*GRIDSIZE[0]/2+int(1.0*Pot_x_tmp/oczko_x))+(oczko_x*0.33) ## wiesza się gdy dam -20    } InteractionLoop.cpp:120
Pot_y   = -halfSize[1]+(oczko_y)*(1.0*GRIDSIZE[1]/2+int(1.0*Pot_y_tmp/oczko_y))+(oczko_y*0.33) ##                  +15 ???  } bo wcale nie jest symetryczne!!
potentialCenter      = [ Pot_x, Pot_y ,0  ]
potentialHalfSize    = halfSize
potentialMaximum     = -100000000; # negative puts ZERO at center, positive - puts this value.
hydrogenEigenFunc_n  = 8
hydrogenEigenFunc_l  = -5

# wavepacket parameters
k0_x         = -0.12
k0_y         = 0
t0           = 160
gaussWidth_x = 30.0
gaussWidth_y = 30.0
potentialCoefficient1= [-1.0,0,0]
potentialCoefficient2= [ 1.0,0,0]

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMPacketHydrogenEigenFunc(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles()],
		[Law2_QMIGeom_QMIPhysCoulombParticlesFree()]
	),
	SchrodingerKosloffPropagator(printIter=20,doCopyTable=False,threadNum=16),#FIXMEatomowe_MASS=1),
	SchrodingerAnalyticPropagator(),
	PyRunner(iterPeriod=1,command='myAddPlotData()')
]

scaleAll=30000
separate_r_R=True
drawFFT=False
fftPOS = size1d if drawFFT else 0.0

def append_dict(d,arg):
    import copy
    ret = copy.deepcopy(d)
    ret.update(arg)
    return ret

displayOptions         = { 'partsScale':scaleAll,'partsSquared':0
                          ,'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5
                          , 'renderRotated45':separate_r_R
                          }

displayOptions1_num    = append_dict(displayOptions,{'renderSe3':(Vector3(size1d*2.4,0,0), Quaternion((1,0,0),0))})
displayOptions2_num    =             displayOptions
displayOptions1_an     = append_dict(displayOptions,{'renderSe3':(Vector3(size1d*2.4,size1d*2.4,0), Quaternion((1,0,0),0))})
displayOptions2_an     = append_dict(displayOptions,{'renderSe3':(Vector3(0,size1d*2.4,0), Quaternion((1,0,0),0))})

#body0.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=1
#                                      ,coefficient=potentialCoefficient1,potentialMaximum=potentialMaximum)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html

## FIXME - przetestować w mathematica z różnie przesuniętymi względem siebie siatkami i różnym rozstawem siatek.
##         głównie chodzi o to, żeby węzły siatek się nie nakrywały.

########## Analytic solutions
body0           = QMBody()
body0.groupMask = 2
body0.shape     = QMGeometry(extents=halfSize1,color=[0.5,0.5,0.5],displayOptions=[QMDisplayOptions(**displayOptions1_an)])
body0.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=2,coefficient=potentialCoefficient1) # m=2 FIXMEatomowe
body0.state     = QMPacketGaussianWave(x0=potentialCenter,t0=t0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setAnalytic()

body1           = QMBody()
body1.groupMask = 2
body1.shape     = QMGeometry(extents=halfSize2,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(**displayOptions2_an)])
body1.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=0.5#,m=2 # FIXMEatomowe
                                      ,coefficient=potentialCoefficient2,potentialMaximum=potentialMaximum)
coulombPacketArg      = {'m1':1,'m2':1,'energyLevel':[hydrogenEigenFunc_n,hydrogenEigenFunc_l,0],'x0':potentialCenter,'gridSize':GRIDSIZE}
body1.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(body1)
O.bodies[nid].state.setAnalytic()


########## Numeric solutions
body2           = QMBody()
body2.groupMask = 1
body2.shape     = QMGeometry(extents=halfSize1,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**displayOptions1_num)])
body2.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=2,coefficient=potentialCoefficient1) # m=2 FIXMEatomowe
body2.state     = QMPacketGaussianWave(x0=potentialCenter,t0=t0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body2)
O.bodies[nid].state.setNumeric()

body3           = QMBody()
body3.groupMask = 1
body3.shape     = QMGeometry(extents=halfSize2,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions2_num)])
body3.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=0.5#,m=2 # m=2 FIXMEatomowe
                                      ,coefficient=potentialCoefficient2,potentialMaximum=potentialMaximum)
body3.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(body3)
O.bodies[nid].state.setNumeric()

############################################
##### now the part pertaining to plots #####
############################################

from yade import plot
## we will have 2 plots:
## 1. t as function of i (joke test function)
## 2. i as function of t on left y-axis ('|||' makes the separation) and z_sph, v_sph (as green circles connected with line) and z_sph_half again as function of t
plot.plots={'t':('error','normAn','normDi')}

def myAddPlotData():
	symId=0
	numId=2
	O.bodies[symId].state.update()
	psiDiff=((O.bodies[symId].state)-(O.bodies[numId].state))
	ERR=(psiDiff|psiDiff).real
	normAN=(abs(O.bodies[symId].state|O.bodies[symId].state))**2
	normDI=(abs(O.bodies[numId].state|O.bodies[numId].state))**2
	print("\nERROR = "+str(ERR))
	print("\nnormAn= "+str(normAN))
	print("\nnormDi= "+str(normDI))
	plot.addData(t=O.time,error=ERR,normAn=normAN,normDi=normDI)

## no QT  ##  plot.liveInterval=2.0
## no QT  ##  plot.plot(subPlots=False)


#O.dt=80
O.dt=0.0000001


for i in range(50):
	O.step()
	O.dt=80
	#O.save(str(sys.argv[0])+"__t="+str(O.time)+".yade.gz")
	if(i>1):
		plot.saveDataTxt(str(sys.argv[0])+"__t="+str(O.time)+"_testDATA.txt")


#O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

## noQT  ##  try:
## noQT  ##  	from yade import qt
## noQT  ##  	qt.Controller()
## noQT  ##  	qt.controller.setWindowTitle("Electron-positron pair in 2D")
## noQT  ##  	qt.Renderer().blinkHighlight=False
## noQT  ##  	qt.View()
## noQT  ##  	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
## noQT  ##  	qt.views()[0].center(False,260) # median=False, suggestedRadius = 5
## noQT  ##  except ImportError:
## noQT  ##  	pass
#O.run(20000)


#### save result for comparison with mathematica
#
# nn=O.bodies[0].state
# ff=open("1d-harmonic-interaction-yade.txt",'w')
# for i in range(nn.gridSize[0]):
#     ff.write(str(nn.iToX(i,0))+" "+str(nn.iToX(96,1))+" "+str((nn.atPsiGlobal([i,96])).real)+" "+str((nn.atPsiGlobal([i,96])).imag)+"\n")
# ff.close()
#Yade [3]: def zapisz(nazwa,x2):
#     ...:     nn=O.bodies[0].state
#     ...:     ff=open(nazwa,'w')
#     ...:     for i in range(nn.gridSize[0]):
#     ...:         ff.write(str(nn.iToX(i,0))+" "+str(nn.iToX(x2,1))+" "+str((nn.atPsiGlobal([i,x2])).real)+" "+str((nn.atPsiGlobal([i,x2])).imag)+"\n")
#     ...:     ff.close()
#     ...:     

