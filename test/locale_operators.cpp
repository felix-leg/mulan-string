#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE locale_operators_module
#include <boost/test/unit_test.hpp>

#include <mls_locale.h>

BOOST_AUTO_TEST_CASE( testGeneralNumberFormatWithInteger ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	std::string nf_name{"general"};
	
	std::string result = enLocale.getNumberFormat(nf_name)->formatInteger(1'000'000);
	BOOST_TEST_REQUIRE( result == "1000000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(100'000);
	BOOST_TEST_REQUIRE( result == "100000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(100);
	BOOST_TEST_REQUIRE( result == "100" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-1'000'000);
	BOOST_TEST_REQUIRE( result == "-1000000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-100'000);
	BOOST_TEST_REQUIRE( result == "-100000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-100);
	BOOST_TEST_REQUIRE( result == "-100" );
}

BOOST_AUTO_TEST_CASE( testGroupedNumberFormatWithInteger ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	std::string nf_name{"grouped"};
	
	std::string result = enLocale.getNumberFormat(nf_name)->formatInteger(1'000'000);
	BOOST_TEST_REQUIRE( result == "1,000,000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(100'000);
	BOOST_TEST_REQUIRE( result == "100,000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(100);
	BOOST_TEST_REQUIRE( result == "100" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-1'000'000);
	BOOST_TEST_REQUIRE( result == "-1,000,000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-100'000);
	BOOST_TEST_REQUIRE( result == "-100,000" );
	
	result = enLocale.getNumberFormat(nf_name)->formatInteger(-100);
	BOOST_TEST_REQUIRE( result == "-100" );
}

BOOST_AUTO_TEST_CASE( testGeneralNumberFormatWithReal ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	std::string nf_name{"general"};
	
	std::string result = enLocale.getNumberFormat(nf_name)->formatReal(1'000.5);
	BOOST_TEST_REQUIRE( result == "1000.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(0.0);
	BOOST_TEST_REQUIRE( result == "0" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(100.5);
	BOOST_TEST_REQUIRE( result == "100.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(100.2, 1);
	BOOST_TEST_REQUIRE( result == "100.2" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-1'000.5);
	BOOST_TEST_REQUIRE( result == "-1000.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-100.5);
	BOOST_TEST_REQUIRE( result == "-100.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-100.2, 1);
	BOOST_TEST_REQUIRE( result == "-100.2" );
}

BOOST_AUTO_TEST_CASE( testGroupedNumberFormatWithReal ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	std::string nf_name{"grouped"};
	
	std::string result = enLocale.getNumberFormat(nf_name)->formatReal(1'000.5);
	BOOST_TEST_REQUIRE( result == "1,000.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(0.0);
	BOOST_TEST_REQUIRE( result == "0" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(100.5);
	BOOST_TEST_REQUIRE( result == "100.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(100.2, 1);
	BOOST_TEST_REQUIRE( result == "100.2" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-1'000.5);
	BOOST_TEST_REQUIRE( result == "-1,000.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-100.5);
	BOOST_TEST_REQUIRE( result == "-100.5" );
	
	result = enLocale.getNumberFormat(nf_name)->formatReal(-100.2, 1);
	BOOST_TEST_REQUIRE( result == "-100.2" );
}
