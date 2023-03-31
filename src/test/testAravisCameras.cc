/*
 * Author: parenti
 *
 * Created on August 26, 2022, 04:58 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include "AravisCamera.hh"

#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <utility>

#include "karabo/core/DeviceClient.hh"
#include "karabo/core/DeviceServer.hh"
#include "karabo/net/EventLoop.hh"
#include "karabo/util/Hash.hh"
#include "karabo/util/PluginLoader.hh"


#define DEVICE_SERVER_ID    "testDeviceSrvCpp"
#define TEST_ARAVIS_ID      "testAravisCamera"
#define TEST_ARAVIS_ID_FAIL "testAravisCameraFail"
#define TEST_BASLER_ID      "testBaslerCamera"
#define TEST_BASLER2_ID     "testBasle2Camera"
#define TEST_PHSC_ID        "testPhScCamera"
#define LOG_PRIORITY        "FATAL"  // Can also be "DEBUG", "INFO" or "ERROR"

#define DEV_CLI_TIMEOUT_SEC 2


/**
 * @brief Test fixture for the AravisCameras device class.
 */
class AravisCamerasFixture: public testing::Test {
protected:

    AravisCamerasFixture() = default;

    void SetUp( ) {
        m_eventLoopThread = std::thread(&karabo::net::EventLoop::work);

        // Load the library dynamically
        const karabo::util::Hash& pluginConfig = karabo::util::Hash("pluginDirectory", ".");
        karabo::util::PluginLoader::create("PluginLoader", pluginConfig)->update();

        // Instantiate C++ Device Server.
        karabo::util::Hash config("serverId", DEVICE_SERVER_ID,
                                  "scanPlugins", true,
                                  "Logger.priority", LOG_PRIORITY);
        m_deviceSrv = karabo::core::DeviceServer::create("DeviceServer", config);
        m_deviceSrv->finalizeInternalInitialization();
        // Instantiate Device Client.
        m_deviceCli = boost::make_shared<karabo::core::DeviceClient>();
    }

    void TearDown( ) {
        m_deviceCli.reset();
        m_deviceSrv.reset();
        karabo::net::EventLoop::stop();
        m_eventLoopThread.join();
    }

    void instantiateAravisCamera(const karabo::util::Hash& devSpecificCfg) {
        karabo::util::Hash devCfg("deviceId", TEST_ARAVIS_ID, "cameraId", "1.2.3.4");
        devCfg.merge(devSpecificCfg);

        std::pair<bool, std::string> success =
            m_deviceCli->instantiate(DEVICE_SERVER_ID, "AravisCamera",
                                     devCfg, DEV_CLI_TIMEOUT_SEC);

        ASSERT_TRUE(success.first)
            << "Error instantiating '" << TEST_ARAVIS_ID << "':\n"
            << success.second;
    }

    void instantiateAravisCameraFail(const karabo::util::Hash& devSpecificCfg) {
        karabo::util::Hash devCfg("deviceId", TEST_ARAVIS_ID_FAIL); // missing mandatory parameter
        devCfg.merge(devSpecificCfg);

        std::pair<bool, std::string> success =
            m_deviceCli->instantiate(DEVICE_SERVER_ID, "AravisCamera",
                                     devCfg, DEV_CLI_TIMEOUT_SEC);

        ASSERT_FALSE(success.first)
            << "Error instantiating '" << TEST_ARAVIS_ID_FAIL << "':\n"
            << success.second;
    }

    void instantiateBaslerCamera(const karabo::util::Hash& devSpecificCfg) {
        karabo::util::Hash devCfg("deviceId", TEST_BASLER_ID, "cameraId", "1.2.3.4");
        devCfg.merge(devSpecificCfg);

        std::pair<bool, std::string> success =
            m_deviceCli->instantiate(DEVICE_SERVER_ID, "AravisBaslerCamera",
                                     devCfg, DEV_CLI_TIMEOUT_SEC);

        ASSERT_TRUE(success.first)
            << "Error instantiating '" << TEST_BASLER_ID << "':\n"
            << success.second;
    }

    void instantiateBasler2Camera(const karabo::util::Hash& devSpecificCfg) {
        karabo::util::Hash devCfg("deviceId", TEST_BASLER2_ID, "cameraId", "1.2.3.4");
        devCfg.merge(devSpecificCfg);

        std::pair<bool, std::string> success =
            m_deviceCli->instantiate(DEVICE_SERVER_ID, "AravisBasler2Camera",
                                     devCfg, DEV_CLI_TIMEOUT_SEC);

        ASSERT_TRUE(success.first)
            << "Error instantiating '" << TEST_BASLER2_ID << "':\n"
            << success.second;
    }

    void instantiatePhScCamera(const karabo::util::Hash& devSpecificCfg) {
        karabo::util::Hash devCfg("deviceId", TEST_PHSC_ID, "cameraId", "1.2.3.4");
        devCfg.merge(devSpecificCfg);

        std::pair<bool, std::string> success =
            m_deviceCli->instantiate(DEVICE_SERVER_ID, "AravisPhotonicScienceCamera",
                                     devCfg, DEV_CLI_TIMEOUT_SEC);

        ASSERT_TRUE(success.first)
            << "Error instantiating '" << TEST_PHSC_ID << "':\n"
            << success.second;
    }

    void deinstantiateTestDevice() {
        ASSERT_NO_THROW(
            m_deviceCli->killDevice(TEST_ARAVIS_ID, DEV_CLI_TIMEOUT_SEC))
        << "Failed to deinstantiate device '" << TEST_ARAVIS_ID << "'";

        ASSERT_NO_THROW(
            m_deviceCli->killDevice(TEST_BASLER_ID, DEV_CLI_TIMEOUT_SEC))
        << "Failed to deinstantiate device '" << TEST_BASLER_ID << "'";

        ASSERT_NO_THROW(
            m_deviceCli->killDevice(TEST_BASLER2_ID, DEV_CLI_TIMEOUT_SEC))
        << "Failed to deinstantiate device '" << TEST_BASLER2_ID << "'";

        ASSERT_NO_THROW(
            m_deviceCli->killDevice(TEST_PHSC_ID, DEV_CLI_TIMEOUT_SEC))
        << "Failed to deinstantiate device '" << TEST_PHSC_ID << "'";

    }

    std::thread m_eventLoopThread;

    karabo::core::DeviceServer::Pointer m_deviceSrv;
    karabo::core::DeviceClient::Pointer m_deviceCli;
};


// TODO: Give the test case a proper name (not "testScaffold")
TEST_F(AravisCamerasFixture, testScaffold){

    // TODO: Provide a non-empty config for the device under test.
    instantiateAravisCamera(karabo::util::Hash());
    instantiateAravisCameraFail(karabo::util::Hash());
    instantiateBaslerCamera(karabo::util::Hash());
    instantiateBasler2Camera(karabo::util::Hash());
    instantiatePhScCamera(karabo::util::Hash());

    // TODO: Define a test body.

    deinstantiateTestDevice();
}
