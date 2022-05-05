/*
 * Author: <parenti>
 *
 * Created on October 20, 2020,  4:34 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisBaslerCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, ...)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisBaslerCamera)

    void AravisBaslerCamera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisBaslerBase::expectedParameters(expected);

        // This class supports the following models: ace, aviator, pilot (Area Scan) and racer (Line Scan)
        const std::vector<std::string> supportedModels = {"acA", "avA", "piA", "raL"};
        OVERWRITE_ELEMENT(expected).key("supportedModels")
                .setNewDefaultValue(supportedModels)
                .commit();
    }

    AravisBaslerCamera::AravisBaslerCamera(const karabo::util::Hash& config) : AravisBaslerBase(config) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
        m_last_clock_reset.now();
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

    bool AravisBaslerCamera::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) {
        return AravisBaslerBase::get_timestamp(buffer, ts, "ChunkTimestamp");
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
