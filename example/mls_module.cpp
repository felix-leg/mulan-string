/**
 * This is a module being a part of user's program project
 * which installs MuLan String library into the project.
 * The file sets the library to the project's need.
*/

//We are using GNU GetText as our backend
#define MULANSTR_USE_GETTEXT

/**
 * MuLan String uses '%{' and '}%' as delimiters inside its templates.
 * If one wants to change it, uncomment below 2 lines and set them
 * to the right values
*/
//#define MULANSTR_TAG_START "<STARTER_TAG>"
//#define MULANSTR_TAG_END "<END_TAG>"

/**
 * MuLan String uses '{' and '}' as delimiters for specific
 * substitute text inside template's insertions
 * 
 * For example:
 * 'The bag is %{owner!G m={his} f={hers} n={its}}%' 
 * The `G` function produces text based on `owner` gender.
 * For this it needs text to substitute for each gender `owner` may has.
 * This substitutions are given inside "{" and "}" brackets
 * 
 * To change the delimiters, uncomment below 2 lines and set them
 * to the right values
*/
//#define MULANSTR_INNER_TAG_START "<STARTER_TAG>"
//#define MULANSTR_INNER_TAG_END "<END_TAG>"

//include the library file
#include <mulanstring/main.cpp>
