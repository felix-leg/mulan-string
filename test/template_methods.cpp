#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE template_methods_module
#include <boost/test/unit_test.hpp>

#include <template.h>

BOOST_AUTO_TEST_CASE( testNoVarNoFuncTemplate ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	mls::Template theTemplate{"simple", enLocale};
	
	std::string result = theTemplate.get();
	
	BOOST_TEST_REQUIRE( result == "simple" );
}

BOOST_AUTO_TEST_CASE( testCommentOnlyTemplate ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	mls::Template theTemplate{"sim%{#comment}%ple", enLocale};
	
	std::string result = theTemplate.get();
	
	BOOST_TEST_REQUIRE( result == "simple" );
}

BOOST_AUTO_TEST_CASE( testVarPutTemplate ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	mls::Template theTemplate{"simple %{var1}% %{var2}%", enLocale};
	mls::Template secondTemplate{"template", enLocale};
	
	std::string result = theTemplate.apply("var1", "string").apply("var2", secondTemplate).get();
	
	BOOST_TEST_REQUIRE( result == "simple string template" );
}

BOOST_AUTO_TEST_CASE( testGenderFunction ) {
	auto& plLocale = mls::locale::getLocale("pl_PL");
	// cSpell: disable
	mls::Template mTmpl{"%{+SG=m}%mąż", plLocale};
	mls::Template fTmpl{"%{+SG=f}%żona", plLocale};
	mls::Template nTmpl{"%{+SG=n}%dziecko", plLocale};
	
	mls::Template combTmpl{"dobr%{person!G:y,a,e}% %{person}%", plLocale};
	
	std::string mResult = combTmpl.apply("person", mTmpl).get();
	std::string fResult = combTmpl.apply("person", fTmpl).get();
	std::string nResult = combTmpl.apply("person", nTmpl).get();
	
	BOOST_TEST_REQUIRE( mResult == "dobry mąż" );
	BOOST_TEST_REQUIRE( fResult == "dobra żona" );
	BOOST_TEST_REQUIRE( nResult == "dobre dziecko" );
	// cSpell: enable
}

BOOST_AUTO_TEST_CASE( testCaseFunction ) {
	auto& plLocale = mls::locale::getLocale("pl_PL");
	// cSpell: disable
	mls::Template homeT{"dom%{+C:,u,owi,,em,u,ie}%", plLocale};
	
	mls::Template nomCase{"To jest %{obj!C=nom}%", plLocale};
	mls::Template genCase{"Wejście do %{obj!C=gen}%", plLocale};
	mls::Template datCase{"Daję %{obj!C=dat}%", plLocale};
	mls::Template accCase{"Widzę %{obj!C=acc}%", plLocale};
	mls::Template insCase{"Rozmawiam z %{obj!C=ins}%", plLocale};
	mls::Template locCase{"Opowiadam o %{obj!C=loc}%", plLocale};
	mls::Template vocCase{"Hej %{obj!C=voc}%!", plLocale};
	
	std::string nomStr = nomCase.apply("obj", homeT).get();
	std::string genStr = genCase.apply("obj", homeT).get();
	std::string datStr = datCase.apply("obj", homeT).get();
	std::string accStr = accCase.apply("obj", homeT).get();
	std::string insStr = insCase.apply("obj", homeT).get();
	std::string locStr = locCase.apply("obj", homeT).get();
	std::string vocStr = vocCase.apply("obj", homeT).get();
	
	BOOST_TEST_REQUIRE( nomStr == "To jest dom" );
	BOOST_TEST_REQUIRE( genStr == "Wejście do domu" );
	BOOST_TEST_REQUIRE( datStr == "Daję domowi" );
	BOOST_TEST_REQUIRE( accStr == "Widzę dom" );
	BOOST_TEST_REQUIRE( insStr == "Rozmawiam z domem" );
	BOOST_TEST_REQUIRE( locStr == "Opowiadam o domu" );
	BOOST_TEST_REQUIRE( vocStr == "Hej domie!" );
	// cSpell: enable
}

// cSpell: words pluralizer
BOOST_AUTO_TEST_CASE( testPluralizerNormalNumbers ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	mls::Template nFiles{"file%{num!P:,s}%", enLocale};
	
	std::string oneFile = nFiles.apply("num", 1).get();
	std::string manyFiles = nFiles.apply("num", 5).get();
	
	BOOST_TEST_REQUIRE( oneFile == "file" );
	BOOST_TEST_REQUIRE( manyFiles == "files" );
}

BOOST_AUTO_TEST_CASE( testPluralizerNegativeNumbers ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	mls::Template nFiles{"file%{num!P:,s}%", enLocale};
	
	std::string oneFile = nFiles.apply("num", -1).get();
	std::string manyFiles = nFiles.apply("num", -5).get();
	
	BOOST_TEST_REQUIRE( oneFile == "file" );
	BOOST_TEST_REQUIRE( manyFiles == "files" );
}

BOOST_AUTO_TEST_CASE( testPluralizerRealNumbers ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	mls::Template nFiles{"file%{num!P:,s}%", enLocale};
	
	std::string oneFile = nFiles.apply("num", 1.2).get();
	std::string manyFiles = nFiles.apply("num", 5.4).get();
	
	BOOST_TEST_REQUIRE( oneFile == "file" );
	BOOST_TEST_REQUIRE( manyFiles == "files" );
}

BOOST_AUTO_TEST_CASE( testIntegerFormaterInTemplates ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	mls::Template numTmp{"%{num!I=general}% %{num!I=grouped}%", enLocale};
	
	std::string numStr = numTmp.apply("num", 1'000).get();
	
	BOOST_TEST_REQUIRE( numStr == "1000 1,000" );
}

BOOST_AUTO_TEST_CASE( testRealFormaterInTemplates ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	//
	mls::Template numTmp1{"%{num!R:general}% %{num!R:grouped}%", enLocale};
	
	std::string numStr1 = numTmp1.applyReal("num", 1000.5).get();
	
	BOOST_TEST_REQUIRE( numStr1 == "1000.5 1,000.5" );
	
	//with precision
	mls::Template numTmp2{"%{num!R:general,3}% %{num!R:grouped,3}%", enLocale};
	
	std::string numStr2 = numTmp2.applyReal("num", 1000.1234).get();
	
	BOOST_TEST_REQUIRE( numStr2 == "1000.123 1,000.123" );
}

BOOST_AUTO_TEST_CASE( testRealFormaterInTemplatesLongVersion ) {
	auto& enLocale = mls::locale::getLocale("en_US");
	
	//
	mls::Template numTmp1{"%{num!R format={general}}% %{num!R format={grouped}}%", enLocale};
	
	std::string numStr1 = numTmp1.applyReal("num", 1000.5).get();
	
	BOOST_TEST_REQUIRE( numStr1 == "1000.5 1,000.5" );
	
	//with precision
	mls::Template numTmp2{"%{num!R prec={3}}% %{num!R format={general} prec={3}}% %{num!R format={grouped} prec={3}}%", enLocale};
	
	std::string numStr2 = numTmp2.applyReal("num", 1000.1234).get();
	
	BOOST_TEST_REQUIRE( numStr2 == "1000.123 1000.123 1,000.123" );
}
