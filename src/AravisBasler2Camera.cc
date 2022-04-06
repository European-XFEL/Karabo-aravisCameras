/*
 * Author: <parenti>
 *
 * Created on April 27, 2020, 18:00 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisBasler2Camera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile in Karabo 2.8.1 - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, AravisBasler2Camera)
    // XXX Work-around: do not register the base class parameters here,
    //     do it in AravisBasler2Camera::expectedParameters
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisBasler2Camera)

    void AravisBasler2Camera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisCamera::expectedParameters(expected);

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS                                             *
        // **************************************************************************************************************

        OVERWRITE_ELEMENT(expected).key("pixelFormat")
                .setNewOptions("Mono8,Mono10,Mono10p,Mono12,Mono12p")
                .setNewDefaultValue("Mono12p")
                .commit();

        OVERWRITE_ELEMENT(expected).key("flip.X")
                .setNewAlias("ReverseX")
                .setNewTags({"genicam"})
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

    }

    AravisBasler2Camera::AravisBasler2Camera(const karabo::util::Hash& config) : AravisCamera(config),
            m_ptp_enabled(false), m_tick_frequency(0) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
        m_is_frame_count_available = false;
    }

    AravisBasler2Camera::~AravisBasler2Camera() {
    }

    void AravisBasler2Camera::resetCamera() {
        GError* error = nullptr;

        arv_camera_execute_command(m_camera, "DeviceReset", &error);

        if (error != nullptr) {
            const std::string message("Could not reset camera");
            KARABO_LOG_ERROR << message << ": " << error->message;
            this->set("status", message);
            g_clear_error(&error);
        } else {
            this->set("status", "Camera reset");
        }
    }

    bool AravisBasler2Camera::synchronize_timestamp() {
        GError* error = nullptr;

        // XXX Possibly use PTP in the future
        m_ptp_enabled = false;

        // XXX The counter cannot be reset during operation on a2A cameras:
        // https://docs.baslerweb.com/timestamp#specifics
        // In case of synchronization loss, a camera reset could be needed.

        m_tick_frequency = this->get<int>("gevTimestampTickFrequency");

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        // Get current timestamp on the camera.
        // It has been verified on an a2A2590-22gmPRO that this takes 4 ms ca.,
        // thus this is the precision we can aim to in the synchronization.
        arv_camera_execute_command(m_camera, "TimestampLatch", &error);
        if (error == nullptr) m_reference_camera_timestamp = arv_camera_get_integer(m_camera, "TimestampLatchValue", &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": Could not synchronize timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisBasler2Camera::configure_timestamp_chunk() {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();

        // Enable chunk data
        arv_camera_set_chunk_mode(m_camera, true, &error);

        // Enable timestamp chunk
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Timestamp", true, &error);

        // Refer the timestamp to acquisition start of current image
        if (error == nullptr) arv_device_set_string_feature_value(m_device, "BslChunkTimestampSelector", "FrameStart", &error);

        if (error != nullptr) {
            arv_camera_set_chunk_mode(m_camera, false, nullptr);
            m_chunk_mode = false;
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not enable timestamp chunk: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        m_chunk_mode = true;
        return true; // success
    }

    bool AravisBasler2Camera::get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const {
        // For ACE2, shape and format must be read from buffer, independently from data chunks being enabled

        gint x, y;
        arv_buffer_get_image_region(buffer, &x, &y, &width, &height);
        format = arv_buffer_get_image_pixel_format(buffer); // e.g. ARV_PIXEL_FORMAT_MONO_8

        return true; // success
    }

    bool AravisBasler2Camera::is_flip_x_available() const {
        return true;
    }

    bool AravisBasler2Camera::is_flip_y_available() const {
        GError* error = nullptr;
        bool value;
        const std::string& deviceId = this->getInstanceId();

        if (!keyHasAlias("flip.Y")) return false; // No alias means no feature available

        const std::string feature = this->getAliasFromKey<std::string>("flip.Y");

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

    bool AravisBasler2Camera::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) {
        // XXX Possibly use PTP in the future

        GError* error = nullptr;

        // Get timestamp from buffer
        const gint64 timestamp = arv_chunk_parser_get_integer_value(m_parser, buffer, "BslChunkTimestampValue", &error);
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

} // namespace karabo
