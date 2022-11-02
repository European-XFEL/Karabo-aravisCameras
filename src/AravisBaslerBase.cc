/*
 * Author: <parenti>
 *
 * Created on January 18, 2022,  3:28 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisBaslerBase.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, AravisBaslerBase)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisBaslerBase)

    void AravisBaslerBase::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisCamera::expectedParameters(expected);

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS                                             *
        // **************************************************************************************************************

        OVERWRITE_ELEMENT(expected).key("flip.X")
                .setNewAlias("ReverseX")
                .setNewTags({"genicam"})
                .commit();

        // This class supports cameras from Basler
        OVERWRITE_ELEMENT(expected).key("supportedVendor")
                .setNewDefaultValue("Basler")
                .commit();

        OVERWRITE_ELEMENT(expected).key("flip.Y")
                .setNewAlias("ReverseY")
                .setNewTags({"genicam"})
                .commit();

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

        INT32_ELEMENT(expected).key("gevSCFTD")
                .alias("GevSCFTD")
                .tags("genicam")
                .displayedName("Frame Transmission Delay")
                .description("The GevSCFTD parameter sets a delay in ticks between when a camera "
                "would normally begin transmitting an acquired frame and when it actually begins "
                "transmission. In most cases, this parameter should be set to zero. However, if "
                "your network hardware can't handle spikes in network traffic (e.g., if you are "
                "triggering multiple cameras simultaneously), you can use the frame transmission "
                "delay parameter to stagger the start of image data transmissions from each "
                "camera.")
                .assignmentOptional().defaultValue(0)
                .minInc(0).maxInc(50000000)
                .unit(Unit::NUMBER)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("gevTimestampTickFrequency")
                .alias("GevTimestampTickFrequency")
                .tags("genicam")
                .displayedName("Tick Frequency")
                .description("This value indicates the number of clock ticks per second.")
                .unit(Unit::HERTZ)
                .readOnly()
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

    AravisBaslerBase::AravisBaslerBase(const karabo::util::Hash& config) : AravisCamera(config),
            m_ptp_enabled(false), m_tick_frequency(0) {
        m_is_base_class = false;
        this->registerScene(boost::bind(&AravisBaslerBase::aravisBaslerScene, this), "scene");
    }

    bool AravisBaslerBase::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts, const std::string& tsFeature) {
        // XXX Possibly use PTP in the future
        GError* error = nullptr;

        // Get timestamp from buffer
        gint64 timestamp;
        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            timestamp = arv_chunk_parser_get_integer_value(m_parser, buffer, tsFeature.c_str(), &error);
        }
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": Could not read image timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        // Elapsed time since last synchronization.
        // NB This can be negative, if the image acquisition started before
        //    synchronization, but finished after.
        const double elapsed_t = double(timestamp - m_reference_camera_timestamp) / m_tick_frequency;

        // Split elapsed time in seconds and attoseconds, then convert to TimeDuration.
        // elapsed_t is in seconds and TimeDuration expects fractions in attoseconds,
        // hence we multiply with 1e18.
        // A TimeDuration can only be positive, thus save sign and invert if negative.
        double intpart;
        const char sign = (elapsed_t >= 0.) ? 1 : -1;
        const unsigned long long fractions = 1.e+18 * modf(sign * elapsed_t, &intpart);
        const unsigned long long seconds = rint(intpart);
        const TimeDuration duration(seconds, fractions);

        // Calculate frame epochstamp from refrence time and elapsed time
        Epochstamp epoch(m_reference_karabo_time.getEpochstamp());
        if (seconds <= m_max_correction_time) {
            if (this->get<bool>("wouldCorrectAboveMaxTime")) {
                this->set("wouldCorrectAboveMaxTime", false);
            }
            if (sign >= 0) {
                epoch += duration;
            } else {
                epoch -= duration;
            }
        } else if (!this->get<bool>("wouldCorrectAboveMaxTime")) {
            this->set("wouldCorrectAboveMaxTime", true);
            return false;
        }

        // Calculate timestamp from epochstamp
        ts = this->getTimestamp(epoch);
        return true;
    }

    bool AravisBaslerBase::is_flip_x_available() const {
        return true;
    }

    bool AravisBaslerBase::is_flip_y_available() const {
        GError* error = nullptr;
        bool value;
        const std::string& deviceId = this->getInstanceId();

        // After first connection alias is removed. The check is needed in case of re-connection.
        if (!keyHasAlias("flip.Y")) return false;

        const std::string feature = this->getAliasFromKey<std::string>("flip.Y");

        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Try to read flip.Y
        value = arv_device_get_boolean_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
            return false;
        } else if (value) { // Flip Y is set, thus available
            return true;
        }

        // Try to set flip.Y
        arv_device_set_boolean_feature_value(m_device, feature.c_str(), true, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_set_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
            return false;
        }

        // Verify that flip.Y has been set: the parameter is available but not settable on some models
        value = arv_device_get_boolean_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
            return false;
        } else if (value) { // Flip Y is set, thus available
            arv_device_set_boolean_feature_value(m_device, feature.c_str(), false, &error); // Set back
            return true;
        } else {
            return false;
        }
    }

    void AravisBaslerBase::resetCamera() {
        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        arv_camera_execute_command(m_camera, "DeviceReset", &error);

        if (error != nullptr) {
            const std::string message("Could not reset camera");
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": " << message << ": " << error->message;
            this->set("status", message);
            g_clear_error(&error);
        } else {
            this->set("status", "Camera reset");
        }
    }

} // namespace karabo
