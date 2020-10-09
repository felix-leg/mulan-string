/**
 * Configuration file for MuLanStr library
 */

#pragma once
#ifndef MULANSTR_CONFIG
#define MULANSTR_CONFIG

#include <string>

namespace mls {
	
#ifndef MULANSTR_TAG_START
const std::string TAG_START = std::string("%{");
#else
const std::string TAG_START = std::string( MULANSTR_TAG_START );
#endif

#ifndef MULANSTR_TAG_END
const std::string TAG_END = std::string("}%");
#else
const std::string TAG_END = std::string( MULANSTR_TAG_END );
#endif

#ifndef MULANSTR_INNER_TAG_START
const std::string INNER_TAG_START = std::string("{");
#else
const std::string INNER_TAG_START = std::string(MULANSTR_INNER_TAG_START);
#endif

#ifndef MULANSTR_INNER_TAG_END
const std::string INNER_TAG_END = std::string("}");
#else
const std::string INNER_TAG_END = std::string(MULANSTR_INNER_TAG_END);
#endif
	
};//end namespace mls

#endif //end of file
