.. _mpy:

MPI parallelization
===================

The module mpy (link) implements parallelization by domain decomposition (distributed memory) using the Message Passing Interface (MPI) implemented by OpenMPI. It aims at exploiting large numbers of cores, where shared memory techniques (OpenMP) are helpess. 
The shared memory and the distributed memory approaches are compatible, i.e. it is possible to run hybrid jobs using both, as shown below (and it may well be the optimal solution in many cases).

Most calls to OpenMPI library are done in Python using mpi4py. For the sake of efficiency some critical communications are triggered via python wrappers of C++ functions wherein messages are produced, sent/received, and processed.

Disclaimer: even though the mpy module provides the function mpirun(), which may seem as a simple replacement for O.run(), setting up a simulation with mpy might be deceptively triavial.
It is anticipated that, in general, a simple replacement of "run" by "mpirun" in an arbitrary script will not work. To understand why, and to tackle the problems, basic knowledge of how MPI works will certainly help (specifically mpi4py). We will try to lower the barrier progressively.



Concepts
________


.. _fig-subdomains:
.. figure:: fig/subdomains.*
	:width: 12cm
	
	Two overlapping subdomains and their intersections. In this situation we have subD1.intersections[id_sd2]==[id4,id5] and subD1.mirrorIntersections[id_sd2]==[id1], with id_sd2 the index of subdomain 2. 


Execution
_________

This section presents methods to execute yade with MPI multiprocessing. In principle the number of processes $np$ can be larger than the number of available cores without problem (this is called oversubscribing, it may also fail depending on OS and MPI implementation). There is no performance gain to expect from oversubscribing, and in production it should be avoided. However it can be useful for experiments (e.g. for testing the examples in this page on a single-core machine).


Interactive mode
----------------
The interactive mode aims primarily at inspecting the simulation after some MPI execution, for debugging for instance. However, functions shown here (especially sendCommand()) may also be usefull to achieve advanced tasks such as controlling transitions between phases of a simulation, collecting and processing results.
The first two flavors may not be used very often in practice, however understanding them is a good way to understand what happens overall.

 - Explicit initialization from python prompt

A pool of yade instances can be spawned with mpy.initialize() as illustrated hereafter. Mind that the next sequences of commands are supposed to be typed directly in the python prompt after starting yade normally, it will not give exactly the same result if it is pasted into a script executed by Yade (see the next section on automatic initialization).

CODE

After mp.initialize(np) the parent instance of yade takes the role of master process (rank=0). It is the only one executing the commands typed directly in the prompt.
The other instances (rank=1 to rank=np-1) are idle and wait for commands.

CODE

Sending commands to the other instances can be done with mpy.sendCommand(), which by default returns the result or the list of results. 

CODE (check that scene pointers are different)
CODE (len(bodies) = 1,0,0,0,...)

Sending commands makes it possible to manage all types of message passing using calls to mpi4py. Every picklable python object (namely, nearly all Yade objects) can be transmitted this way:

CODE (send body)
CODE (len(bodies) = 1,0,0,0,...)

 
 - Explicit initialization from python script
 
Though usefull, the function sendCommand() is not enough to efficiently manipulate the yade instances in all cases. Even basic features of the python language are missing, e.g. function definitions and loops are a problem - in fact every code fragment which can't fit on a single line is. That is a reason why the mpy module provides a mechanism to initialize from a script.

Whenever Yade is started with a script as argument the script name will be remembered, and if initialize() is executed (in the script itself or interactively in the prompt) all Yade instances will be initialized with that same script. It makes distributing function definitions and simulation parameters trivial (and even distributing scene constructions as seen later). This behaviour is very close to what happens very classicaly in the passive mode, i.e. all processes execute the same program.   

If the previous commands are pasted into a script used to start Yade, there is a small surprise, now all instances insert the body.

CODE

That's because all instances executed the script in the initialize() phase. Though logical, this result is not what we want usually if we try to split a simulation into pieces. The solution (typical of all mpi programs) is to use rank of the process in conditionals:

CODE

 - Automatic initialization

Effectively running DEM in parallel on the basis of just the above commands is probably accessible to good hackers but it would be tedious and computationaly innefficient. mpy provides the function mpirun which automatizes most of the steps required for the consistent time integration of a distributed scene. This includes, mainly, splitting the scene in subdomains based on indices assigned to bodies and handling collisions between the subdomains as time integration proceeds.

If needed the first execution of mpirun will call the function initialize(), which can therefore be omitted on user's side in most cases. 

Here is a concrete example where a floor is assigned to master and multiple groups of spheres are assigned to subdomains:


[CODE] test3D
[COMMENTS] merge/not, erase/master/not, w_interaction/not...


If withMerge=True the bodies in master are updated to reflect in the master scene the evolution of their distributed counterparts. This is done once after finishing the required number of iterations in mpirun. This *merge* operation can include updating interactions.
Merging is an expensive task which requires the communication of large messages and, therefore, it should be done purposely and at a reasonable frequency. It can even be the main bottleneck for massively parallel scenes. Nevertheless it can be usefull for debugging using the 3D view, or for various post-processing tasks. Beyond that it is not required for a proper time integration in general.

 - Don't know how to split? Leave it to mpirun
 
 mpirun will decide by itself how to distribute the bodies across several subdomains if XXX=True. In such case the difference between the sequential script and its mpi version is limited to importing mpy and calling mpirun after turning that flag on. 

 [CODE]
 [BRIEF NOTES ON BISSECTION ALGORITHM - reference?]


Passive mode
------------





Centralized scene construction
------------------------------

Distributed scene construction
------------------------------

Problems to expect
------------------

Reduction (partial sums)


Control variables
_________________

 - VERBOSE_OUTPUT


Various remarks
_______________
- sendCommand() has a hardcoded latency of 0.001s to not keep all cores 100\% busy waiting for a command (with possibly little left to OS). If sendCommand() is used at high frequency in complex algorithms it might be beneficial to decrease that sleep time. 