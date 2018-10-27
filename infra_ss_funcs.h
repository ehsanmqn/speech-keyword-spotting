
#ifndef _OFERD_SS_FUNCS_H_
#define _OFERD_SS_FUNCS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_svector.h"

namespace infra {
//-----------------------------------------------------------------------------
/** Performs svector-svector multiplication: outcome = u * v 
    @param u A constant reference to a svector
    @param v A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::svector& u, const infra::svector& v, double& outcome);

//-----------------------------------------------------------------------------
/** Performs svector svector multiplication and returns the answer
    @param s1 A constant reference to a sparse_svector
    @param s2 A constant reference to a sparse_svector
    @return the product
*/
double prod(const infra::svector& u, const infra::svector& v);

//-----------------------------------------------------------------------------
/** Performs svector-svector multiplication and adds the outcome to
    what is currently stored in outcome: outcome += u * v 
    @param u A constant reference to a svector
    @param v A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::svector& u, const infra::svector& v, 
              double& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between two svectors
    @param u A constant reference to a svector
    @param v A constant reference to a svector
    @param outcome A reference to where the outcome will be stored
*/
void dist2(const infra::svector& u, const infra::svector& v, double& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between two svectors
    @param u A constant reference to a svector
    @param v A constant reference to a svector
    @return The squared l2 distance between the two svectors
*/
double dist2(const infra::svector& u, const infra::svector& v);
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
