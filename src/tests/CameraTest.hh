/*
 * File:   CameraTest.hh
 * Author: parenti
 *
 * Created on Sep 13, 2019, 10:51:04 AM
 */

#ifndef CAMERATEST_HH
#define	CAMERATEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <karabo/karabo.hpp>

class CameraTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(CameraTest);

    CPPUNIT_TEST(shouldCreateCamera);
    CPPUNIT_TEST(shouldNotCreateCamera);

    CPPUNIT_TEST_SUITE_END();

public:
    CameraTest();
    virtual ~CameraTest();
    void setUp();
    void tearDown();

private:
    void shouldCreateCamera();
    void shouldNotCreateCamera();

    karabo::util::Hash m_configOk;
    karabo::util::Hash m_configNotOk;
};

#endif	/* CAMERATEST_HH */

