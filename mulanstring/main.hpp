#ifndef MULANSTR_MAIN_HPP
#define MULANSTR_MAIN_HPP

/**
 * The main header file for MuLan String library.
 * This header is meant to be included into every user program's module.
 * Some parts of MLS can be configured by defining preprocessor macros.
*/

//MLS configuration
#include "mulanstr_config.hpp"

//Backends
#ifdef MULANSTR_USE_GETTEXT

#include "backend/gettext.hpp"

#else
#	error No defined backend for MuLan String Library
#endif

//MLS elements
#include "basic_parse.hpp"
#include "language_features.hpp"
#include "template_object.hpp"
#include "interface.hpp"

#ifndef MULANSTR_DONT_USE_UNDERSCORE
#define _(STRING) mls::translate(STRING)
#define _c(CAT,STRING) mls::translate(CAT,STRING)
#define _l(STRING,LOC) mls::translateWithLocale(STRING,LOC)
#define _cl(CAT,STRING,LOC) mls::translateWithLocale(CAT,STRING,LOC)
#endif

#endif /* MULANSTR_MAIN_HPP */
