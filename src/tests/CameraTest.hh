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

    CPPUNIT_TEST(shouldCreateAravisCamera);
    CPPUNIT_TEST(shouldNotCreateAravisCamera);

    CPPUNIT_TEST(shouldCreateBaslerCamera);

    CPPUNIT_TEST(shouldCreateBasler2Camera);

    CPPUNIT_TEST(shouldCreatePhScienceCamera);

    CPPUNIT_TEST_SUITE_END();

public:
    CameraTest();
    virtual ~CameraTest();
    void setUp();
    void tearDown();

private:
    void shouldCreateAravisCamera();
    void shouldNotCreateAravisCamera();

    void shouldCreateBaslerCamera();

    void shouldCreateBasler2Camera();

    void shouldCreatePhScienceCamera();

    karabo::util::Hash m_configOk;
    karabo::util::Hash m_configNotOk;
};

#endif	/* CAMERATEST_HH */

