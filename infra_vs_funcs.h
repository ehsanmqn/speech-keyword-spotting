
#ifndef _OFERD_SV_FUNCS_H_
#define _OFERD_SV_FUNCS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_vector.h"
#include "infra_svector.h"

namespace infra {
//-----------------------------------------------------------------------------
/** Performs vector_base-svector multiplication: outcome = v * s 
    @param v A constant reference to a vector_base
    @param s A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::vector_base& v, const infra::svector& s, 
          double& outcome);

//-----------------------------------------------------------------------------
/** Performs svector-vector_base multiplication: outcome = s * v 
    @param s A constant reference to a svector
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::svector& s, const infra::vector_base& v, 
          double& outcome);

//-----------------------------------------------------------------------------
/** Operator * for vector-svector multiplication
    @param v A constant reference to a svector
    @param s A constant reference to a vector_base
    @return the product
*/
double operator*(const infra::vector_base& v, const infra::svector& s);

//-----------------------------------------------------------------------------
/** Operator * for svector-vector multiplication
    @param s A constant reference to a svector
    @param v A constant reference to a vector_base
    @return the product
*/
double operator*(const infra::svector& s, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs vector_base-svector multiplication and adds the outcome to
    what is currently stored in outcome: outcome += v * s 
    @param v A constant reference to a vector_base
    @param s A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::vector_base& v, const infra::svector& s, 
              double& outcome);

//-----------------------------------------------------------------------------
/** Performs svector-vector_base multiplication and adds the outcome to
    what is currently stored in outcome: outcome += s * v 
    @param s A constant reference to a svector
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::svector& s, const infra::vector_base& v, 
              double& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between a vector_base and a svector
    @param v A constant reference to a vector_base
    @param s A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void dist2(const infra::vector_base& v, const infra::svector& s, 
           double& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between a svector and a vector_base
    @param s A constant reference to a svector
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void dist2(const infra::svector& s, const infra::vector_base& v, 
           double& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between a vector_base and a svector
    @param v A constant reference to a vector_base
    @param s A constant reference to a svector
*/
double dist2(const infra::vector_base& v, const infra::svector& s);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between a svector and a vector_base
    @param s A constant reference to a svector
    @param v A constant reference to a vector_base
*/
double dist2(const infra::svector& s, const infra::vector_base& v);
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
