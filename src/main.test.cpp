
#pragma target server-test

#include <iostream>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResult.h>

// https://lahosken.san-francisco.ca.us/frivolity/prog/cppunit/

int RunTests(void)
{
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    CppUnit::TextTestRunner runner;
    runner.addTest(suite);

    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

    bool wasSucessful = runner.run();

    return wasSucessful ? 0 : 1;
}

int main(void) {
    return RunTests();
}
