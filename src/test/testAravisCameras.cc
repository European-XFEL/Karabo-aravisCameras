/*
 * Author: smithm
 *
 * Created on March 24, 2023, 04:58 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisCamera.hh"

#include <thread>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>

#include "karabo/util/Hash.hh"

#include "testrunner.hh"

#define TEST_DEVICE_ID      "testAravisCamera"

using namespace ::testing;

/**
 * @brief Test fixture for the AravisCamera device classes.
 *        Since there are multiple classes to instantiate,
 *        this test fixture is parameterized by the classIds.
 */
class DefaultCfg: public KaraboDeviceFixture, public WithParamInterface<std::string> {
protected:

    DefaultCfg() = default;

    void SetUp( ) override {
        /**
         * Add configuration for this 'DefaultCfg' test fixture
         * to the devCfg hash here
         */

        karabo::util::Hash goodCfg("deviceId", TEST_DEVICE_ID,
                                   "_deviceId_", TEST_DEVICE_ID,
                                   "cameraId", "1.2.3.4");
        devCfg.merge(goodCfg);

        /**
         * Instantiate device inside a device server
         *
         * Recommended method if not using googletest/googlemock expectations
         */
        // instantiate the device to be tested
        //std::string classId = GetParam();
        //instantiateWithDeviceServer(classId, TEST_DEVICE_ID, devCfg);
    }

    void TearDown( ) override {
        /**
         * Shutdown the device
         */
        // test the preDestruction() hook
        m_deviceCli->execute(TEST_DEVICE_ID, "slotKillDevice");
    }

    karabo::util::Hash devCfg;
};

class InvalidCfg: public KaraboDeviceFixture {
protected:

    InvalidCfg() = default;

    void SetUp( ) override {
        /**
         * Add configuration for this 'InvalidCfg' test fixture
         * that should make instantiation fail
         */

        karabo::util::Hash badCfg("deviceId", TEST_DEVICE_ID,
                                  "_deviceId_", TEST_DEVICE_ID);
        devCfg.merge(badCfg);

        /**
         * Instantiate device inside a device server
         *
         * Recommended method if not using googletest/googlemock expectations
         */
        // instantiate the device to be tested
        //instantiateWithDeviceServer(classId, TEST_DEVICE_ID, devCfg);
    }

    void TearDown( ) override {
        /**
         * Do nothing since the device should not instantiate
         */
    }

    karabo::util::Hash devCfg;
};


// test only that device instantiates
TEST_P(DefaultCfg, testDeviceInstantiation) {
    // get current parameter (note the use of TEST_P(...) above)
    std::string clsIdFromParam = GetParam();
    auto bp = instantiateAndGetPointer(clsIdFromParam, TEST_DEVICE_ID, devCfg);

    // this pointer is needed only to convince the linker to link
    // libaravisCameras.so directly to this test executable
    // TODO: find a better way with linker flags directly
    auto dp = boost::dynamic_pointer_cast<karabo::AravisCamera>(bp);

    karabo::util::Hash result = m_deviceCli->get(TEST_DEVICE_ID);
    std::string cls = result.get<std::string>("classId");
    std::string clsVer = result.get<std::string>("classVersion");

    std::cout << std::endl;
    std::cout << "Device under test is class " << cls;
    std::cout << ", version " << clsVer;
    std::cout << std::endl;
    std::cout << std::endl;

    ASSERT_STREQ(cls.c_str(), clsIdFromParam.c_str());
}

// run the above DefaultCfg test, with the default config hash,
// for each camera class listed in Values(...)
INSTANTIATE_TEST_CASE_P(
    AravisCameras,
    DefaultCfg,
    Values("AravisCamera",
           "AravisBaslerCamera",
           "AravisBasler2Camera",
           "AravisPhotonicScienceCamera")
);


// test only that device fails to instantiate
TEST_F(InvalidCfg, testDeviceInstantiationFailure) {

    EXPECT_ANY_THROW({ instantiateAndGetPointer("AravisCamera", TEST_DEVICE_ID, devCfg); });

}
