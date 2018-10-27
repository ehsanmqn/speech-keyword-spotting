
#ifndef _INFRA_REFCOUNT_DARRAY_H
#define _INFRA_REFCOUNT_DARRAY_H

//*****************************************************************************
/** Implements an array of doubles with reference counting.
    @author Ofer Dekel (oferd@cs.huji.ac.il)
*/
namespace infra{
//----------------------------------------------------------------------------
/** This class holds an array of doubles, and maintains a reference count
    for the datastructure. That is, all copies of the refcount_darray point
    to the same allocated memory - changing one will change them all. The data
    is kept in memory as long as someone is referencing it. It is deallocated
    automatically when the last reference to it dies.
*/ 
class refcount_darray
{
    //=============================================================================
    // refcount_darray Function Declarations
    //=============================================================================
public:
    //----------------------------------------------------------------------------
    /** Constructs a reference counting array of doubles. Allocates new memory.
    @param size The array size
*/
    explicit inline refcount_darray(unsigned long size);

    //----------------------------------------------------------------------------
    /** Destructor. Reduces the reference count by 1. If the count reaches 0, the
    allocated memory is released.
*/
    inline ~refcount_darray();

    //----------------------------------------------------------------------------
    /** Copy constructor - only copies the pointer to the existing memory.
    Increments the reference count by 1. Does not allocate a new array.
    @param r A reference to the other refcount_darray being copied
*/
    inline refcount_darray(const refcount_darray& r);

    //----------------------------------------------------------------------------
    /** Swaps the memory and the counter between two refcount_darrays
    @param r A reference to the other refcount_darray
*/
    inline void swap(refcount_darray& r);

    //----------------------------------------------------------------------------
    /** Returns a c-style pointer to the array without changing the
    reference count
    @return A pointer to the array
*/
    inline double* ptr() const;

    //=============================================================================
    // Private Functions
    //=============================================================================
private:
    //----------------------------------------------------------------------------
    /** Private operator = which does nothing. This declaration makes it
    impossible to call operator =. Instead, create a new refcount_darray
    and call swap().
*/
    inline void operator=(const refcount_darray& r) {}

    //=============================================================================
    // refcount_darray Data Members
    //=============================================================================
    double* _ptr;
    unsigned int* _count;
};
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
