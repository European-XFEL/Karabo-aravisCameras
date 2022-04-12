/*
 * Author: <parenti>
 *
 * Created on October 20, 2020, 16:34 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisBaslerCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile in Karabo 2.8.1 - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, AravisBaslerCamera)
    // XXX Work-around: do not register the base class parameters here,
    //     do it in AravisAravisBaslerCamera::expectedParameters
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisBaslerCamera)

    void AravisBaslerCamera::expectedParameters(Schema& expected) {
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

    AravisBaslerCamera::AravisBaslerCamera(const karabo::util::Hash& config) : AravisCamera(config),
            m_ptp_enabled(false), m_tick_frequency(0) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
        m_last_clock_reset.now();
        this->registerScene(boost::bind(&Self::aravisBaslerScene, this), "scene");
    }

    AravisBaslerCamera::~AravisBaslerCamera() {
    }

    void AravisBaslerCamera::resetCamera() {
        GError* error = nullptr;

        // set a post connection helper such that we reset rois and binning. Reset cameras messes these up
        m_post_connection_cb = boost::bind(&AravisBaslerCamera::reset_roi_and_binning,
                                           this, 
                                           this->get<int>("roi.x"),
                                           this->get<int>("roi.y"),
                                           this->get<int>("roi.width"),
                                           this->get<int>("roi.height"),
                                           this->get<int>("bin.x"),
                                           this->get<int>("bin.y"));

        boost::mutex::scoped_lock lock(m_camera_mtx);
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

    bool AravisBaslerCamera::synchronize_timestamp() {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock lock(m_camera_mtx);

        // XXX Possibly use PTP in the future
        m_ptp_enabled = false;

        const karabo::util::Epochstamp epoch;
        if (epoch.elapsed(m_last_clock_reset).getTotalSeconds() > 60.) {
            // Verify synchronization once every minute
            bool resetNeeded = false;
            if (m_min_latency > 0. && m_max_latency / m_min_latency > 5.) {
                // When min and max latency differ too much, the clock could need to be reset
                KARABO_LOG_FRAMEWORK_INFO << deviceId << ": max_latency / min_latency = " << m_max_latency / m_min_latency;
                resetNeeded = true;
            } else if (m_max_latency > 3.) {
                // Max latency higher than 3 s
                KARABO_LOG_FRAMEWORK_INFO << deviceId << ": max_latency = " << m_max_latency << " s";
                resetNeeded = true;
            }

            if (resetNeeded) {
                const std::string message("Timestamp synchronization loss -> reset timestamp");
                KARABO_LOG_WARN << message;
                this->set("status", message);
                arv_camera_execute_command(m_camera, "GevTimestampControlReset", nullptr);
                arv_camera_execute_command(m_camera, "GevTimestampControlLatchReset", nullptr);
                m_last_clock_reset.now();
            }
        }

        m_tick_frequency = this->get<int>("gevTimestampTickFrequency");

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        // Get current timestamp on the camera.
        // It has been verified on an acA640-120gm that this takes 1 ms ca.,
        // thus this is the precision we can aim to in the synchronization.
        arv_camera_execute_command(m_camera, "GevTimestampControlLatch", &error);
        if (error == nullptr) m_reference_camera_timestamp = arv_camera_get_integer(m_camera, "GevTimestampValue", &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not synchronize timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisBaslerCamera::configure_timestamp_chunk() {
        GError* error = nullptr;
        boost::mutex::scoped_lock lock(m_camera_mtx);

        // Enable chunk data
        arv_camera_set_chunk_mode(m_camera, true, &error);

        // Those will be needed to get frame shape and pixel format
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Timestamp", true, &error);
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Width", true, &error);
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Height", true, &error);
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "PixelFormat", true, &error);

        if (error != nullptr) {
            arv_camera_set_chunk_mode(m_camera, false, nullptr);
            m_chunk_mode = false;
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": Could not enable timestamp chunk: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        m_chunk_mode = true;
        return true; // success
    }

    bool AravisBaslerCamera::get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const {
        GError* error = nullptr;
        boost::mutex::scoped_lock lock(m_camera_mtx);

        width = arv_chunk_parser_get_integer_value(m_parser, buffer, "ChunkWidth", &error);
        if (error == nullptr) height = arv_chunk_parser_get_integer_value(m_parser, buffer, "ChunkHeight", &error);
        if (error == nullptr) format = arv_chunk_parser_get_integer_value(m_parser, buffer, "ChunkPixelFormat", &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": Could not get image shape or format: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisBaslerCamera::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) {
        // XXX Possibly use PTP in the future
        GError* error = nullptr;

        // Get timestamp from buffer
        const gint64 timestamp = arv_chunk_parser_get_integer_value(m_parser, buffer, "ChunkTimestamp", &error);
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

    bool AravisBaslerCamera::is_flip_x_available() const {
        return true;
    }

    bool AravisBaslerCamera::is_flip_y_available() const {
        GError* error = nullptr;
        bool value;
        const std::string& deviceId = this->getInstanceId();

        if (!keyHasAlias("flip.Y")) return false; // No alias means no feature available

        const std::string feature = this->getAliasFromKey<std::string>("flip.Y");

        boost::mutex::scoped_lock lock(m_camera_mtx);

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

    void AravisBaslerCamera::reset_roi_and_binning(int x, int y, int width, int height, int bin_x, int bin_y) {
        Hash h;
        h.set("roi.x", x);
        h.set("roi.y", y);
        h.set("roi.height", height);
        h.set("roi.width", width);
        h.set("bin.x", bin_x);
        h.set("bin.y", bin_y);
        this->set(h);

        bool success = this->set_region(x, y, width, height);
        if (!success) {
            const std::string message("Could not set ROI after resetting camera!");
            KARABO_LOG_ERROR << message;
            this->set("status", message);
        }
        success = this->set_binning(bin_x, bin_y);
        if (!success) {
            const std::string message("Could not set binning after resetting camera!");
            KARABO_LOG_ERROR << message;
            this->set("status", message);
        }
    }

} // namespace karabo
