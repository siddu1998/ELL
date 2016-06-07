////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     EMLL.i (interfaces)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

%module "EMLL"

// Common stuff
#ifdef SWIGPYTHON
#pragma SWIG nowarn=325,341,362,401,503
#endif

// Useful defines
%{
#ifdef SWIGPYTHON
  #define SWIG_FILE_WITH_INIT
  #define SWIG_PYTHON_EXTRA_NATIVE_CONTAINERS 
#endif
%}

// Include common STL libraries
%{
#include <cstdint>
#include <sstream>
#include <stdexcept>
%}

#ifndef SWIGXML
%include typemaps.i
%include "std_string.i"
%include "std_vector.i"
%include "std_shared_ptr.i"
#endif

%include "exception.i" 
%include "unique_ptr.i"
#ifdef SWIGPYTHON
%include "std_iostream.i"  // Sadly, there is no std_iostream.i for C#
#endif

// Make trivial definitions for some things in std:: that SWIG chokes on, so we remember to call their constructors
namespace std 
{
    class default_random_engine {};
}

// Redefine uint64_t to long so we can use it in python
%typemap(in) uint64_t = unsigned long;
%apply unsigned long {uint64_t}

%typemap(in) int64_t = long;
%apply long {int64_t}

#ifndef SWIGXML
%template () std::vector<double>;
%template () std::vector<float>;
#endif

// Add some primitive exception handling
%exception {
    try { 
        $action 
    }
    catch(std::runtime_error err) {
        SWIG_exception(SWIG_RuntimeError, const_cast<char*>(err.what()));        
    }    
    catch (...) {
        SWIG_exception(SWIG_RuntimeError, "Exception in EMLL library");
    }
}

// Macro for exposing operator[] to python
%define WRAP_OP_AT(Class, ValueType)
  %extend Class 
  {
    ValueType __getitem__(size_t index)
    {
      return (*$self)[index];
    }
  };
%enddef

// Macro for turning Print(ostream) into __str__ 
%define WRAP_PRINT_TO_STR(Class)
    %extend Class
    {
        std::string __str__() 
        {        
            std::ostringstream oss(std::ostringstream::out);
            ($self)->Print(oss);
            return oss.str();
        }
    };
%enddef

// Define some namespaces so we can refer to them later
namespace lossFunctions {};
namespace predictors {};
namespace dataset {};

%ignore dataset::RowDataset::operator[];
%import "RowDataset.h"
%import "IDataVector.h"

#ifndef SWIGXML
%template () std::vector<dataset::IDataVector>;
#endif

namespace dataset
{
    class GenericRowIterator {}; // This is necessary to prevent memory leak of datasets::GenericRowIterator
}

typedef dataset::RowDataset<IDataVector> dataset::GenericRowDataset;
typedef dataset::GenericRowDataset::Iterator dataset::GenericRowIterator;

namespace utilities
{
    template <typename ValueType>
    class IIterator {}; 
    
    template <typename IteratorType, typename ValueType> class StlIterator {};
    %template () StlIterator<typename std::vector<dataset::IDataVector>::const_iterator, dataset::IDataVector>;

    template <typename IteratorType, typename ValueType> class StlIndexValueIterator {};
    %template () StlIndexValueIterator<typename std::vector<dataset::IDataVector>::const_iterator, dataset::IDataVector>;
}

%template () utilities::StlIterator<typename std::vector<dataset::SupervisedExample<dataset::IDataVector>,std::allocator<dataset::SupervisedExample<dataset::IDataVector>>>::const_iterator, dataset::SupervisedExample<dataset::IDataVector>>;
typedef utilities::StlIterator<typename std::vector<dataset::SupervisedExample<dataset::IDataVector>>::const_iterator, dataset::SupervisedExample<dataset::IDataVector>> dataset::GenericRowIterator;

//%import predictors.i

// Interface includes for lossFunctions library
%include lossFunctions.i

// Interface includes for linear library
%include linear.i

// Interface includes for layers library
%include layers.i

// Interface includes for dataset library
%include dataset.i

%include "SGDIncrementalTrainer_wrap.h"

// Interface for the predictors library
%include predictors.i

// Interface includes for utilities library
%include utilities.i

// Interface includes for trainers library
%include trainers.i

// Interface for common library
%include common.i

wrap_unique_ptr(LayerPtr, layers::Layer)

#ifndef SWIGXML
%template () std::vector<dataset::SupervisedExample<dataset::IDataVector>>;
%template () utilities::StlIterator<typename std::vector<dataset::SupervisedExample<dataset::IDataVector>>::const_iterator, dataset::SupervisedExample<dataset::IDataVector>>;
%template () utilities::StlIterator<typename std::vector<dataset::SupervisedExample<dataset::IDataVector>, std::allocator<dataset::SupervisedExample<dataset::IDataVector>>>::const_iterator, dataset::SupervisedExample<dataset::IDataVector>>;

%template () dataset::RowDataset<dataset::IDataVector>;
%template () trainers::SGDIncrementalTrainer<lossFunctions::SquaredLoss>;
#endif

typedef trainers::SGDIncrementalTrainer<lossFunctions::SquaredLoss>::PredictorType predictors::LinearPredictor;
class trainers::SGDIncrementalTrainer<lossFunctions::SquaredLoss>::PredictorType {};

// Interface for features library
%include features.i

#ifndef SWIGXML
%shared_ptr(layers::Map)
%shared_ptr(layers::Model)
//%template (GenericRowDataset) dataset::RowDataset<dataset::IDataVector>
//%shared_ptr(GenericRowDataset)
%shared_ptr(RowDataset)
#endif
