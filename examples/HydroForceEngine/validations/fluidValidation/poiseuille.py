from __future__ import print_function
#########################################################################################################################################################################
# Author: Raphael Maurin, raphael.maurin@imft.fr
# 22/11/2017
#
# Script to validate the 1D volume-averaged fluid resolution of HydroForceEngine, without accounting for turbulence. Simulate a Poiseuille flow
# No particles are simulated and only the fluid resolution is tested and compared to the analytical solution
#
# Consider a newtonian fluid flow of height "fluidHeight" driven by a  pressure gradient (dpdx) between two fixed walls (iusl = 0, fixed condition), 
# without turbulence (iturbu=0). 
# 
# The obtained 1D fluid velocity profile (vxFluid) is compared to the analytical solution in the figure "figPoiseuille.png"
#
#########################################################################################################################################################################
from builtins import range
import numpy as np

################################
# PARAMETERS AND CONFIGURATION DEFINITIONS
#Fluid flow characteristics
fluidHeight = 1.	#Fluid height, i.e. gap between the two walls, in m
dpdx =-0.10		#Driving pressure gradient, in Pa/m
densFluid = 1e3		#Fluid density kg/m^3, water
kinematicViscoFluid = 1e-6	#Kinematic fluid viscosity, m^2/s, water

#Simulation characteristic
tfin = 1e7		#Total simulated time, in s
dtFluid = 1e4		#Resolution time step, in s
#Mesh
ndimz = 101   #Number of grid cells in the height
dz =  fluidHeight/(1.0*(ndimz-1))	#spatial step between two mesh nodes

# Initialization of the fluid velocity
vxFluid =np.zeros(ndimz)

################################
# CREATE THE ASSOCIATED HYDROFORCEENGINE OBJECT FOR THE FLUID RESOLUTION
a = HydroForceEngine(densFluid=densFluid,viscoDyn=kinematicViscoFluid*densFluid,deltaZ=dz,nCell=ndimz,vxFluid=np.array(vxFluid),dpdx=dpdx,gravity = Vector3(0,0,-9.81),phiMax=0.61,turbulentViscosity = np.zeros(ndimz),iturbu=0,iusl=0,uTop=0.,phiPart=np.zeros(ndimz),vxPart = np.zeros(ndimz),averageDrag=np.zeros(ndimz))# Define the minimum parameters of the HydroForceEngine for the fluid resolution

################################
# FLUID RESOLUTION
a.fluidResolution(tfin,dtFluid)# Call the fluid resolution associated with the defined HydroForceEngine. Use the parameters defined just above. 
print("\nFluid Resolution finished !  ","max(vxFluid)=",np.max(a.vxFluid),"\n")


################################
# COMPARISON TO THE ANALYTICAL SOLUTION
print("Comparison to the analytical solution\n")
vxFluid = np.array(a.vxFluid)

# Re-create the fluid resolution mesh: regular spacing, with differences at the bottom (0) and at the top (ndimz-1). 
zScale=np.zeros(ndimz+1)
zScale[0] = 0.
zScale[1] = 0.5*dz
for i in range(2,ndimz):
	zScale[i]=zScale[i-1]+dz
zScale[ndimz] = zScale[ndimz-1] + 0.5*dz


# Analytical solution of the fluid velocity
upois=-dpdx*1/2./(kinematicViscoFluid*densFluid)*(fluidHeight*zScale-zScale*zScale)


## plots
from pylab import *
from matplotlib import pyplot
import matplotlib.gridspec as gridspec
figure(1)
ax1 =subplot(111)
p1=ax1.plot(vxFluid,zScale,'-ob',label='Yade 1D fluid res.')	#Simulation results = blue points
p1=ax1.plot(upois,zScale,'-r',label='Analytical solution',lw = 2) #Analytical result = red line
xlabel('$\\left<v_x\\right>^f (m/s)$')
ylabel('z (m)')
legend(loc=1)
savefig('figPoiseuille.png')
show()
print("finished! exit\n")
exit()	#Leave the simulation at the end 
