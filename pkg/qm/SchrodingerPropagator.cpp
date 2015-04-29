// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "SchrodingerPropagator.hpp"
#include "QMInteraction.hpp"
#include <core/Scene.hpp>
#include <unsupported/Eigen/FFT>
#include <functional>   // std::plus, std::multiplies
#include <algorithm>    // std::transform
#include <limits>

#ifdef YADE_FFTW3
#include <fftw3.h>
#endif

YADE_PLUGIN(
	(SchrodingerAnalyticPropagator)
	(SchrodingerKosloffPropagator)
	);

/*********************************************************************************
*
* A N A L Y T I C   P R O P A G A T I O N   of   S H R Ö D I N G E R  E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerAnalyticPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerAnalyticPropagator::~SchrodingerAnalyticPropagator(){};

void SchrodingerAnalyticPropagator::action()
{
	const Real& dt=scene->dt;
//	const Real& time=scene->time;
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
		QMStateAnalytic* analytic=dynamic_cast<QMStateAnalytic*>(b->state.get());
		const Body::id_t& id=b->getId();
		if(analytic) {
			analytic->t += dt;
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

Real SchrodingerKosloffPropagator::eMax()
{
	Real ret(0); // assume that negative maximum energy is not possible
	FOREACH(const shared_ptr<Body>& b, *scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(psi) {// FIXME - px^2+py^2 + potential FIXME, Kosloff eq.2.4 !!!
			int rank = psi->tableValuesPosition.rank();
			Real Ekin(0);
			for(int dim=0 ; dim<rank ; dim++)
				Ekin += std::pow(psi->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 /*FIXME: must be mass here psi->m */);
	
			ret=std::max(ret, Ekin );


//NDimTable<Complexr> Vpsi(psiN___1.dim(),0);
//FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){
//	QMInteractionGeometry* igeom=dynamic_cast<QMInteractionGeometry*>(i->geom.get());
//	if(igeom) {
//		Vpsi+=igeom->potentialValues;      // ψ₁: (potential)
//	}
//};


		}
	};
	return ret;
}

void SchrodingerKosloffPropagator::calcPsiPlus_1(const NDimTable<Complexr>& psi_0,NDimTable<Complexr>& psi_1,
	/*FIXME - remove*/QMStateDiscrete* psi)
{
	Real mass(1); // FIXME - this shouldn't be here
	Real dt=scene->dt;

	Real R   = calcKosloffR(); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(); // FIXME -  that's duplicate here, depends on dt !!

	// FIXME,FIXME ↓ -- this should be somewhere else. Still need to find a good place.
	static bool hasTable(false);
	static NDimTable<Real    > kTable(psi_0.dim());
	if(! hasTable){
		std::size_t size(kTable.size0(0));
		std::size_t rank = psi_0.rank();
		if(rank==1){
		      for(int i=0;i<kTable.size0(0);i++) kTable.at(i)=-1.0*std::pow(psi->iToK((i+size/2)%size,0),2);
		} else
		if(rank==2){
		      for(int i=0;i<kTable.size0(0);i++) 
		      for(int j=0;j<kTable.size0(1);j++) 
			      kTable.at(i,j)=-1.0*std::pow(psi->iToK((i+size/2)%size,0),2)
			                     -    std::pow(psi->iToK((j+size/2)%size,1),2);
		} //FIXME - should work for ALL, any number of !!!!!!!!!!!!!!!!!!!!! dimensions!!!!!!!!!!!!
		hasTable=true;
	//kTable.print();
	}
	
	// prepare the potential
	NDimTable<Complexr> Vpsi(psi_0.dim(),0);
	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
		QMInteractionGeometry* igeom=dynamic_cast<QMInteractionGeometry*>(i->geom.get());
		if(igeom) {
			Vpsi+=igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ
		}
	};
	Vpsi  *=   psi_0;                   // ψᵥ: ψᵥ=V ψ₀
	Vpsi  *= dt     /(hbar*R);          // ψᵥ: ψᵥ=(dt V ψ₀)/(ℏ R)

	psi_1  = FFT(psi_0);                // ψ₁: ψ₁=              ℱ(ψ₀)
	psi_1 *= kTable;                    // ψ₁: ψ₁=           -k²ℱ(ψ₀)
	psi_1   .IFFT();                    // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
	psi_1 *= dt*hbar/(R*2*mass);        // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1.multAdd(psi_0,(1+G/R));       // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀
	psi_1 -= Vpsi;                      // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)

	// FIXME: return std::move(psi_1);
}

void SchrodingerKosloffPropagator::action()
{
	timeLimit.readWallClock();
	Real R   = calcKosloffR(); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(); // FIXME -  that's duplicate here, depends on dt !!
	Real R13 = 1.3*R;
	Real min = 100.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.
//std::cerr << " ............ 1  \n";
	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		const Body::id_t& id=b->getId();
//std::cerr << " ............ 2  \n";
		if(psi) {
			// prepare for the loop, FIXME: it's 1D only
			                 //8 ↓
			NDimTable<Complexr>/*&*/  psi_final(psi->tableValuesPosition);     // will become ψ(t+dt):
			NDimTable<Complexr>  psiN___0 (psi_final);                    // psiN___0=ψ₀
			NDimTable<Complexr>  psiN___1 = {};               // ψ₁:
//std::cerr << " ............ 3  \n";
			calcPsiPlus_1(psiN___0,psiN___1,psi);
			
			Complexr ak(1);
			Complexr ak0=calcAKseriesCoefficient(0,R);
			Complexr ak1=calcAKseriesCoefficient(1,R);
			//1 int j(0);                                                      // ψ(t+dt): psi_final=a₀ψ₀+a₁ψ₁
			//1 FOREACH(Complexr& psi_i, psi_final) psi_i=ak0*psi_i + ak1*psiN___1[j++];
			                                        psi_final*=ak0;           // FIXME - make it look nicer
			                                        psi_final+=psiN___1*ak1;  // FIXME - make it look nicer

			int i(0);
			// never stop when i<R*1.3, unless steps is positive. Auto stop based on std::numeric_limits<Real>::min()
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				//2 std::vector<Complexr> psi_tmp(psiN___0.size());        // ψ₂:
				    NDimTable<Complexr> psiN___2;        // ψ₂:

				//3 calcPsiPlus_1(psiN___1,psi_tmp,psi);                   // ψ₂: psi_tmp  =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				    calcPsiPlus_1(psiN___1,psiN___2,psi);                  // ψ₂: psiN___2 =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)

				//4 FOREACH(Complexr& psi_i, psi_tmp) psi_i*=2;            // ψ₂: psi_tmp  = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) )
				    psiN___2 *= 2;                                         // ψ₂: psiN___2 = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) )
				
				//5 std::vector<Complexr> psiN___2(psiN___0.size());       // ψ₂:
				//5                                                        // ψ₂: psiN___2 = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				//5 std::transform(psi_tmp.begin(), psi_tmp.end(), psiN___0.begin(), psiN___2.begin(), std::minus<Complexr>());
				    psiN___2 -= psiN___0;

				// FIXME - all ak can be precalculated, because R=const
				ak = calcAKseriesCoefficient(i,R);

				//1 j=0;                                                   // ψ(t+dt):  psi_final+=a₂ψ₂
				//6 FOREACH(Complexr& psi_i, psi_final) psi_i+=ak*psiN___2[j++]; 
				                                                              //
				                                                            //!!!!!
				                                                        //!!!!!!!!!!!!!!
				    psi_final += psiN___2*ak; // FIXME - (maybe) inefficient !!!!!!!!!!!!!!!!!!!!!\\


				psiN___0=std::move(psiN___1);                                     // ψ₀ ← ψ₁
				psiN___1=std::move(psiN___2);                                     // ψ₁ ← ψ₂
			}
			//7 Complexr expiRG = std::exp(-1.0*Mathr::I*(R+G));
			//7 FOREACH(Complexr& psi_i, psi_final) psi_i*=expiRG;             // ψ(t+dt): psi_final=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)
			    psi_final *= std::exp(-1.0*Mathr::I*(R+G));

			//std::cerr << "middle ak " << ak << "\n";
			//8 ↑psi->tableValuesPosition=psi_final;
			   //↑ zrobione na górze
			psi->tableValuesPosition=psi_final;

			if(timeLimit.messageAllowed(4)) std::cerr << "final ak=" << std::abs(ak) << " iterations: " << i-1 << "/" << steps << "\n";
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

