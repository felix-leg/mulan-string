cmake_minimum_required(VERSION 3.10.3)
#use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(MuLanStringGettextExample VERSION 1.0)

if(MSVC)
	if(VCPKG_HOME)
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_HOME}/installed/x64-windows/share")
		include( "${VCPKG_HOME}/scripts/buildsystems/vcpkg.cmake" )
	else()
		message(WARNING "Under MSVC you may need VCPKG. If you do so, run CMake with -DVCPKG_HOME=<your vcpkg home directory>")
	endif()
endif()

#------- GetText support
include(FindIntl)
find_package(Intl REQUIRED)
include(FindGettext)
find_package(Gettext REQUIRED)

#------ The example
add_executable(example main.cpp)
#target_compile_definitions(gettext_backend PRIVATE "LOCALES_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/locale\"")
#Intl
target_include_directories(example PUBLIC "${Intl_INCLUDE_DIRS}" "${CMAKE_CURRENT_SOURCE_DIR}/../../mulanstring")
if(MSVC)
	list(APPEND Intl_LIBRARIES "${VCPKG_HOME}installed/x64-windows/lib/intl.lib;${VCPKG_HOME}installed/x64-windows/lib/iconv.lib") #MSVC seems to forget about these libraries 
endif()
target_link_libraries(example "${Intl_LIBRARIES}")
#GetText
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/example.po"
	COMMAND "${GETTEXT_MSGMERGE_EXECUTABLE}" -U "--lang=pl_PL" "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/example.po" "${CMAKE_CURRENT_SOURCE_DIR}/po/example.pot"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/example.pot"
)
#	generates `mofiles` make command
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/example.mo"
	COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" -o "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/example.mo" "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/example.po"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/example.po"
)
add_custom_target( mofiles DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/example.mo" )
#	extract translatable strings
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/po/example.pot"
	COMMAND xgettext "--keyword=_" "--language=C++" "--package-name=gettext_test" "--package-version=1.0" "--from-code=UTF-8" -o "${CMAKE_CURRENT_SOURCE_DIR}/po/example.pot" "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
add_custom_target( potfiles DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/example.pot")


