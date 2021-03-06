
#ifndef _OFERD_VV_FUNCS_H_
#define _OFERD_VV_FUNCS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_vector.h"

namespace infra{
//-----------------------------------------------------------------------------
/** Performs vector_base-vector_base multiplication: outcome = u * v 
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::vector_base& u, const infra::vector_base& v, double& 
          outcome);

//-----------------------------------------------------------------------------
/** Operator * for vector-vector multiplication
    @param u A constant reference to a vector
    @param v A constant reference to a vector
    @return The product
*/
double operator* (const infra::vector_base& u, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs vector_base-vector_base multiplication and adds the outcome to
    what is currently stored in outcome: outcome += u * v 
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::vector_base& u, const infra::vector_base& v, double&
              outcome);

//-----------------------------------------------------------------------------
/** Performs vector_base addition
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to where the sum vector_base will be stored
*/
void sum(const infra::vector_base& u, const infra::vector_base& v,
	 infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Operator + for vector-vector addition
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @return The sum vector
*/
infra::vector_base operator+(const infra::vector_base& u, 
                             const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs vector_base subtraction
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to where the difference vector_base will be 
    stored
*/
void diff(const infra::vector_base& u, const infra::vector_base& v,
	  infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Operator + for vector-vector subtraction
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @return The difference vector
*/
infra::vector_base operator-(const infra::vector_base& u, 
                             const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Coordinate-wise multiplication
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to the outcome vector_base
*/
void coordwise_mult(const infra::vector_base& u, const infra::vector_base& v,
		    infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Coordinate-wise division
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to the outcome vector_base
*/
void coordwise_div(const infra::vector_base& u, const infra::vector_base& v,
		   infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between two vector_bases
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void dist2(const infra::vector_base& u, const infra::vector_base& v, double& 
           outcome);

//-----------------------------------------------------------------------------
/** Calculates the squared l2 distance between two vector_bases
    @param u A constant reference to a vector_base
    @param v A constant reference to a vector_base
    @return The squared l2 distance between the two vector_bases
*/
double dist2(const infra::vector_base& u, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Operator + for vector-scalar coordinate-wise addition. 
    @param u A constant reference to a vector
    @param s A scalar
    @return The outcome
*/
infra::vector_base operator+(const infra::vector_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator + for scalar-vector coordinate-wise addition. 
    @param s A scalar
    @param u A constant reference to a vector
    @return The outcome
*/
infra::vector_base operator+(const double& s,
                             const infra::vector_base& u);

//-----------------------------------------------------------------------------
/** Operator - for vector-scalar coordinate-wise subtraction;
    @param u A constant reference to a vector
    @param s A scalar
    @return The outcome
*/
infra::vector_base operator-(const infra::vector_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator - for scalar-vector coordinate-wise subtraction;
    @param s A scalar
    @param u A constant reference to a vector
    @return The outcome
*/
infra::vector_base operator-(const double& s,
                             const infra::vector_base& u);

//-----------------------------------------------------------------------------
/** Operator * for vector-scalar coordinate-wise multiplication.
    @param u A constant reference to a vector
    @param s A scalar
    @return The outcome
*/
infra::vector_base operator*(const infra::vector_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator * for scalar-vector coordinate-wise multiplication.
    @param s A scalar
    @param u A constant reference to a vector
    @return The outcome
*/
infra::vector_base operator*(const double& s,
                             const infra::vector_base& u);

//-----------------------------------------------------------------------------
/** Operator / for vector-scalar coordinate-wise division.
    @param u A constant reference to a vector
    @param s A scalar
    @return The outcome
*/
infra::vector_base operator/(const infra::vector_base& u, 
                             const double& s);


//-----------------------------------------------------------------------------
/** Operator / for scalar-vector coordinate-wise division.
    @param s A scalar
    @param u A constant reference to a vector
    @return The outcome
*/
infra::vector_base operator/(const double& s,
                             const infra::vector_base& u);
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
