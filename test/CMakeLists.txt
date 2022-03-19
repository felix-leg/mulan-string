cmake_minimum_required(VERSION 3.10.3)
#use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(MuLanStringTests VERSION 1.0)

if(MSVC)
	if(VCPKG_HOME)
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_HOME}/installed/x64-windows/share")
		include( "${VCPKG_HOME}/scripts/buildsystems/vcpkg.cmake" )
	else()
		message(WARNING "Under MSVC you may need VCPKG. If you do so, run CMake with -DVCPKG_HOME=<your vcpkg home directory>")
	endif()
endif()

# Initalize tests
enable_testing()
find_package(Boost 1.56 COMPONENTS unit_test_framework REQUIRED)
#generate Boost tests
set(MULAN_STRING_SRC "${CMAKE_SOURCE_DIR}/../src")
function(make_test TEST_NAME TESTED_FILES)
	list(TRANSFORM TESTED_FILES PREPEND "${MULAN_STRING_SRC}/")
	list(APPEND TESTED_FILES "${TEST_NAME}.cpp")
	
	add_executable(${TEST_NAME} ${TESTED_FILES})
	
	target_include_directories(${TEST_NAME} PRIVATE "${MULAN_STRING_SRC}")
	target_include_directories(${TEST_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
	target_link_libraries(${TEST_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
	
	add_test(NAME ${TEST_NAME}_test COMMAND ${TEST_NAME})
endfunction()

#---------- List of tests
make_test(template_preparser "preparser.h;preparser.cpp;errors.h;errors.cpp")
make_test(locale_operators "mls_locale.h;mls_locale.cpp")
make_test(template_methods "preparser.h;preparser.cpp;errors.h;errors.cpp;mls_locale.h;mls_locale.cpp;template.h;template.cpp")

#------- GetText support
include(FindIntl)
find_package(Intl)
include(FindGettext)
find_package(Gettext)

if(Intl_FOUND AND GETTEXT_FOUND)
	message("Intl and GetText found")
	make_test(gettext_backend "gettext_backend.h;gettext_backend.cpp;preparser.h;preparser.cpp;errors.h;errors.cpp;mls_locale.h;mls_locale.cpp;template.h;template.cpp")
	target_compile_definitions(gettext_backend PRIVATE "LOCALES_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/locale\"")
	#Intl
	target_include_directories(gettext_backend PUBLIC "${Intl_INCLUDE_DIRS}")
	if(MSVC)
		list(APPEND Intl_LIBRARIES "${VCPKG_HOME}installed/x64-windows/lib/intl.lib;${VCPKG_HOME}installed/x64-windows/lib/iconv.lib") #MSVC seems to forget about these libraries 
	endif()
	target_link_libraries(gettext_backend "${Intl_LIBRARIES}")
	#GetText
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/gettext_test.po"
		COMMAND "${GETTEXT_MSGMERGE_EXECUTABLE}" -U "--lang=pl_PL" "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/gettext_test.po" "${CMAKE_CURRENT_SOURCE_DIR}/po/gettext_test.pot"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/gettext_test.pot"
	)
	#	generates `mofiles` make command
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/gettext_test.mo"
		COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" -o "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/gettext_test.mo" "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/gettext_test.po"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/pl_PL/gettext_test.po"
	)
	add_custom_target( mofiles DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/locale/pl_PL/LC_MESSAGES/gettext_test.mo" )
	#	extract translatable strings
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/po/gettext_test.pot"
		COMMAND xgettext "--keyword=_" "--language=C++" "--package-name=gettext_test" "--package-version=1.0" "--from-code=UTF-8" -o "${CMAKE_CURRENT_SOURCE_DIR}/po/gettext_test.pot" "${CMAKE_CURRENT_SOURCE_DIR}/gettext_backend.cpp"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gettext_backend.cpp"
	)
	add_custom_target( potfiles DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/gettext_test.pot")
endif()

