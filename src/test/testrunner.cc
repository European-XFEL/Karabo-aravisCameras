/*
 * Author: smithm
 *
 * Created on August 22, 2022, 10:36 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "testrunner.hh"


KaraboDeviceFixture::KaraboDeviceFixture() {

    m_eventLoopThread = std::thread(&karabo::net::EventLoop::work);

    // Instantiate C++ Device Client
    m_deviceCli = boost::make_shared<karabo::core::DeviceClient>(std::string(), false);
    m_deviceCli->initialize();
}

void KaraboDeviceFixture::instantiateWithDeviceServer(
        const std::string& classId,
        const std::string& instanceId,
        const karabo::util::Hash& devCfg) {

    // Instantiate C++ device server
    const karabo::util::Hash& pluginConfig = karabo::util::Hash("pluginDirectory", ".");
    karabo::util::PluginLoader::create("PluginLoader", pluginConfig)->update();

    // scanPlugins is set to true to scan $KARABO/plugins directory
    // can be set to false if other libraries are not needed for testing
    karabo::util::Hash config("serverId", DEVICE_SERVER_ID,
                              "scanPlugins", false,
                              "Logger.priority", LOG_PRIORITY);
    m_deviceSrv = karabo::core::DeviceServer::create("DeviceServer", config);
    m_deviceSrv->finalizeInternalInitialization();

    // instantiate the device under test
    std::pair<bool, std::string> success =
    m_deviceCli->instantiate(DEVICE_SERVER_ID, classId,
                             devCfg, DEV_CLI_TIMEOUT_SEC);

    // throw an init exception if cannot instantiate
    if(!success.first) {
        std::string msg = "Failure instantiating '" + instanceId + "':\n" + success.second;
        throw KARABO_INIT_EXCEPTION(msg);
    }
}

karabo::core::BaseDevice::Pointer KaraboDeviceFixture::instantiateAndGetPointer(
        const std::string& classId,
        const std::string& instanceId,
        const karabo::util::Hash& devCfg) {

    std::string errorMsg;
    karabo::core::BaseDevice::Pointer devPtr;

    try {
        // instantiate the device under test
        devPtr = karabo::core::BaseDevice::create(classId, devCfg);
        // build a broker configuration Hash
        using namespace karabo::net;
        using namespace karabo::util;
        const std::string brokerType = Broker::brokerTypeFromEnv();
        Hash valBrokerCfg = Hash(brokerType, Hash(brokerType, Hash("instanceId", instanceId)));
        // connect the device under test to the broker
        Broker::Pointer connection = Broker::createChoice(brokerType, valBrokerCfg);
        devPtr->finalizeInternalInitialization(
            connection,
            true, // no server feeds the device with broadcasts, so it has to listen itself
            "");  // timeserver id (only needed by slotGetTime) does not matter
    } catch (const std::exception& e) {
        errorMsg = e.what();
        if (errorMsg.empty()) errorMsg = "Unexpected std::exception";
    }

    // throw an init exception if cannot instantiate
    if(!errorMsg.empty()) {
        std::string msg = "Failure instantiating '" + instanceId + "':\n" + errorMsg;
        throw KARABO_INIT_EXCEPTION(msg);
    }

    return devPtr;
}

KaraboDeviceFixture::~KaraboDeviceFixture( ) {
    m_deviceCli.reset();
    m_deviceSrv.reset();
    karabo::net::EventLoop::stop();
    m_eventLoopThread.join();
}


/*
 * @brief GoogleTest entry point
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}