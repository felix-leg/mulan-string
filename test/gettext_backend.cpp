#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE gettext_backend_module
#include <boost/test/unit_test.hpp>

#include <gettext_backend.h>
#include <template.h>
// cSpell:disable

BOOST_AUTO_TEST_CASE( testTranslation ) {
	mls::backend::init("gettext_test", "pl_PL.UTF-8", LOCALES_DIR); 
	
	auto toTranslate = _("To translate");
	auto translated = toTranslate.get();
	
	BOOST_TEST_REQUIRE( translated == "Do przetłumaczenia" );
}
