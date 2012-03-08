#ifndef LYP_C_functional_h
#define LYP_C_functional_h
/**********************************************************
* LYP_C_functional.h: declarations for LYP_C_functional for KS-DFT
* Robert Parrish, robparrish@gmail.com
* Autogenerated by MATLAB Script on 06-Mar-2012
*
***********************************************************/
#include "functional.h"

namespace psi { namespace functional {

class LYP_C_Functional : public Functional {
public:
    LYP_C_Functional(int npoints, int deriv);
    virtual ~LYP_C_Functional();
    virtual void computeRKSFunctional(boost::shared_ptr<RKSFunctions> prop);
    virtual void computeUKSFunctional(boost::shared_ptr<UKSFunctions> prop);
};
}}
#endif

