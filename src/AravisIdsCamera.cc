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
    OVERWRITE_ELEMENT(expected)
        .key("supportedModels")
        .setNewDefaultValue(supportedModels)
        .commit();

    OVERWRITE_ELEMENT(expected)
        .key("flip.X")
        .setNewAlias("ReverseX")
        .setNewTags({"genicam"})
        .commit();

    OVERWRITE_ELEMENT(expected)
        .key("flip.Y")
        .setNewAlias("ReverseY")
        .setNewTags({"genicam"})
        .commit();

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
    : AravisCamera(config) {
    m_is_base_class = false;
    m_is_device_reset_available = true;
}

void AravisIdsCamera::postAcquisitionStop() {
    this->clear_stream();
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
        KARABO_LOG_FRAMEWORK_ERROR
            << this->getInstanceId() << ": " << message << ": " << error->message;
        this->set("status", message);
        g_clear_error(&error);
    } else {
        this->set("status", "Camera reset");
    }
}

} // namespace karabo
