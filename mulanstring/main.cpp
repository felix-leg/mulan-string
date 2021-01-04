/**
 * The main object file for MuLan String library.
 * This header is meant to be included into a special file inside user's program project.
 * Some parts of MLS can be configured by defining preprocessor macros.
*/

//MLS configuration
#include "mulanstr_config.hpp"

//Backends
#ifdef MULANSTR_USE_GETTEXT

#include "backend/gettext.cpp"

#elif defined(MULANSTR_USE_DUMMY)

#include "backend/dummy.cpp"

#else
#	error No defined backend for MuLan String Library
#endif

//MLS elements
#include "basic_parse.cpp"
#include "language_features.cpp"
#include "template_object.cpp"
#include "interface.cpp"
