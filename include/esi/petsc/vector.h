#ifndef __PETSc_Vector_h__
#define __PETSc_Vector_h__

// The esi::petsc::Vector supports the 
//    esi::Vector<Scalar,Ordinal>
//    esi::Vector<Scalar,Ordinal>ReplaceAccess interfaces

#include "esi/petsc/indexspace.h"

#include "esi/Vector.h"
#include "esi/VectorReplaceAccess.h"

// this contains the PETSc definition of Vector
#include "petscvec.h"

namespace esi{namespace petsc{

/**=========================================================================**/
template<class Scalar,class Ordinal>
  class Vector : public virtual esi::VectorReplaceAccess<Scalar,Ordinal>, public esi::petsc::Object
{
  public:

    // Destructor.
    virtual ~Vector(void);

    // Construct a Vector from a IndexSpace.
    Vector(  esi::IndexSpace<Ordinal> *source);

    // Construct a Vector from a PETSc Vector
    Vector(Vec pvec);

    //  Interface for esi::Object  ---------------

    virtual esi::ErrorCode getInterface(const char* name, void*& iface);
    virtual esi::ErrorCode getInterfacesSupported(esi::Argv * list);

    //  Interface for ESI_Vector  ---------------
    
    virtual esi::ErrorCode clone(esi::Vector<Scalar,Ordinal>*& x);
    virtual esi::ErrorCode getGlobalSize( Ordinal & dim) ;
    virtual esi::ErrorCode getLocalSize( Ordinal & dim) ;
    virtual esi::ErrorCode getIndexSpace(  esi::IndexSpace<Ordinal>*& outmap)  ;
    virtual esi::ErrorCode copy( esi::Vector<Scalar,Ordinal>& x) ;   
    virtual esi::ErrorCode put(  Scalar scalar) ;
    virtual esi::ErrorCode scale(  Scalar scalar) ;
    virtual esi::ErrorCode scaleDiagonal(  esi::Vector<Scalar,Ordinal>& x) ;
    virtual esi::ErrorCode norm1( TYPENAME esi::Vector<Scalar,Ordinal>::magnitude_type& norm)   ;
    virtual esi::ErrorCode norm2( TYPENAME esi::Vector<Scalar,Ordinal>::magnitude_type& norm)   ;
    virtual esi::ErrorCode norm2squared( TYPENAME esi::Vector<Scalar,Ordinal>::magnitude_type& norm)   ;
    virtual esi::ErrorCode normInfinity( TYPENAME esi::Vector<Scalar,Ordinal>::magnitude_type& norm)   ;
    virtual esi::ErrorCode dot( esi::Vector<Scalar,Ordinal>& x, Scalar& product)   ;
    virtual esi::ErrorCode axpy( esi::Vector<Scalar,Ordinal>& x, Scalar scalar) ;
    virtual esi::ErrorCode aypx(Scalar scalar, esi::Vector<Scalar,Ordinal>& x) ;

    virtual esi::ErrorCode minAbsCoef(TYPENAME esi::Vector<Scalar,Ordinal>::magnitude_type&)  {return 1;}
    virtual esi::ErrorCode axpby(Scalar,esi::Vector<Scalar,Ordinal>&,Scalar,esi::Vector<Scalar,Ordinal>&);
    virtual esi::ErrorCode getCoefPtrReadLock(Scalar *&) ;
    virtual esi::ErrorCode getCoefPtrReadWriteLock(Scalar *&);
    virtual esi::ErrorCode releaseCoefPtrLock(Scalar *&) ;

    // Interface for ESI_VectorReplaceAccess
   
    virtual esi::ErrorCode setArrayPointer(Scalar* array, Ordinal length);

    class Factory : public virtual ::esi::Vector<Scalar,Ordinal>::Factory
    {
      public:

      // Destructor.
      virtual ~Factory(void){};

      // Construct a Vector
      virtual esi::ErrorCode create(esi::IndexSpace<Ordinal>&,esi::Vector<Scalar,Ordinal>*&v);
    };

  private:
    Vec                      vec;
    esi::IndexSpace<Ordinal> *map;
};

/**=========================================================================**/
template<>
  class Vector<double,int>: public virtual esi::VectorReplaceAccess<double,int>, public esi::petsc::Object
{
  public:

    // Destructor.
    virtual ~Vector(void);

    // Construct a Vector from a IndexSpace.
    Vector(  esi::IndexSpace<int> *source);

    // Construct a Vector from a PETSc Vector
    Vector(Vec pvec);

    //  Interface for esi::Object  ---------------

    virtual esi::ErrorCode getInterface(const char* name, void*& iface);
    virtual esi::ErrorCode getInterfacesSupported(esi::Argv * list);


    //  Interface for ESI_Vector  ---------------
    
    virtual esi::ErrorCode clone(esi::Vector<double,int>*& x);
    virtual esi::ErrorCode getGlobalSize( int & dim) ;
    virtual esi::ErrorCode getLocalSize( int & dim) ;
    virtual esi::ErrorCode getIndexSpace(  esi::IndexSpace<int>*& outmap)  ;
    virtual esi::ErrorCode copy( esi::Vector<double,int>& x) ;   
    virtual esi::ErrorCode put(  double scalar) ;
    virtual esi::ErrorCode scale(  double scalar) ;
    virtual esi::ErrorCode scaleDiagonal(  esi::Vector<double,int>& x) ;
    virtual esi::ErrorCode norm1( magnitude_type& norm)   ;
    virtual esi::ErrorCode norm2( magnitude_type& norm)   ;
    virtual esi::ErrorCode norm2squared( magnitude_type& norm)   ;
    virtual esi::ErrorCode normInfinity( magnitude_type& norm)   ;
    virtual esi::ErrorCode dot( esi::Vector<double,int>& x, double& product)   ;
    virtual esi::ErrorCode axpy( esi::Vector<double,int>& x, double scalar) ;
    virtual esi::ErrorCode aypx(double scalar, esi::Vector<double,int>& x) ;

    virtual esi::ErrorCode minAbsCoef(magnitude_type&)  {return 1;}
    virtual esi::ErrorCode axpby(double,esi::Vector<double,int>&,double,esi::Vector<double,int>&);
    virtual esi::ErrorCode getCoefPtrReadLock(double *&) ;
    virtual esi::ErrorCode getCoefPtrReadWriteLock(double *&);
    virtual esi::ErrorCode releaseCoefPtrLock(double *&) ;

    // Interface for ESI_VectorReplaceAccess
   
    virtual esi::ErrorCode setArrayPointer(double* array, int length);

    class Factory : public virtual ::esi::Vector<double,int>::Factory
    {
      public:

      // Destructor.
      virtual ~Factory(void){};

      // Construct a Vector
      virtual esi::ErrorCode create(esi::IndexSpace<int>&,esi::Vector<double,int>*&v);
    };

  private:
    Vec                    vec;
    ::esi::IndexSpace<int> *map;
};

}}
EXTERN PetscErrorCode VecESIWrap(Vec,esi::Vector<double,int>**);

#endif




