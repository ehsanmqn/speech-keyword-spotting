//*****************************************************************************
#include "infra_ss_funcs.h"
#include "infra_svector.imp"
#include "infra_exception.h"

//-----------------------------------------------------------------------------
void infra::prod(const infra::svector& u, const infra::svector& v,
		 double& outcome) {

  infra_assert(u.size() == v.size(), "When calling prod(), both svectors "
	       << "must be of equal size. In this case, the sizes are "
	       << u.size() << " and " << v.size() << " respectively");

  // init the outcome to zero
  outcome = 0.0;

  // calculate the inner product
  infra::svector::const_iterator u_iter = u.begin();
  infra::svector::const_iterator u_end = u.end();
  infra::svector::const_iterator v_iter = v.begin();
  infra::svector::const_iterator v_end = v.end();

  while(u_iter < u_end && v_iter < v_end) {
    if(u_iter->index == v_iter->index) {
      outcome += u_iter->value * v_iter->value;
      ++u_iter;
      ++v_iter;
    }
    else if(u_iter->index < v_iter->index) {
      ++u_iter;
    }
    else {
      ++v_iter;
    }
  }
}

//-----------------------------------------------------------------------------
double infra::prod(const infra::svector& u, const infra::svector& v) {

  // call the prod function 
  double outcome;
  prod(u,v,outcome);
  return outcome;
}

//-----------------------------------------------------------------------------
void infra::add_prod(const infra::svector& u, const infra::svector& v,
		     double& outcome) {
  
  infra_assert(u.size() == v.size(), "When calling add_prod(), both svectors "
	       << "must be of equal size. In this case, the sizes are "
	       << u.size() << " and " << v.size() << " respectively");

  // calculate the inner product
  infra::svector::const_iterator u_iter = u.begin();
  infra::svector::const_iterator u_end = u.end();
  infra::svector::const_iterator v_iter = v.begin();
  infra::svector::const_iterator v_end = v.end();

  while(u_iter < u_end && v_iter < v_end) {
    if(u_iter->index == v_iter->index) {
      outcome += u_iter->value * v_iter->value;
      ++u_iter;
      ++v_iter;
    }
    else if(u_iter->index < v_iter->index) {
      ++u_iter;
    }
    else {
      ++v_iter;
    }
  }
}

//-----------------------------------------------------------------------------
void infra::dist2(const infra::svector& u, const infra::svector& v,
		  double& outcome) {

  infra_assert(u.size() == v.size(), "When calling dist2(), both svectors "
	       << "must be of equal size. In this case, the sizes are "
	       << u.size() << " and " << v.size() << " respectively");
    
  // init the outcome to zero
  outcome = 0.0;

  // calculate the inner product
  infra::svector::const_iterator u_iter = u.begin();
  infra::svector::const_iterator u_end = u.end();
  infra::svector::const_iterator v_iter = v.begin();
  infra::svector::const_iterator v_end = v.end();

  while(u_iter < u_end && v_iter < v_end) {
    if(u_iter->index == v_iter->index) {
      outcome += (u_iter->value - v_iter->value) *
	(u_iter->value - v_iter->value);
      ++u_iter;
      ++v_iter;
    }
    else if(u_iter->index < v_iter->index) {
      ++u_iter;
    }
    else {
      ++v_iter;
    }
  }
}

//-----------------------------------------------------------------------------
double infra::dist2(const infra::svector& u, const infra::svector& v) {

  // call the prod function 
  double outcome;
  dist2(u,v,outcome);
  return outcome;
}



//*****************************************************************************
//                                     E O F
//*****************************************************************************
