#include "AravisIdsCamera.hh"

USING_KARABO_NAMESPACES

namespace karabo {

    KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisIdsCamera)

    void AravisIdsCamera::expectedParameters(Schema& expected) {
        AravisCamera::expectedParameters(expected);

        OVERWRITE_ELEMENT(expected)
              .key("supportedVendor")
              .setNewDefaultValue("IDS Imaging Development Systems GmbH")
              .commit();

        const std::vector<std::string> supportedModels = {"GV-53FxLE-M", "GV-58CxLE-M"};
        OVERWRITE_ELEMENT(expected).key("supportedModels").setNewDefaultValue(supportedModels).commit();

        OVERWRITE_ELEMENT(expected).key("flip.X").setNewAlias("ReverseX").setNewTags({"genicam"}).commit();

        OVERWRITE_ELEMENT(expected).key("flip.Y").setNewAlias("ReverseY").setNewTags({"genicam"}).commit();

        FLOAT_ELEMENT(expected)
            .key("temperature")
            .alias("DeviceTemperature")
            .tags("poll")
            .displayedName("Temperature")
            .description("Shows the current temperature of the camera board.")
            .unit(Unit::DEGREE_CELSIUS)
            .readOnly()
            .commit();

    }

    AravisIdsCamera::AravisIdsCamera(const karabo::data::Hash& config)
        : AravisCamera(config),
        m_is_timestamp_latch_available(false),
        m_reference_camera_timestamp(0.) {
        m_is_base_class = false;
        m_is_device_reset_available = true;
    }

    bool AravisIdsCamera::is_flip_x_available() const {
        return this->isFeatureAvailable("ReverseX");
    }

    bool AravisIdsCamera::is_flip_y_available() const {
        return this->isFeatureAvailable("ReverseY");
    }

    std::string AravisIdsCamera::get_frame_rate_enable_parameter_name() const {
        return "AcquisitionFrameRateTargetEnable";
    }

    void AravisIdsCamera::resetCamera() {
        GError* error = nullptr;

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_execute_command(m_camera, "DeviceReset", &error);
        }

        if (error != nullptr) {
            const std::string message("Could not reset camera");
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": " << message << ": " << error->message;
            this->set("status", message);
            g_clear_error(&error);
        } else {
            this->set("status", "Camera reset");
        }
    }

    bool AravisIdsCamera::synchronize_timestamp() {
        const std::string& deviceId = this->getInstanceId();

        if (!m_is_timestamp_latch_available) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << "Could not synchronize timestamp: TimestampLatch feature not available";
            return false;
        }

        GError* error = nullptr;
        double camera_timestamp;

        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get current timestamp on the camera (TimestampLatchValue).
        // TimestampLatchValue counts the number of ticks since the last reset of the counter.
        arv_camera_execute_command(m_camera, "TimestampLatch", &error);
        if (error == nullptr) camera_timestamp = arv_camera_get_integer(m_camera, "TimestampLatchValue", &error);

        if (error != nullptr) {
            const std::string message("Could not synchronize timestamp");
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not synchronize timestamp: " << error->message;
            g_clear_error(&error);
            this->set("status", message);
            return false; // failure
        }

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        // converting camera_timestamp from ns to s
        m_reference_camera_timestamp = camera_timestamp / 1e+9;

        return true; // success
    }

    int AravisIdsCamera::get_tick_frequency() {
        if (m_is_gv_device) {
            // Only tested on GEV cameras, but it might work on USB3V ones too        
            m_is_timestamp_latch_available = this->isFeatureAvailable("TimestampLatch") && this->isFeatureAvailable("TimestampLatchValue");
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const int tickFrequency =
                    arv_device_get_float_feature_value(
                        m_device, "DeviceClockFrequency", nullptr);

            return tickFrequency;
        }

        return 0;
    }

    bool AravisIdsCamera::get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) {
        // Get timestamp from buffer.
        // The timestamp is provided in ns, thus convert it to s.
        const double timestamp = arv_buffer_get_timestamp(buffer) / 1e+9;

        // Elapsed time since last synchronization.
        // NB This can be negative, if the image acquisition started before
        //    synchronization, but finished after.
        const double elapsed_t = timestamp - m_reference_camera_timestamp;

        // Split elapsed time in seconds and attoseconds, then convert to TimeDuration.
        // elapsed_t is in seconds and TimeDuration expects fractions in attoseconds,
        // hence we multiply with 1e18.
        // A TimeDuration can only be positive, thus save sign and invert if negative.
        double intpart;
        const char sign = (elapsed_t >= 0.) ? 1 : -1;
        const unsigned long long fractions = 1.e+18 * modf(sign * elapsed_t, &intpart);
        const unsigned long long seconds = rint(intpart);
        const TimeDuration duration(seconds, fractions);

        // Calculate frame epochstamp from reference time and elapsed time
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
