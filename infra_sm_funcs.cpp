//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_sm_funcs.h"
#include "infra_vector.imp"
#include "infra_svector.imp"
#include "infra_matrix.imp"
#include "infra_exception.h"

/*---------------------------------------------------------------------------*/
void infra::prod(const infra::matrix_base& M, const infra::svector& s,
		 infra::vector_base& v) {

  infra_assert(M.width() == s.size() &&  M.height() == v.size(),
	       "When calling prod(M,s,v), the dimensions of M must "
	       << "equal the sizes of v and v respectively. In this "
	       << "case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << v.size() << " and " << s.size());

  // init outcome vector
  v.zeros();

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    infra::matrix_base::const_iterator M_iter = M.begin();
    M_iter += (s_iter->index * M.height());
    infra::matrix_base::const_iterator M_end = M_iter;
    M_end += M.height();
    unsigned long i=0;
    while(M_iter < M_end) {
      v[i] += (*M_iter) * s_iter->value;
      ++M_iter;
      ++i;
    }   
    ++s_iter;
  }
}

/*---------------------------------------------------------------------------*/
infra::vector_base operator*(const infra::matrix_base& M, 
                             const infra::svector& s) {

  infra::vector outcome(M.height());
  prod(M, s, outcome);
  return outcome;
}

/*---------------------------------------------------------------------------*/
void infra::prod(const infra::svector& s, const infra::matrix_base& M, 
		 infra::vector_base& v) {

  infra_assert(M.height() == s.size() &&  M.width() == v.size(),
	       "When calling prod(s,M,v), the dimensions of M must "
	       << "equal the sizes of v and v respectively. In this "
	       << "case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << s.size() << " and " << v.size());

  // init outcome vector
  v.zeros();

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    for(unsigned long j=0; j<M.width(); ++j) {
      v[j] += M(s_iter->index,j) * s_iter->value;
    }
    ++s_iter;
  }
}

/*---------------------------------------------------------------------------*/
infra::vector_base operator*(const infra::svector& s,
                             const infra::matrix_base& M) {

  infra::vector outcome(M.width());
  prod(s, M, outcome);
  return outcome;
}

/*---------------------------------------------------------------------------*/
void infra::add_prod(const infra::matrix_base& M, const infra::svector& s,
		     infra::vector_base& v) {

  infra_assert(M.width() == s.size() &&  M.height() == v.size(),
	       "When calling add_prod(M,s,v), the dimensions of M must "
	       << "equal the sizes of v and v respectively. In this "
	       << "case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << v.size() << " and " << s.size());

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    infra::matrix_base::const_iterator M_iter = M.begin();
    M_iter += (s_iter->index * M.height());
    infra::matrix_base::const_iterator M_end = M_iter;
    M_end += M.height();
    unsigned long i=0;
    while(M_iter < M_end) {
      v[i] += (*M_iter) * s_iter->value;
      ++M_iter;
      ++i;
    }   
    ++s_iter;
  }
}

/*---------------------------------------------------------------------------*/
void infra::add_prod(const infra::svector& s, const infra::matrix_base& M, 
		     infra::vector_base& v) {

  infra_assert(M.height() == s.size() &&  M.width() == v.size(),
	       "When calling add_prod(s,M,v), the dimensions of M must "
	       << "equal the sizes of v and v respectively. In this "
	       << "case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << s.size() << " and " << v.size());

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    for(unsigned long j=0; j<M.width(); ++j) {
      v[j] += M(s_iter->index,j) * s_iter->value;
    }
    ++s_iter;
  }
}

/*---------------------------------------------------------------------------*/
void infra::subtract_prod(const infra::matrix_base& M, const infra::svector& s,
			  infra::vector_base& v) {

  infra_assert(M.width() == s.size() &&  M.height() == v.size(),
	       "When calling subtract_prod(M,s,v), the dimensions of M "
	       << "must equal the sizes of v and v respectively. In "
	       << "this case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << v.size() << " and " << s.size());

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    infra::matrix_base::const_iterator M_iter = M.begin();
    M_iter += (s_iter->index * M.height());
    infra::matrix_base::const_iterator M_end = M_iter;
    M_end += M.height();
    unsigned long i=0;
    while(M_iter < M_end) {
      v[i] -= (*M_iter) * s_iter->value;
      ++M_iter;
      ++i;
    }   
    ++s_iter;
  }
}

/*---------------------------------------------------------------------------*/
void infra::subtract_prod(const infra::svector& s, const infra::matrix_base& M,
                          infra::vector_base& v) {

  infra_assert(M.height() == s.size() &&  M.width() == v.size(),
	       "When calling subtract_prod(s,M,v), the dimensions of M must "
	       << "equal the sizes of v and v respectively. In this "
	       << "case, the dimensions of M are " << M.height() << "x"
	       << M.width() << " and the sizes of v and s are "
	       << s.size() << " and " << v.size());

  infra::svector::const_iterator s_iter = s.begin();
  infra::svector::const_iterator s_end = s.end();

  while(s_iter < s_end) {
    for(unsigned long j=0; j<M.width(); ++j) {
      v[j] -= M(s_iter->index,j) * s_iter->value;
    }
    ++s_iter;
  }
}

//*****************************************************************************
//                                     E O F
//*****************************************************************************
