#ifdef YADE_SPH
#include"SPHEngine.hpp"
#include<core/Scene.hpp>
#include<pkg/dem/ViscoelasticPM.hpp>
#include<pkg/common/Sphere.hpp>

#include<core/State.hpp>
#include<core/Omega.hpp>

void SPHEngine::action(){
  {
    YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
      if(mask>0 && (b->groupMask & mask)==0) continue;
      this->calculateSPHRho(b);
      b->press=std::max(0.0, k*(b->rho - b->rho0));
    } YADE_PARALLEL_FOREACH_BODY_END();
  }
}

void SPHEngine::calculateSPHRho(const shared_ptr<Body>& b) {
  if (b->rho0<0) {
    b->rho0 = rho0;
  }
  Real rho = 0;
  
  // Pointer to kernel function
  KernelFunction kernelFunctionCurDensity = returnKernelFunction (KernFunctionDensity, Norm);
  
  // Calculate rho for every particle
  for(Body::MapId2IntrT::iterator it=b->intrs.begin(),end=b->intrs.end(); it!=end; ++it) {
    const shared_ptr<Body> b2 = Body::byId((*it).first,scene);
    Sphere* s=dynamic_cast<Sphere*>(b->shape.get());
    if(!s) continue;
    
    if (((*it).second)->geom and ((*it).second)->phys) {
      const ScGeom geom = *(YADE_PTR_CAST<ScGeom>(((*it).second)->geom));
      const ViscElPhys phys=*(YADE_PTR_CAST<ViscElPhys>(((*it).second)->phys));
      
      if((b2->groupMask & mask)==0)  continue;
      
      Real Mass = b2->state->mass;
      if (Mass == 0) Mass = b->state->mass;
      
      const Real SmoothDist = (b2->state->pos - b->state->pos).norm();
     
      // [Monaghan1992], (2.7) (3.8) 
      rho += b2->state->mass*kernelFunctionCurDensity(SmoothDist, h);
    }
  }
  // Self mass contribution
  rho += b->state->mass*kernelFunctionCurDensity(0.0, h);
  b->rho = rho;
}

Real smoothkernelLucy(const double & r, const double & h) {
  if (r<=h) {
    // Lucy Kernel function, 
    return 105./(16.*M_PI*h*h*h) * (1 + 3 * r / h) * pow((1.0 - r / h), 3);
  } else {
    return 0;
  }
}

Real smoothkernelLucyGrad(const double & r, const double & h) {
  if (r<=h) {
    // 1st derivative of Lucy Kernel function, 
    return 105./(16.*M_PI*h*h*h) * (-12 * r) * (1/( h * h ) - 2 * r / (h * h * h ) + r * r / (h * h * h * h));
  } else {
    return 0;
  }
}

Real smoothkernelLucyLapl(const double & r, const double & h) {
  if (r<=h) {
    // 2nd derivative of Lucy Kernel function, 
    return 105./(16.*M_PI*h*h*h) * (-12) / (h * h * h * h) * (h * h - 2 * r * h + 3 * r * r);
  } else {
    return 0;
  }
}

KernelFunction returnKernelFunction(const int a, const typeKernFunctions typeF) {
  return returnKernelFunction(a, a, typeF);
}

KernelFunction returnKernelFunction(const int a, const int b, const typeKernFunctions typeF) {
  if (a != b) {
    throw runtime_error("Kernel types should be equal!");
  }
  if (a==Lucy) {
    if (typeF==Norm) {
      return smoothkernelLucy;
    } else if (typeF==Grad) {
      return smoothkernelLucyGrad;
    } else if (typeF==Lapl) {
      return smoothkernelLucyLapl;
    } else {
      KERNELFUNCDESCR
    }
  } else {
    KERNELFUNCDESCR
  }
}

bool computeForceSPH(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I, Vector3r & force) {
  const ScGeom& geom=*static_cast<ScGeom*>(_geom.get());
  Scene* scene=Omega::instance().getScene().get();
  ViscElPhys& phys=*static_cast<ViscElPhys*>(_phys.get());
  
  const int id1 = I->getId1();
  const int id2 = I->getId2();
  
  const BodyContainer& bodies = *scene->bodies;
  
  const State& de1 = *static_cast<State*>(bodies[id1]->state.get());
  const State& de2 = *static_cast<State*>(bodies[id2]->state.get());
  
  const Real Mass1 = bodies[id1]->state->mass;
  const Real Mass2 = bodies[id2]->state->mass;
  
  const Real Rho1 = bodies[id1]->rho;
  const Real Rho2 = bodies[id2]->rho;
  
  const Vector3r xixj = de2.pos - de1.pos;
  
  if (xixj.norm() < phys.h) {
    Real fpressure = 0.0;
    if (bodies[id1]->rho!=0.0 and bodies[id2]->rho!=0.0) {
      // from [Monaghan1992], (3.3), multiply by Mass2, because we need a force, not du/dt
      fpressure = - Mass1 * Mass2 * (
                  bodies[id1]->press/(Rho1*Rho1) + 
                  bodies[id2]->press/(Rho2*Rho2) 
                  ) 
                  * phys.kernelFunctionCurrentPressure(xixj.norm(), phys.h);
    }
    
    force = fpressure*geom.normal;
    return true;
  } else {
    return false;
  }
}
YADE_PLUGIN((SPHEngine));
#endif

