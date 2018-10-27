
//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_vs_funcs.h"
#include "infra_vector.imp"
#include "infra_svector.imp"
#include "infra_exception.h"

//-----------------------------------------------------------------------------
void infra::prod(const infra::vector_base& v, const infra::svector& s,
		 double& outcome) {

  infra_assert(v.size() == s.size(), "When calling prod(), both vector_bases "
	       << "must be of equal size. In this case, the sizes are "
	       << v.size() << " and " << s.size() << " respectively");

  // init the outcome to zero
  outcome = 0.0;

  // calculate the inner product
  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    outcome += s_iter->value * v[s_iter->index];
    ++s_iter;
  }
}

//-----------------------------------------------------------------------------
void infra::prod(const infra::svector& s, const infra::vector_base& v,
		 double& outcome) {
  prod(v,s,outcome);
}

//-----------------------------------------------------------------------------
double operator*(const infra::vector_base& v, const infra::svector& s) {

  // call the prod function 
  double outcome;
  prod(v,s,outcome);
  return outcome;
}

//-----------------------------------------------------------------------------
double operator*(const infra::svector& s, const infra::vector_base& v) {
  // call the prod function 
  double outcome;
  prod(s,v,outcome);
  return outcome;
}

//-----------------------------------------------------------------------------
void infra::add_prod(const infra::vector_base& v, const infra::svector& s,
		     double& outcome) {

  infra_assert(v.size() == s.size(), "When calling add_prod(), both vector_bases "
	       << "must be of equal size. In this case, the sizes are "
	       << v.size() << " and " << s.size() << " respectively");

  // calculate the inner product
  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    outcome += s_iter->value * v[s_iter->index];
    ++s_iter;
  }
}

//-----------------------------------------------------------------------------
void infra::add_prod(const infra::svector& s, const infra::vector_base& v,
		     double& outcome) {
  add_prod(v,s,outcome);
}

//-----------------------------------------------------------------------------
void infra::dist2(const infra::vector_base& v, const infra::svector& s,
		  double& outcome) {

    infra_assert(v.size() == s.size(), "When calling dist2(), both vector_bases "
	       << "must be of equal size. In this case, the sizes are "
	       << v.size() << " and " << s.size() << " respectively");

  // init the outcome to zero
  outcome = 0.0;

  // calculate the inner product
  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    outcome += (s_iter->value - v[s_iter->index])
      * (s_iter->value - v[s_iter->index]);
    ++s_iter;
  }
}

//-----------------------------------------------------------------------------
double infra::dist2(const infra::vector_base& v, const infra::svector& s) {

  // call the prod function 
  double outcome;
  dist2(v,s,outcome);
  return outcome;
}

//*****************************************************************************
//                                     E O F
//*****************************************************************************
