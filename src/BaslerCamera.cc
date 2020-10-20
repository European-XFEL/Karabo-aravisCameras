/*
 * Author: <parenti>
 *
 * Created on October 20, 2020, 16:34 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "BaslerCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile in Karabo 2.8.1 - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, BaslerCamera)
    // XXX Work-around: do not register the base class parameters here,
    //     do it in BaslerCamera::expectedParameters
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, BaslerCamera)

    void BaslerCamera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisCamera::expectedParameters(expected);

        // Description of not available properties
        const std::string notAvailable("Not available for this camera.");

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS                                             *
        // **************************************************************************************************************


        // **************************************************************************************************************
        //                                   READ ONLY HARDWARE PARAMETERS                                              *
        // **************************************************************************************************************


    }

    BaslerCamera::BaslerCamera(const karabo::util::Hash& config) : AravisCamera(config) {
    }

    BaslerCamera::~BaslerCamera() {
    }

} // namespace karabo
