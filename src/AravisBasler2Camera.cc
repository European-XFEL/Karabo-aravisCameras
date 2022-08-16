/*
 * Author: <parenti>
 *
 * Created on April 27, 2021,  6:00 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisBasler2Camera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, ...)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisBasler2Camera)

    void AravisBasler2Camera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisBaslerBase::expectedParameters(expected);

        OVERWRITE_ELEMENT(expected).key("pixelFormat")
                .setNewOptions("Mono8,Mono10,Mono10p,Mono12,Mono12p")
                .setNewDefaultValue("Mono12p")
                .commit();

        // This class supports the following models: ace2 (Area Scan)
        const std::vector<std::string> supportedModels = {"a2A"};
        OVERWRITE_ELEMENT(expected).key("supportedModels")
                .setNewDefaultValue(supportedModels)
                .commit();
    }

    AravisBasler2Camera::AravisBasler2Camera(const karabo::util::Hash& config) : AravisBaslerBase(config) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
        m_is_frame_count_available = false;
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

        boost::mutex::scoped_lock lock(m_camera_mtx);

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
        boost::mutex::scoped_lock lock(m_camera_mtx);

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

    bool AravisBasler2Camera::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) {
        return AravisBaslerBase::get_timestamp(buffer, ts, "BslChunkTimestampValue");
    }

} // namespace karabo
