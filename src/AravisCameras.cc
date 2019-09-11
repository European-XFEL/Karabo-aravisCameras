/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisCameras.hh"

using namespace std;

USING_KARABO_NAMESPACES;

namespace karabo {


    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, AravisCameras)

    void AravisCameras::expectedParameters(Schema& expected) {
    }


    AravisCameras::AravisCameras(const karabo::util::Hash& config) : Device<>(config) {
    }


    AravisCameras::~AravisCameras() {
    }


    void AravisCameras::preReconfigure(karabo::util::Hash& incomingReconfiguration) {
    }


    void AravisCameras::postReconfigure() {
    }
}
