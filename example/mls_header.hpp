/**
 * This is header file which imports MuLan String library's header
 * talored to the project needs
*/

//We want to use GNU GetText as the backend for our project
#define MULANSTR_USE_GETTEXT

//If we would want MLS to not define _() helper functions, then uncomment below line
//#define MULANSTR_DONT_USE_UNDERSCORE

//include the header file
#include <mulanstring/main.hpp>
