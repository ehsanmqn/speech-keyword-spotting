
#ifndef _OFERD_SM_FUNCS_H_
#define _OFERD_SM_FUNCS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_matrix.h"
#include "infra_vector.h"
#include "infra_svector.h"

namespace infra {
//-----------------------------------------------------------------------------
/** Performs matrix-svector multiplication and stores the outcome in
    a vector (not a svector): v = M * s 
    @param M A constant reference to a matrix
    @param s A constant reference to a svector
    @param v A reference to the vector where the outcome will be stored
*/
void prod(const infra::matrix_base& M, const infra::svector& s,
	  infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs matrix-svector multiplication and returns the outcome in
    a vector (not a svector): v = M * s 
    @param M A constant reference to a matrix
    @param s A constant reference to a svector
    @return The outcome in a vector
*/
infra::vector_base operator*(const infra::matrix_base& M, 
                             const infra::svector& s);

//-----------------------------------------------------------------------------
/** Performs svector-matrix multiplication and stores the outcome in
    a vector (not a svector): v = s * M
    @param s A constant reference to a svector
    @param M A constant reference to a matrix
    @param v A reference to the vector where the outcome will be stored
*/
void prod(const infra::svector& s, const infra::matrix_base& M, 
	  infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs svector-matrix multiplication and returns the outcome in
    a vector (not a svector): v = s * M
    @param s A constant reference to a svector
    @param M A constant reference to a matrix
    @return The outcome in a vector
*/
infra::vector_base operator*(const infra::svector& s,
                             const infra::matrix_base& M);

//-----------------------------------------------------------------------------
/** Adds a matrix-svector multiplication to a vector (not a svector):
    v += M * s 
    @param M A constant reference to a matrix
    @param s A constant reference to a svector
    @param v A reference to the vector where the outcome will be stored
*/
void add_prod(const infra::matrix_base& M, const infra::svector& s,
	      infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Adds a svector-matrix multiplication to a vector (not a svector):
    v += s * M
    @param s A constant reference to a svector
    @param M A constant reference to a matrix
    @param v A reference to the vector where the outcome will be stored
*/
void add_prod(const infra::svector& s, const infra::matrix_base& M, 
	      infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Subtracts a matrix-svector multiplication from a vector (not a svector):
    v -= M * s 
    @param M A constant reference to a matrix
    @param s A constant reference to a svector
    @param v A reference to the vector where the outcome will be stored
*/
void subtract_prod(const infra::matrix_base& M, const infra::svector& s,
	      infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Subtracts a svector-matrix multiplication from a vector (not a svector):
    v -= s * M
    @param s A constant reference to a svector
    @param M A constant reference to a matrix
    @param v A reference to the vector where the outcome will be stored
*/
void subtract_prod(const infra::svector& s, const infra::matrix_base& M, 
		   infra::vector_base& v);

};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
