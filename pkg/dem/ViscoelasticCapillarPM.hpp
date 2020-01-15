#pragma once
#include "ViscoelasticPM.hpp"
#include <boost/unordered_map.hpp>
#include <core/PartialEngine.hpp>
#include <functional>

namespace yade { // Cannot have #include directive inside.

class ViscElCapMat : public ViscElMat {
	public:
		virtual ~ViscElCapMat();
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR(ViscElCapMat,ViscElMat,"Material for extended viscoelastic model of contact with capillary parameters.",
		((bool,Capillar,false,,"True, if capillar forces need to be added."))
#ifdef YADE_LIQMIGRATION
		((bool,LiqMigrEnabled,true,,"True, if liquid migration mechanism is needed. On by default."))
#endif
		((Real,Vb,0.0,,"Liquid bridge volume [m^3]"))
		((Real,gamma,0.0,,"Surface tension [N/m]"))
		((Real,theta,0.0,,"Contact angle [°]"))
		((Real,dcap,0.0,,"Damping coefficient for the capillary phase [-]"))
		((std::string,CapillarType,"",,"Different types of capillar interaction: Willett_numeric, Willett_analytic [Willett2000]_ , Weigert [Weigert1999]_ , Rabinovich [Rabinov2005]_ , Lambert (simplified, corrected Rabinovich model) [Lambert2008]_ ")),
		createIndex();
	);
	// clang-format on
	REGISTER_CLASS_INDEX(ViscElCapMat,ViscElMat);
};
REGISTER_SERIALIZABLE(ViscElCapMat);

/// Interaction physics
enum CapType {None_Capillar, Willett_numeric, Willett_analytic, Weigert, Rabinovich, Lambert, Soulie};
class ViscElCapPhys : public ViscElPhys{
	typedef Real (* CapillarFunction)(const ScGeom& geom, ViscElCapPhys& phys);
	public:
		virtual ~ViscElCapPhys();
		Real R;
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR(ViscElCapPhys,ViscElPhys,"IPhys created from :yref:`ViscElCapMat`, for use with :yref:`Law2_ScGeom_ViscElCapPhys_Basic`.",
		((bool,Capillar,false,,"True, if capillar forces need to be added."))
		((bool,liqBridgeCreated,false,,"Whether liquid bridge was created, only after a normal contact of spheres"))
		((bool,liqBridgeActive,false,, "Whether liquid bridge is active at the moment"))
		((Real,sCrit,false,,"Critical bridge length [m]"))
		((Real,Vb,0.0,,"Liquid bridge volume [m^3]"))
		((Real,gamma,0.0,,"Surface tension [N/m]"))
		((Real,theta,0.0,,"Contact angle [rad]"))
		((CapType,CapillarType,None_Capillar,,"Different types of capillar interaction: Willett_numeric, Willett_analytic, Weigert, Rabinovich, Lambert, Soulie"))
		((Real,dcap,0.0,,"Damping coefficient for the capillary phase [-]"))
#ifdef YADE_LIQMIGRATION
		((bool,LiqMigrEnabled,,,"True, if liquid migration mechanism is needed."))
		((Real,Vmax,0.0,,"Maximal liquid bridge volume [m^3]"))
		((Real,Vf1,0.0,, "Liquid which will be returned to the 1st body after rupture [m^3]"))
		((Real,Vf2,0.0,, "Liquid which will be returned to the 2nd body after rupture [m^3]"))
#endif
		,
		createIndex();
	)
	// clang-format on
	REGISTER_CLASS_INDEX(ViscElCapPhys,ViscElPhys);
};
REGISTER_SERIALIZABLE(ViscElCapPhys);

/// Convert material to interaction physics.
class Ip2_ViscElCapMat_ViscElCapMat_ViscElCapPhys: public Ip2_ViscElMat_ViscElMat_ViscElPhys {
	public :
		virtual void go(const shared_ptr<Material>& b1,
					const shared_ptr<Material>& b2,
					const shared_ptr<Interaction>& interaction);
	// clang-format off
	YADE_CLASS_BASE_DOC(Ip2_ViscElCapMat_ViscElCapMat_ViscElCapPhys,Ip2_ViscElMat_ViscElMat_ViscElPhys,"Convert 2 instances of :yref:`ViscElCapMat` to :yref:`ViscElCapPhys` using the rule of consecutive connection.");
	// clang-format on
	FUNCTOR2D(ViscElCapMat,ViscElCapMat);
};
REGISTER_SERIALIZABLE(Ip2_ViscElCapMat_ViscElCapMat_ViscElCapPhys);

/// Constitutive law
class Law2_ScGeom_ViscElCapPhys_Basic: public LawFunctor {
	public :
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		static Real Willett_numeric_f     (const ScGeom& geom, ViscElCapPhys& phys);
		static Real Willett_analytic_f    (const ScGeom& geom, ViscElCapPhys& phys);
		static Real Weigert_f             (const ScGeom& geom, ViscElCapPhys& phys);
		static Real Rabinovich_f          (const ScGeom& geom, ViscElCapPhys& phys);
		static Real Lambert_f             (const ScGeom& geom, ViscElCapPhys& phys);
		static Real Soulie_f              (const ScGeom& geom, ViscElCapPhys& phys);
		static Real None_f                (const ScGeom& geom, ViscElCapPhys& phys);
		std::deque<std::function<Real(const ScGeom& geom, ViscElCapPhys& phys)> > CapFunctionsPool;
		Real critDist(const Real& Vb, const Real& R, const Real& Theta);
	FUNCTOR2D(ScGeom,ViscElCapPhys);
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(Law2_ScGeom_ViscElCapPhys_Basic,LawFunctor,"Extended version of Linear viscoelastic model with capillary parameters.",
		((OpenMPAccumulator<Real>,VLiqBridg,,Attr::noSave,"The total volume of liquid bridges"))
		((OpenMPAccumulator<int>, NLiqBridg,,Attr::noSave,"The total number of liquid bridges"))
		,{
//enum CapType {None_Capillar, Willett_numeric, Willett_analytic, Weigert, Rabinovich, Lambert, Soulie};
			CapFunctionsPool.resize(20, nullptr);
			CapFunctionsPool[None_Capillar] = None_f;
			CapFunctionsPool[Willett_numeric] = Willett_numeric_f;
			CapFunctionsPool[Willett_analytic] = Willett_analytic_f;
			CapFunctionsPool[Weigert] = Weigert_f;
			CapFunctionsPool[Rabinovich] = Rabinovich_f;
			CapFunctionsPool[Lambert] = Lambert_f;
			CapFunctionsPool[Soulie] = Soulie_f;
		 }
		,/* py */
		;
	)
	// clang-format on
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Law2_ScGeom_ViscElCapPhys_Basic);

#ifdef YADE_LIQMIGRATION
typedef boost::unordered_map<Body::id_t, int> mapBodyInt;
typedef boost::unordered_map<Body::id_t, Real> mapBodyReal;
class LiqControl: public PartialEngine{
	public:
		virtual void action();
		void addBodyMapInt( mapBodyInt & m, Body::id_t b );
		void addBodyMapReal( mapBodyReal & m, Body::id_t b, Real addV );
		Real vMax(shared_ptr<Body> b1, shared_ptr<Body> b2);
		Real totalLiqVol(int mask) const;
		Real liqVolBody(id_t id) const;
		bool addLiqInter(id_t id1, id_t id2, Real liq);
		void updateLiquid(shared_ptr<Body> b);
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(LiqControl,PartialEngine,"This engine implements liquid migration model, introduced here [Mani2013]_ . ",
		((int,mask,0,, "Bitmask for liquid  creation."))
		((Real,liqVolRup,0.,, "Liquid volume (integral value), which has been freed after rupture occured, [m^3]."))
		((Real,liqVolShr,0.,, "Liquid volume (integral value), which has been shared among of contacts, [m^3]."))
		((Real,vMaxCoef,0.03,, "Coefficient for vMax, [-]."))
		((bool,particleconserve,false,, "If True, the particle will have the same liquid volume during simulation e.g. liquid will not migrate [false]."))
		,/* ctor */
		,/* py */
		.def("totalLiq",&LiqControl::totalLiqVol,(boost::python::arg("mask")=0),"Return total volume of water in simulation.")
		.def("liqBody",&LiqControl::liqVolBody,(boost::python::arg("id")=-1),"Return total volume of water in body.")
		.def("addLiqInter",&LiqControl::addLiqInter,(boost::python::arg("id1")=-1, boost::python::arg("id2")=-1, boost::python::arg("liq")=-1),"Add liquid into the interaction.")
  );
	// clang-format on
};

Real liqVolIterBody (shared_ptr<Body> b);
REGISTER_SERIALIZABLE(LiqControl);
#endif

} // namespace yade

