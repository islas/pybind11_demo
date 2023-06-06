
#ifndef EmbeddedInterpreter_hpp
#define EmbeddedInterpreter_hpp

#include <vector>
#include <string>
#include <memory>
#include <map>

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"


class EmbeddedInterpreter
{
public:
  // Ctor Dtor
  EmbeddedInterpreter();
  virtual ~EmbeddedInterpreter();

  void initialize();
  void finalize();

  void addToScope( std::string directory );

  // Module handling
  void pymoduleLoad      ( std::string pymodule );
  void pymoduleInitialize( std::string pymodule );
  void pymoduleFinalize  ( std::string pymodule );

  void pymoduleCall      ( std::string pymodule );


  // Building python-accesible modules
  void embedFloatPtr( std::string pymodule, std::string attr, float *ptr, size_t numDims, size_t *pDimSize );

private:

  void createBaseEmbeddedModules( );

  static const std::string PY_INITIALIZE_METHOD;
  static const std::string PY_FINALIZE_METHOD;
  static const std::string PY_MAIN_METHOD;

  std::unique_ptr< pybind11::scoped_interpreter >        upGuard_;            ///< Directly maintain the lifetime of this guard within this scope
  std::vector< std::string >                             userDirectories_;   ///< User supplied locations for user python modules
  std::unordered_map< std::string, pybind11::module_ >   pymodules_;         ///< Map of pymodules loaded ready to be called
  std::unordered_map< std::string, pybind11::module_ >   pymodulesEmbedded_; ///< Map of embedded pymodules available to python

  std::vector< float > demoData_;
  size_t               demoSize_[1];

  // Python modules
  pybind11::module_   sys_;
  pybind11::function  sysPathAppend_;

};

extern "C"
{

void                  EmbeddedInterpreter_ctor      ( EmbeddedInterpreter **ppObj );
void                  EmbeddedInterpreter_dtor      ( EmbeddedInterpreter **ppObj );
void                  EmbeddedInterpreter_initialize( EmbeddedInterpreter **ppObj );
void                  EmbeddedInterpreter_finalize  ( EmbeddedInterpreter **ppObj );
void                  EmbeddedInterpreter_addToScope( EmbeddedInterpreter **ppObj, char *directory );
void                  EmbeddedInterpreter_pymoduleLoad      ( EmbeddedInterpreter **ppObj, char *pymodule );
void                  EmbeddedInterpreter_pymoduleInitialize( EmbeddedInterpreter **ppObj, char *pymodule );
void                  EmbeddedInterpreter_pymoduleFinalize  ( EmbeddedInterpreter **ppObj, char *pymodule );
void                  EmbeddedInterpreter_pymoduleCall      ( EmbeddedInterpreter **ppObj, char *pymodule );
void                  EmbeddedInterpreter_embedFloatPtr     ( EmbeddedInterpreter **ppObj, char *pymodule, char *attr, float *ptr, size_t numDims, size_t *pDimSize );

}

#endif