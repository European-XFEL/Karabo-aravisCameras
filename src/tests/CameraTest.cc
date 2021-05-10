/*
 * File:   CameraTest.cc
 * Author: parenti
 *
 * Created on Sep 13, 2019, 10:51:04 AM
 */

#include "CameraTest.hh"
#include <karabo/karabo.hpp>
#include "../AravisCamera.hh"

using namespace std;

USING_KARABO_NAMESPACES;

CPPUNIT_TEST_SUITE_REGISTRATION(CameraTest);

CameraTest::CameraTest() {
    m_configOk.set("deviceId", "testdevice");
    m_configOk.set("cameraId", "1.2.3.4");

    m_configNotOk.set("deviceId", "testdevice");
}

CameraTest::~CameraTest() {
}

void CameraTest::setUp() {
}

void CameraTest::tearDown() {
}

void CameraTest::shouldCreateAravisCamera() {
    BaseDevice::Pointer device = BaseDevice::create("AravisCamera", m_configOk);
    CPPUNIT_ASSERT_EQUAL(string("AravisCamera"), (device->getClassInfo()).getClassName());
}

void CameraTest::shouldNotCreateAravisCamera() {
    CPPUNIT_ASSERT_THROW(BaseDevice::create("AravisCamera", m_configNotOk), karabo::util::ParameterException);
}

void CameraTest::shouldCreatePhScienceCamera() {
    BaseDevice::Pointer device = BaseDevice::create("AravisPhotonicScienceCamera", m_configOk);
    CPPUNIT_ASSERT_EQUAL(string("AravisPhotonicScienceCamera"), (device->getClassInfo()).getClassName());
}

void CameraTest::shouldNotCreatePhScienceCamera() {
    CPPUNIT_ASSERT_THROW(BaseDevice::create("AravisPhotonicScienceCamera", m_configNotOk), karabo::util::ParameterException);
}
