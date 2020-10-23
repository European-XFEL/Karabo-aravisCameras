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

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS                                             *
        // **************************************************************************************************************

        INT32_ELEMENT(expected).key("gevSCBWR")
                .alias("GevSCBWR")
                .tags("genicam")
                .displayedName("GevSCBWR")
                .description("This value reserves a portion of Ethernet bandwidth assigned to the camera for "
                "packet resends and for the transmission of control data between the camera and the host PC. "
                "The setting is expressed as a percentage of the bandwidth assigned parameter. "
                "For example, if the Bandwidth Assigned parameter indicates that 30 MBytes/s have been assigned "
                "to the camera and the Bandwidth Reserve parameter is set to 5%, then the bandwidth reserve "
                "will be 1.5 MBytes/s.")
                .assignmentOptional().noDefaultValue()
                .unit(Unit::PERCENT)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("gevSCBWRA")
                .alias("GevSCBWRA")
                .tags("genicam")
                .displayedName("GevSCBWRA")
                .description("This value sets a multiplier for the Bandwidth Reserve parameter. "
                "The multiplier is used to establish an extra pool of reserved bandwidth that can be used "
                "if an unusually large burst of packet resends is needed.")
                .assignmentOptional().noDefaultValue()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        // **************************************************************************************************************
        //                                   READ ONLY HARDWARE PARAMETERS                                              *
        // **************************************************************************************************************

        FLOAT_ELEMENT(expected).key("resultingLinePeriodAbs")
                .alias("ResultingLinePeriodAbs")
                .tags("poll")
                .displayedName("Resulting Line Period (Abs)")
                .description("Indicates the 'absolute' value of the minimum allowed acquisition line period. "
                "The 'absolute' value is a float value that indicates the minimum allowed acquisition line "
                "period in microseconds given the current settings for the area of interest, exposure time, "
                "and bandwidth.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MICRO)
                .readOnly()
                .commit();

        FLOAT_ELEMENT(expected).key("resultingLineRateAbs")
                .alias("ResultingLineRateAbs")
                .tags("poll")
                .displayedName("Resulting Line Rate (Abs)")
                .description("Indicates the 'absolute' value of the maximum allowed acquisition line rate. "
                "The 'absolute' value is a float value that indicates the maximum allowed acquisition line "
                "rate in lines per second given the current settings for the area of interest, exposure time, "
                "and bandwidth.")
                .unit(Unit::HERTZ)
                .readOnly()
                .commit();

        FLOAT_ELEMENT(expected).key("resultingFramePeriodAbs")
                .alias("ResultingFramePeriodAbs")
                .tags("poll")
                .displayedName("Resulting Frame Period (Abs)")
                .description("Indicates the 'absolute' value of the minimum allowed acquisition frame period. "
                "The 'absolute' value is a float value that indicates the minimum allowed acquisition frame "
                "period in microseconds given the current settings for the area of interest, exposure time, "
                "and bandwidth.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MICRO)
                .readOnly()
                .commit();

        FLOAT_ELEMENT(expected).key("ResultingFrameRateAbs")
                .alias("ResultingFrameRateAbs")
                .tags("poll")
                .displayedName("Resulting Frame Rate (Abs)")
                .description("Indicates the 'absolute' value of the maximum allowed acquisition frame rate. "
                "The 'absolute' value is a float value that indicates the maximum allowed acquisition frame "
                "rate in frames per second given the current settings for the area of interest, exposure time, "
                "and bandwidth.")
                .unit(Unit::HERTZ)
                .readOnly()
                .commit();

    }

    BaslerCamera::BaslerCamera(const karabo::util::Hash& config) : AravisCamera(config) {
    }

    BaslerCamera::~BaslerCamera() {
    }

} // namespace karabo
