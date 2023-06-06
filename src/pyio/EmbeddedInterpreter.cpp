
#include "EmbeddedInterpreter.hpp"

#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <type_traits>

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "pybind11/numpy.h"

const std::string EmbeddedInterpreter::PY_INITIALIZE_METHOD   = "initialize";
const std::string EmbeddedInterpreter::PY_FINALIZE_METHOD     = "finalize";
const std::string EmbeddedInterpreter::PY_MAIN_METHOD         = "main";

////////////////////////////////////////////////////////////////////////////////
/// \brief Ctor
////////////////////////////////////////////////////////////////////////////////
EmbeddedInterpreter::EmbeddedInterpreter()
{
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Dtor
////////////////////////////////////////////////////////////////////////////////
EmbeddedInterpreter::~EmbeddedInterpreter()
{
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Initialize the embedded python interpreter
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::initialize()
{

  // Import sys
  sys_ = pybind11::module_::import( "sys" );
  sysPathAppend_ = sys_.attr( "path" ).attr( "append" );

  // Generate embedded modules
  createBaseEmbeddedModules();

  // Initialize demo data
  demoData_.clear();
  demoData_.assign( { 1, 2, 3, 4, 5 } );
  demoSize_[0] = 5;
  embedFloatPtr( "demo", "demoData", demoData_.data(), 1, demoSize_ );

}

////////////////////////////////////////////////////////////////////////////////
/// \brief Clean up embedded python interpreter
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::finalize()
{
  // Clear containers
  {
    userDirectories_.clear();
    pymodules_.clear();
  }

}


////////////////////////////////////////////////////////////////////////////////
/// \brief Adds module search directories to the python interpreter
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::addToScope( std::string directory )
{
  userDirectories_.push_back( directory );
  sysPathAppend_( directory );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Loads a pymodule into C++ scope to be used by C/C++/Fortran
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::pymoduleLoad( std::string pymodule )
{
  pybind11::module_ loaded = pybind11::module_::import( pymodule.c_str() );

  pymodules_[ pymodule ] = loaded;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Calls a pymodule's initialization function, if any
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::pymoduleInitialize( std::string pymodule )
{
  if ( pybind11::hasattr( pymodules_[ pymodule ], PY_INITIALIZE_METHOD.c_str() ) )
  {
    pymodules_[ pymodule ].attr( PY_INITIALIZE_METHOD.c_str() )();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Calls a pymodule's finalization function, if any
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::pymoduleFinalize( std::string pymodule )
{
  if ( pybind11::hasattr( pymodules_[ pymodule ], PY_FINALIZE_METHOD.c_str() ) )
  {
    pymodules_[ pymodule ].attr( PY_FINALIZE_METHOD.c_str() )();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Calls a pymodule's primary entry function
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::pymoduleCall( std::string pymodule )
{
  if ( pybind11::hasattr( pymodules_[ pymodule ], PY_MAIN_METHOD.c_str() ) )
  {
    pymodules_[ pymodule ].attr( PY_MAIN_METHOD.c_str() )();
  }
}



////////////////////////////////////////////////////////////////////////////////
/// \brief "Creates" imports of the base modules provided
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::createBaseEmbeddedModules( )
{
  // No good way of gathering list of all embdedded modules dynamically
  // so we "just have to know" somehow

  std::list< std::string > modulesToLoad { 
                                            "static_data",
                                            "runtime_data",
                                            "helper",
                                            "demo",
#ifdef SOME_FEATURE
                                            "feature_mod",
#endif
                                          };

  for ( std::string &module : modulesToLoad )
  {
    pymodulesEmbedded_[ module ] = pybind11::module_::import( module.c_str() );
  }
}


////////////////////////////////////////////////////////////////////////////////
/// \brief Builds into a module a float ptr to use
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter::embedFloatPtr( std::string pymodule, std::string attr, float *ptr, size_t numDims, size_t *pDimSize )
{

  // Get embedded module
  pybind11::module_ mod = pymodulesEmbedded_[ std::string( pymodule ) ];

  // We are okay to make copies of these since they should be "small"
  pybind11::array::ShapeContainer dims = pybind11::array::ShapeContainer( std::vector< ssize_t >( pDimSize, pDimSize + numDims ) );
  pybind11::str dummyDataOwner;

  // Add attribute to it
  mod.def(
          attr.c_str(),
          // // Lambda
          [=]() {
                return 
                  pybind11::array_t< std::remove_reference< decltype( ptr[0] ) >::type, pybind11::array::f_style | pybind11::array::forcecast >( 
                    dims,  // buffer dimensions
                    static_cast< const std::remove_reference< decltype( ptr[0] ) >::type * >( ptr ),
                    dummyDataOwner
                    );
          },
          pybind11::return_value_policy::automatic_reference
          );
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///
/// C bindings below
///
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for "ctor"
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_ctor( EmbeddedInterpreter **ppObj )
{
  EmbeddedInterpreter *pInterp = new EmbeddedInterpreter();
#ifndef NDEBUG
  std::cout << __func__ << ": " << "Returning " << static_cast< void * >( pInterp ) << std::endl;
#endif
  
  *ppObj = pInterp;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for "dtor"
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_dtor( EmbeddedInterpreter **ppObj )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  "Deleting " << static_cast< void * >( *ppObj ) << std::endl;
#endif
  delete (*ppObj);
  (*ppObj) = 0;
  ppObj    = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for initialize
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_initialize( EmbeddedInterpreter **ppObj )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->initialize();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for finalize
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_finalize  ( EmbeddedInterpreter **ppObj )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->finalize();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for addToScope
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_addToScope( EmbeddedInterpreter **ppObj, char *directory )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->addToScope( std::string( directory ) );
}


////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for pymoduleLoad
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_pymoduleLoad( EmbeddedInterpreter **ppObj, char *pymodule )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->pymoduleLoad( std::string( pymodule ) );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for pymoduleInitialize
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_pymoduleInitialize( EmbeddedInterpreter **ppObj, char *pymodule )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->pymoduleInitialize( std::string( pymodule ) );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for pymoduleFinalize
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_pymoduleFinalize( EmbeddedInterpreter **ppObj, char *pymodule )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->pymoduleFinalize( std::string( pymodule ) );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for pymoduleCall
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_pymoduleCall( EmbeddedInterpreter **ppObj, char *pymodule )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->pymoduleCall( std::string( pymodule ) );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief C binding for embedFloatPtr
////////////////////////////////////////////////////////////////////////////////
void
EmbeddedInterpreter_embedFloatPtr( EmbeddedInterpreter **ppObj, char *pymodule, char *attr, float *ptr, size_t numDims, size_t *pDimSize )
{
#ifndef NDEBUG
  std::cout << __func__ << ": " <<  static_cast< void * >( *ppObj ) << std::endl;
#endif
  (*ppObj)->embedFloatPtr( std::string( pymodule ), std::string( attr ), ptr, numDims, pDimSize );
}