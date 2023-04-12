/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include <boost/algorithm/string/trim.hpp>

#include "AravisCamera.hh"

using namespace std;

USING_KARABO_NAMESPACES;

// Used by Basler ACE 2, but not (yet?) available in aravis
#define ARV_PIXEL_FORMAT_MONO_10_P              ((ArvPixelFormat) 0x010a0046u)
#define ARV_PIXEL_FORMAT_MONO_12_P              ((ArvPixelFormat) 0x010c0047u)

#define GET_PATH(hash, path, type) hash.has(path) ? hash.get<type>(path) : this->get<type>(path);

namespace karabo {

    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera)

    boost::mutex AravisCamera::m_connect_mtx;

    void AravisCamera::expectedParameters(Schema& expected) {
        OVERWRITE_ELEMENT(expected).key("state")
                .setNewOptions(State::UNKNOWN, State::ERROR, State::ON, State::ACQUIRING)
                .commit();

        STRING_ELEMENT(expected).key("idType")
                .displayedName("ID Type")
                .description("The type of identifier to be used, to connect to the camera."
                "Available options are 'IP' (IP address), 'HOST' (IP name), SN (Serial Number), MAC (MAC address).")
                .assignmentOptional().defaultValue("IP")
                .options("IP,HOST,SN,MAC")
                .init()
                .commit();

        STRING_ELEMENT(expected).key("cameraId")
                .displayedName("Camera ID")
                .description("The 'identifier' of the network camera. It can be an IP address (e.g. 192.168.1.153), "
                "an IP name (e.g. exflqr1234), a serial number or a MAC address (e.g. 00:30:53:25:ab:b7). "
                "The type must be specified in the 'idType' property.")
                .assignmentMandatory()
                .init() // cannot be changed after device instantiation
                .commit();

        INT64_ELEMENT(expected).key("packetDelay")
                .displayedName("Packet Delay")
                .description("Configure the inter packet delay to insert between each packet for the current stream. "
                "This can be used as a crude flow-control mechanism if the application or the network "
                "infrastructure cannot keep up with the packets coming from the device.")
                .assignmentOptional().noDefaultValue()
                .minInc(0)
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::NANO)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("autoPacketSize")
                .displayedName("Auto Packet Size")
                .description("Automatically determine the biggest packet size that can be used for data streaming, "
                "and set its value accordingly. If this functionality is not available, the packet size will be set "
                "to a default value (1500 bytes).")
                .assignmentOptional().defaultValue(true)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("packetSize")
                .displayedName("Packet Size")
                .description("Specifies the packet size to be used by the camera for data streaming. "
                "This does not include data leader and data trailer and the last data packet which might be "
                "of smaller size.")
                .assignmentOptional().noDefaultValue()
                .minExc(0)
                .unit(Unit::BYTE)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        SLOT_ELEMENT(expected).key("acquire")
                .displayedName("Acquire")
                .allowedStates(State::ON)
                .commit();

        SLOT_ELEMENT(expected).key("stop")
                .displayedName("Stop")
                .allowedStates(State::ACQUIRING)
                .commit();

        SLOT_ELEMENT(expected).key("trigger")
                .displayedName("Software Trigger")
                .allowedStates(State::ACQUIRING)
                .commit();

        SLOT_ELEMENT(expected).key("refresh")
                .displayedName("Refresh")
                .description("Refresh hardware parameters and options.")
                .allowedStates(State::ON)
                .commit();

        SLOT_ELEMENT(expected).key("reset")
                .displayedName("Reset")
                .description("'Software' reset, i.e. just reset the error state.")
                .allowedStates(State::ERROR)
                .commit();

        NODE_ELEMENT(expected).key("frameRate")
                .displayedName("Frame Rate")
                .commit();

        BOOL_ELEMENT(expected).key("frameRate.enable")
                .displayedName("Frame Rate Enable")
                .description("Enable the frame rate control.")
                .assignmentOptional().defaultValue(false)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        FLOAT_ELEMENT(expected).key("frameRate.target")
                .displayedName("Target Frame Rate")
                .description("Sets the 'target' value of the acquisition frame rate on the camera. "
                "Please be aware that if you enable this feature in combination with external trigger, "
                "the resulting 'actual' frame rate will most likely be smaller.")
                .assignmentOptional().defaultValue(10.)
                .minInc(0.)
                .unit(Unit::HERTZ)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        FLOAT_ELEMENT(expected).key("frameRate.actual")
                .displayedName("Actual Frame Rate")
                .description("The measured frame rate.")
                .unit(Unit::HERTZ)
                .readOnly()
                .initialValue(0.)
                .commit();

        UINT64_ELEMENT(expected).key("errorCount")
                .displayedName("Acq. Error Count")
                .description("The number of errors occurred during acquisition.")
                .unit(Unit::COUNT)
                .readOnly()
                .initialValue(0)
                .commit();

        STRING_ELEMENT(expected).key("lastError")
                .displayedName("Last Acq. Error")
                .description("Description of the last error occurred during acquisition.")
                .readOnly()
                .initialValue("")
                .commit();

        NODE_ELEMENT(expected).key("latency")
                .displayedName("Image Latency")
                .description("The latency between the image timestamp - if available - and the "
                "reception time. The reference interval is 1 s.")
                .commit();

        FLOAT_ELEMENT(expected).key("latency.mean")
                .displayedName("Mean Latency")
                .description("Mean image latency.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MILLI)
                .readOnly().initialValue(0.)
                .commit();

        FLOAT_ELEMENT(expected).key("latency.min")
                .displayedName("Min Latency")
                .description("Minimum image latency.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MILLI)
                .readOnly().initialValue(0.)
                .commit();

        FLOAT_ELEMENT(expected).key("latency.max")
                .displayedName("Max Latency")
                .description("Maximum image latency.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MILLI)
                .readOnly().initialValue(0.)
                .commit();

        INT32_ELEMENT(expected).key("pollingInterval")
                .displayedName("Polling Interval")
                .description("The interval for polling the camera for read-out values.")
                .assignmentOptional().defaultValue(20)
                .unit(Unit::SECOND)
                .minInc(5).maxInc(60)
                .reconfigurable()
                .commit();

        STRING_ELEMENT(expected).key("camId")
                .displayedName("Camera ID")
                .readOnly().initialValue("")
                .commit();

        STRING_ELEMENT(expected).key("vendor")
                .displayedName("Vendor Name")
                .description("The vendor of the camera.")
                .readOnly().initialValue("")
                .commit();

        STRING_ELEMENT(expected).key("supportedVendor")
                .displayedName("Supported Vendor")
                .description("The vendor supported by this Karabo device.")
                .readOnly().initialValue("")
                .commit();

        STRING_ELEMENT(expected).key("model")
                .displayedName("Model Name")
                .description("The model of the camera.")
                .readOnly().initialValue("")
                .commit();

        VECTOR_STRING_ELEMENT(expected).key("supportedModels")
                .displayedName("Supported Models")
                .description("The camera models supported by this Karabo device.")
                .readOnly().initialValue({})
                .commit();

        INT32_ELEMENT(expected).key("width")
                .displayedName("Sensor Width")
                .readOnly().initialValue(0)
                .commit();

        INT32_ELEMENT(expected).key("height")
                .displayedName("Sensor Height")
                .readOnly().initialValue(0)
                .commit();

        NODE_ELEMENT(expected).key("roi")
                .displayedName("Image ROI")
                .commit();

        INT32_ELEMENT(expected).key("roi.x")
                .displayedName("X Offset")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(0)
                .minInc(0)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("roi.y")
                .displayedName("Y Offset")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(0)
                .minInc(0)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("roi.width")
                .displayedName("Width")
                .description("The ROI width. Use '0' for the whole sensor width.")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(0)
                .minInc(0)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("roi.height")
                .displayedName("Height")
                .description("The ROI height. Use '0' for the whole sensor height.")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(0)
                .minInc(0)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        NODE_ELEMENT(expected).key("bin")
                .displayedName("Image Binning")
                .commit();

        INT32_ELEMENT(expected).key("bin.x")
                .displayedName("X Binning")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(1)
                .minInc(1)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("bin.y")
                .displayedName("Y Binning")
                .unit(Unit::PIXEL)
                .assignmentOptional().defaultValue(1)
                .minInc(1)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        NODE_ELEMENT(expected).key("flip")
                .displayedName("Image Flip")
                .description("Enables mirroring of the image.")
                .commit();

        BOOL_ELEMENT(expected).key("flip.X")
                .displayedName("Horizonzal Flip")
                .description("Enable horizontal flip. This is done before the image rotation.")
                .assignmentOptional().defaultValue(false)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("flip.Y")
                .displayedName("Vertical Flip")
                .description("Enable vertical flip. This is done before the image rotation.")
                .assignmentOptional().defaultValue(false)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        UINT32_ELEMENT(expected).key("rotation")
                .displayedName("Image Rotation")
                .description("The image rotation. This is done after the image flip.")
                .assignmentOptional().defaultValue(0)
                .options("0,90,180,270")
                .unit(Unit::DEGREE)
                .allowedStates(State::UNKNOWN, State::ON)
                .reconfigurable()
                .commit();

        STRING_ELEMENT(expected).key("pixelFormat")
                .displayedName("Pixel Format")
                .description("This enumeration sets the format of the pixel data transmitted for acquired images. "
                "For example Mono8 means monochromatic, 8 bits-per-pixel.")
                .assignmentOptional().defaultValue("Mono12Packed")
                // Fill-up with some commonly available options. They will be updated on connection.
                .options("Mono8,Mono12,Mono12Packed")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        UINT16_ELEMENT(expected).key("bpp")
                .displayedName("Bits-per-pixel")
                .readOnly().initialValue(0)
                .commit();

        DOUBLE_ELEMENT(expected).key("exposureTime")
                .displayedName("Exposure Time")
                .description("This float value sets the camera's exposure time. "
                "It can only be a multiple of the minimum exposure time.")
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MICRO)
                .assignmentOptional().defaultValue(10.)
                .minExc(0.)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerSelector")
                .displayedName("Trigger Selector")
                .description("This enumeration selects the trigger type to configure. "
                "Once a trigger type has been selected, all changes to the trigger settings will be applied to "
                "the selected trigger.")
                .assignmentOptional().noDefaultValue()
                // options will be injected on connection
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerMode")
                .displayedName("Trigger Mode")
                .description("This enumeration enables or disables the selected trigger.")
                .assignmentOptional().defaultValue("Off")
                .options("On,Off")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerSource")
                .displayedName("Trigger Source")
                .description("This enumeration sets the signal source for the selected trigger.")
                .assignmentOptional().noDefaultValue()
                // options will be injected on connection
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerActivation")
                .displayedName("Trigger Activation")
                .description("This enumeration sets the signal transition needed to activate the selected trigger.")
                .assignmentOptional().defaultValue("RisingEdge")
                .options("RisingEdge,FallingEdge")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("autoGain")
                .displayedName("Auto Gain")
                .description("Configures automatic gain feature.")
                .assignmentOptional().defaultValue("Off")
                .options("Off,Once,Continuous")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("isNormGain")
                .displayedName("Enable Normalized Gain")
                .description("The 'gain' parameter will be interpreted as 'normalized'.")
                .assignmentOptional().defaultValue(true)
                .init() // XXX What happens if I make it reconfigurable?
                .commit();

        DOUBLE_ELEMENT(expected).key("gain")
                .displayedName("Gain")
                .description("Sets the gain of the ADC converter.")
                .assignmentOptional().noDefaultValue()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        DOUBLE_ELEMENT(expected).key("absGain")
                .displayedName("Absolute Gain")
                .description("The absolute gain of the ADC converter.")
                .readOnly()
                .commit();

        STRING_ELEMENT(expected).key("acquisitionMode")
                .displayedName("Acquisition Mode")
                .description("This property sets the image acquisition mode.")
                .assignmentOptional().defaultValue("Continuous")
                .options("Continuous,SingleFrame,MultiFrame")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT64_ELEMENT(expected).key("frameCount")
                .displayedName("Frame Count")
                .description("This value sets the number of frames acquired in the 'Multiframe' acquisition mode.")
                .assignmentOptional().noDefaultValue()
                .minInc(1)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        const std::vector<std::string> interfaces = {"Camera"};
        VECTOR_STRING_ELEMENT(expected).key("interfaces")
                .expertAccess()
                .readOnly().initialValue(interfaces)
                .commit();

        UINT32_ELEMENT(expected).key("maxCorrectionTime")
                .displayedName("Max. Train Correction Time")
                .description("Maximum time the clock based train Id correction will correct. If the delay "
                "is outside this time, no correction will be performed.")
                .unit(Unit::SECOND)
                .assignmentOptional().defaultValue(5)
                .minInc(1).maxInc(600)
                .init()
                .commit();

        BOOL_ELEMENT(expected).key("wouldCorrectAboveMaxTime")
                .displayedName("Would Correct Above Max. Time")
                .description("True if a correction above maxCorrectionTime would happen.")
                .readOnly()
                .commit();
    }


    AravisCamera::AravisCamera(const karabo::util::Hash& config) : CameraImageSource(config),
            m_is_base_class(true), m_arv_camera_trigger(true), m_is_device_reset_available(false),
            m_is_frame_count_available(false), m_camera(nullptr), m_device(nullptr), m_parser(nullptr),
            m_chunk_mode(false), m_width(0), m_height(0), m_format(0),
            m_max_correction_time(0), m_min_latency(0.),
            m_max_latency(0.), m_connect(true), m_is_connected(false),
            m_reconnect_timer(EventLoop::getIOService()), m_failed_connections(0u),
            m_poll_timer(EventLoop::getIOService()), m_is_acquiring(false), m_stream(nullptr),
            m_is_binning_available(false), m_is_exposure_time_available(false),
            m_is_flip_x_available(false), m_is_flip_y_available(false),
            m_is_frame_rate_available(false), m_is_gain_available(false), m_is_gain_auto_available(false),
            m_errorCount(0), m_lastError(ARV_BUFFER_STATUS_SUCCESS),
            m_counter(0), m_sum_latency(0.) {

        m_max_correction_time = config.get<unsigned int>("maxCorrectionTime");

        // From <arvbuffer.h>
        m_bufferStatus[ARV_BUFFER_STATUS_UNKNOWN] = "Unknown status";
        m_bufferStatus[ARV_BUFFER_STATUS_SUCCESS] = "The buffer contains a valid image";
        m_bufferStatus[ARV_BUFFER_STATUS_CLEARED] = "The buffer is cleared";
        m_bufferStatus[ARV_BUFFER_STATUS_TIMEOUT] = "Timeout was reached before all packets are received";
        m_bufferStatus[ARV_BUFFER_STATUS_MISSING_PACKETS] = "Stream has missing packets";
        m_bufferStatus[ARV_BUFFER_STATUS_WRONG_PACKET_ID] = "Stream has packet with wrong id";
        m_bufferStatus[ARV_BUFFER_STATUS_SIZE_MISMATCH] = "The received image didn't fit in the buffer data space";
        m_bufferStatus[ARV_BUFFER_STATUS_FILLING] = "The image is currently being filled";
        m_bufferStatus[ARV_BUFFER_STATUS_ABORTED] = "The filling was aborted before completion";

        KARABO_SLOT(acquire);
        KARABO_SLOT(stop);
        KARABO_SLOT(trigger);
        KARABO_SLOT(refresh);
        KARABO_SLOT(reset);
        KARABO_SLOT(resetCamera);

        KARABO_INITIAL_FUNCTION(initialize);
    }


    AravisCamera::~AravisCamera() {
        this->clear_stream();
        this->clear_camera();

        m_connect = false;
        m_reconnect_timer.cancel();
        m_poll_timer.cancel();
    }


    void AravisCamera::preReconfigure(karabo::util::Hash& incomingReconfiguration) {
        this->configure(incomingReconfiguration);
    }


    void AravisCamera::postReconfigure() {
        const bool success = this->updateOutputSchema();
        if (!success) {
            this->updateState(State::ERROR);
        }
    }


    void AravisCamera::getPathsByTag(std::vector<std::string>& paths, const std::string& tags) {
        // N.B. Device::getCurrentConfiguration(tags)) cannot be used, as it
        // does not return parameters with no value set

        const Schema schema = this->getFullSchema();
        const Hash& parameters = schema.getParameterHash();
        const Hash filteredParameters = this->filterByTags(parameters, tags);

        filteredParameters.getPaths(paths);
    }


    // Check that a feature is implemented and available on the camera
    // XXX For getting/setting a feature we should check that the feature is available
    //     (it could be implemented but temporarily unavailable.)
    //     For updating the schema we should only check that it is implemented.
    bool AravisCamera::isFeatureAvailable(const std::string& feature) const {
        if (m_device != nullptr) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            ArvGcNode* node = arv_device_get_feature(m_device, feature.c_str());
            if (node != nullptr && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE (node), NULL) &&
                arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE (node), NULL)) {
                // The feature is implemented and available
                return true;
            }
        }

        return false;
    }


    void AravisCamera::disableElement(const std::string& key, karabo::util::Schema& schemaUpdate) {
        OVERWRITE_ELEMENT(schemaUpdate).key(key)
            .setNewDescription("Not available on this camera.")
            .setNowReadOnly()
            .commit();
    }


    Result AravisCamera::getBoolFeature(const std::string& feature, bool& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        value = arv_device_get_boolean_feature_value(m_device, feature.c_str(), &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_device_get_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }


    Result AravisCamera::getStringFeature(const std::string& feature, std::string& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        value = arv_device_get_string_feature_value(m_device, feature.c_str(), &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_device_get_string_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }


    Result AravisCamera::getIntFeature(const std::string& feature, long long& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        value = arv_device_get_integer_feature_value(m_device, feature.c_str(), &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_device_get_integer_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }


    Result AravisCamera::getFloatFeature(const std::string& feature, double& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        value = arv_device_get_float_feature_value(m_device, feature.c_str(), &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_device_get_float_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }


    Result AravisCamera::setBoolFeature(const std::string& feature, bool& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        arv_device_set_boolean_feature_value(m_device, feature.c_str(), value, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_set_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
        } else {
            return Result::SUCCESS; // success
        }

        // read back value
        const bool rvalue = arv_device_get_boolean_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) { // Could not read back value
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_boolean_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        } else if (rvalue != value) { // The value was not set
            value = rvalue;
            return Result::FAIL;
        } else { // The value was set
            return Result::SUCCESS;
        }
    }


    Result AravisCamera::setStringFeature(const std::string& feature, std::string& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        arv_device_set_string_feature_value(m_device, feature.c_str(), value.c_str(), &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_set_string_feature_value failed: " << error->message;
            g_clear_error(&error);
        } else {
            return Result::SUCCESS; // success
        }

        // read back value
        const std::string rvalue = arv_device_get_string_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) { // Could not read back value
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_string_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        } else if (rvalue != value) { // The value was not set
            value = rvalue;
            return Result::FAIL;
        } else { // The value was set
            return Result::SUCCESS;
        }
    }


    Result AravisCamera::setIntFeature(const std::string& feature, long long& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        arv_device_set_integer_feature_value(m_device, feature.c_str(), value, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_set_integer_feature_value failed: " << error->message;
            g_clear_error(&error);
        } else {
            return Result::SUCCESS; // success
        }

        // read back value
        const long long rvalue = arv_device_get_integer_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) { // Could not read back value
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_integer_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        } else if (rvalue != value) { // The value was not set
            value = rvalue;
            return Result::FAIL;
        } else { // The value was set
            return Result::SUCCESS;
        }
    }


    Result AravisCamera::setFloatFeature(const std::string& feature, double& value) {
        if (!this->isFeatureAvailable(feature)) return Result::NOT_AVAILABLE;

        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        arv_device_set_float_feature_value(m_device, feature.c_str(), value, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ":arv_device_set_float_feature_value failed: " << error->message;
            g_clear_error(&error);
        } {
            return Result::SUCCESS; // success
        }

        // read back value
        const double rvalue = arv_device_get_float_feature_value(m_device, feature.c_str(), &error);
        if (error != nullptr) { // The value was not set
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_float_feature_value failed: " << error->message;
            g_clear_error(&error);
            return Result::FAIL;
        } else if (rvalue != value) { // The value was not set
            value = rvalue;
            return Result::FAIL;
        } else { // The value was set
            return Result::SUCCESS;
        }
    }


    void AravisCamera::initialize() {
        m_reconnect_timer.expires_from_now(boost::posix_time::milliseconds(1));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));

        m_poll_timer.expires_from_now(boost::posix_time::seconds(1l));
        m_poll_timer.async_wait(karabo::util::bind_weak(&AravisCamera::pollCamera, this, boost::asio::placeholders::error));
    }


    void AravisCamera::connect(const boost::system::error_code & ec) {
        if (ec == boost::asio::error::operation_aborted) return;
        if (!m_connect) return;

        if (m_is_connected) {
            // Already connected
            m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        } else {
            // Clear resources before trying reconnection
            this->clear_camera();
            this->clear_stream();
        }

        const std::string& idType = this->get<std::string>("idType");
        const std::string& cameraId = this->get<std::string>("cameraId");
        std::string cameraIp;

        // ArvInterface (e.g arv_interface_update_device_list) is not thread safe, thus I create
        // here a class level lock.
        boost::mutex::scoped_lock class_lock(AravisCamera::m_connect_mtx);

        if (idType == "IP") { // IP address
            if (cameraId.size() == 0) {
                this->connection_failed_helper("Cannot connect: the provided IP is empty");
                return;
            }

            cameraIp = cameraId;

        } else if (idType == "HOST") { // IP name
            std::string message;
            const bool success = this->resolveHostname(cameraId, cameraIp, message);
            if (!success) {
                this->connection_failed_helper(message);
                return;
            } else if (m_failed_connections < 1) {
                KARABO_LOG_INFO << message;
            }

        } else if (idType == "SN") { // Serial number
            // Update the internal list of available devices
            arv_update_device_list();

            for (unsigned int idx=0; idx < arv_get_n_devices(); ++idx) {
                // Look for a matching serial number
                if (cameraId == arv_get_device_serial_nbr(idx)) {
                    cameraIp = arv_get_device_address(idx);
                    if (m_failed_connections < 1) {
                        KARABO_LOG_INFO << "Serial number resolved: " << cameraId << " -> " <<  cameraIp;
                    }
                    break;
                }
            }
            if (cameraIp.size() == 0) {
                const std::string message("Could not discover any camera with serial: " + cameraId);
                this->connection_failed_helper(message);
                return;
            }

        } else if (idType == "MAC") { // MAC address
            // Update the internal list of available devices
            arv_update_device_list();

            for (unsigned int idx=0; idx < arv_get_n_devices(); ++idx) {
                // Look for a matching MAC address
                if (cameraId == arv_get_device_physical_id(idx)) {
                    cameraIp = arv_get_device_address(idx);
                    if (m_failed_connections < 1) {
                        KARABO_LOG_INFO << "MAC address resolved: " << cameraId << " -> " <<  cameraIp;
                    }
                    break;
                }
            }
            if (cameraIp.size() == 0) {
                const std::string message("Could not discover any camera with MAC: " + cameraId);
                this->connection_failed_helper(message);
                return;
            }

        }

        GError* error = nullptr;
        Hash h; // For the bulk update

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);

            m_camera = arv_camera_new(cameraIp.c_str(), &error);

            if (error != nullptr) {
                std::stringstream ss;
                ss << "arv_camera_new failed: " << error->message; // detailed message
                this->connection_failed_helper("Cannot connect to " + cameraIp, ss.str());
                g_clear_error(&error);
                return;
            }

            // Read immutable properties
            if (error == nullptr) h.set("camId", std::string(arv_camera_get_device_id(m_camera, &error)));
            const std::string vendor(arv_camera_get_vendor_name(m_camera, &error));
            if (error == nullptr) h.set("vendor", vendor);
            const std::string model(arv_camera_get_model_name(m_camera, &error));
            if (error == nullptr) h.set("model", model);

            // For derived classes, check that vendor and model are supported by the class
            const bool is_supported = m_is_base_class || verify_vendor_and_model(vendor, model);
            if (!is_supported) {
                this->set(h);
                // Must unlock before 'clear_camera' is called
                camera_lock.unlock();
                this->clear_camera();
                // Camera not supported -> quit connection loop. It can be restarted by 'reset'
                m_connect = false;
                this->updateState(State::ERROR);
                return;
            }

            // ArvDevice gives more complete access to camera features
            m_device = arv_camera_get_device(m_camera);

            // The following is a workaround due to the fact that ARAVIS 0.6 does
            // not decode the AccessStatus from the discovery pong.
            // Therefore we send a "TriggerSoftware" command, which is listed as
            // "recommended" in the GenICam standard, and check for status.
            // If it is not "SUCCESS" we assume it's because of another application
            // controlling the camera.
            arv_device_execute_command(m_device, "TriggerSoftware", &error);
            if (error != nullptr) {
                const std::string message("Cannot connect to " + cameraIp
                    + ". Another application might be controlling it.");
                std::stringstream ss;
                ss << "arv_device_execute_command failed: " << error->message; // detailed message
                this->connection_failed_helper(message, ss.str());
                g_clear_error(&error);
                return;
            }

            // Instantiation of a chunk parser
            m_parser = arv_camera_create_chunk_parser(m_camera);
        }

        // Enable chunk data, if available on the camera
        this->configure_timestamp_chunk();

        // Successfully connected!
        const std::string message("Connected to " + cameraIp);
        h.set("status", message);
        KARABO_LOG_INFO << message;

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            // Connect the control-lost signal
            g_signal_connect(m_device, "control-lost", G_CALLBACK(AravisCamera::control_lost_cb), static_cast<void*>(this));

            if (error == nullptr) {
                gint width, height;
                arv_camera_get_sensor_size(m_camera, &width, &height, &error);
                h.set("width", width);
                h.set("height", height);
            }

            if (error == nullptr) m_is_binning_available = arv_camera_is_binning_available(m_camera, &error);
            if (error == nullptr) m_is_exposure_time_available = arv_camera_is_exposure_time_available(m_camera, &error);
            if (error == nullptr) m_is_frame_rate_available = arv_camera_is_frame_rate_available(m_camera, &error);
            if (error == nullptr) m_is_gain_available = arv_camera_is_gain_available(m_camera, &error);
            if (error == nullptr) m_is_gain_auto_available = arv_camera_is_gain_auto_available(m_camera, &error);
        }

        // Verify whether horizontal and vertical flip are available on the camera
        m_is_flip_x_available = this->is_flip_x_available();
        m_is_flip_y_available = this->is_flip_y_available();

        // The exposure time feature name is used to read out the increment
        std::vector<std::string> features = {"ExposureTime", // e.g. Basler a2A
            "ExposureTimeRaw"}; // e.g. Basler acA
        for (const std::string feat : features) {
            if (this->isFeatureAvailable(feat)) {
                m_exposure_time_feature = feat;
                break;
            }
        }

        if (error != nullptr) {
            const std::string detailed_msg(error->message);
            this->connection_failed_helper("An error occurred whilst connecting to " + cameraIp, detailed_msg);
            g_clear_error(&error);
            return;
        }

        this->set(h);

        // Apply initial configuration
        Hash initialConfiguration = this->getCurrentConfiguration();
        this->configure(initialConfiguration);

        m_need_schema_update = true; // Always update schema upon connection
        const bool success = this->updateOutputSchema();
        if (!success) {
            this->connection_failed_helper("Could not update output schema");
            return;
        }

        if (m_is_acquiring) {
            // Connection to the camera was lost during acquisition -> restart it
            this->acquire();
        } else {
            this->updateState(State::ON);
        }

        m_is_connected = true;
        m_failed_connections = 0;
        m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
    }


    void AravisCamera::connection_failed_helper(const std::string& message, const std::string& detailed_msg) {
        const std::string& deviceId = this->getInstanceId();

        if (m_failed_connections < 1) {
            // Only log first error message
            KARABO_LOG_ERROR << message;
            this->set("status", message);
            if (!detailed_msg.empty()) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": " << detailed_msg;
            }
        } else {
            KARABO_LOG_DEBUG << message;
            if (!detailed_msg.empty()) {
                KARABO_LOG_FRAMEWORK_DEBUG << deviceId << ": " << detailed_msg;
            }
        }

        // Increase counter
        ++m_failed_connections;

        // Try reconnecting after some time
        m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
    }


    bool AravisCamera::verify_vendor_and_model(const std::string& vendor, const std::string& model) {
        const std::string& supportedVendor = this->get<std::string>("supportedVendor");
        if (vendor != supportedVendor) {
            const std::string msg("This Karabo device does not support cameras from " + vendor);
            KARABO_LOG_ERROR << msg;
            this->set("status", msg);
            return false;
        }

        const std::vector<std::string>& supportedModels = this->get<std::vector<std::string>>("supportedModels");
        for (const std::string& supported : supportedModels) {
            if (model.find(supported) == 0) {
                return true;
            }
        }
        const std::string msg("This Karabo device does not support model " + model +
            " from " + vendor);
        KARABO_LOG_ERROR << msg;
        this->set("status", msg);
        return false;
    }


    bool AravisCamera::set_auto_packet_size() {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        const guint packetSize = arv_camera_gv_auto_packet_size(m_camera, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_gv_auto_packet_size failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        arv_camera_gv_set_packet_size(m_camera, packetSize, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_gv_set_packet_size failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }


    bool AravisCamera::set_region(int& x, int& y, int& width, int& height) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Before getting width and height bounds, reset x and y offsets.
        arv_camera_set_region(m_camera, 0, 0, width, height, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_region failed: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        // Get bounds for width and height.
        // x and y bounds can only be gotten after width and height are applied.
        gint wmin, wmax, hmin, hmax;
        arv_camera_get_width_bounds(m_camera, &wmin, &wmax, &error);
        if (error == nullptr) {
            arv_camera_get_height_bounds(m_camera, &hmin, &hmax, &error);
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << "Could not get ROI bounds: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        if (width == 0) {
            // Whole sensor width
            width = wmax;
        } else {
            width = max(width, wmin);
            width = min(width, wmax);
        }

        if (height == 0) {
            // Whole sensor height
            height = hmax;
        } else {
            height = max(height, hmin);
            height = min(height, hmax);
        }

        // Apply width and height settings
        arv_camera_set_region(m_camera, 0, 0, width, height, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_region failed: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        // Get x and y bounds
        gint xmin, xmax, ymin, ymax;
        arv_camera_get_x_offset_bounds(m_camera, &xmin, &xmax, &error);
        if (error == nullptr) {
            arv_camera_get_y_offset_bounds(m_camera, &ymin, &ymax, &error);
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << "Could not get ROI bounds: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        x = max(x, xmin);
        x = min(x, xmax);
        y = max(y, ymin);
        y = min(y, ymax);

        // Finally x and y offsets too
        arv_camera_set_region(m_camera, x, y, width, height, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_region failed: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        m_width = width;
        m_height = height;
        return true; // success
    }


    bool AravisCamera::set_binning(int& bin_x, int& bin_y) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get bounds
        gint xmin, xmax, ymin, ymax;
        arv_camera_get_x_binning_bounds(m_camera, &xmin, &xmax, &error);
        if (error == nullptr) {
            arv_camera_get_y_binning_bounds(m_camera, &ymin, &ymax, &error);
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not get binning bounds: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        // Apply bounds
        bin_x = max(bin_x, xmin);
        bin_x = min(bin_x, xmax);
        bin_y = max(bin_y, ymin);
        bin_y = min(bin_y, ymax);

        arv_camera_set_binning(m_camera, bin_x, bin_y, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_binning failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }


    bool AravisCamera::set_exposure_time(double& exposure_time) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get bounds
        double tmin, tmax;
        arv_camera_get_exposure_time_bounds(m_camera, &tmin, &tmax, &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_exposure_time_bounds failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        // Get increment
        double incr = G_MINDOUBLE;
        ArvGcNode* exptime_node = arv_device_get_feature(m_device, m_exposure_time_feature.c_str());
        if (ARV_IS_GC_INTEGER(exptime_node)) {
            incr = arv_camera_get_integer_increment(m_camera, m_exposure_time_feature.c_str(), &error);
        } else if (ARV_IS_GC_FLOAT(exptime_node)) {
            incr = arv_camera_get_float_increment(m_camera, m_exposure_time_feature.c_str(), &error);
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_xxx_increment failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        } else if (incr == G_MINDOUBLE) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": could not read exposure time increment";
            return false; // failure
        }

        if (incr > 0.) {
            // Exposure time must be incremented in steps.
            exposure_time = tmin + incr * round((exposure_time - tmin) / incr);
        }

        // Apply bounds
        exposure_time = max(exposure_time, tmin);
        exposure_time = min(exposure_time, tmax);

        arv_camera_set_exposure_time(m_camera, exposure_time, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_exposure_time failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }


    bool AravisCamera::set_frame_rate(bool enable, double frame_rate) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        if (enable) {
            // set frame rate

            if (frame_rate <= 0.) {
                // If no valid rate is provided, the one on the camera is re-applied
                frame_rate = arv_camera_get_frame_rate(m_camera, &error);
                if (error != nullptr) {
                    KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_frame_rate failed: " << error->message;
                    g_clear_error(&error);
                    return false; // failure
                }
            }

            // read the current trigger selector
            const std::string& triggerSelector = arv_device_get_string_feature_value(m_device, "TriggerSelector", &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_device_get_string_feature_value failed: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

            // read which triggers are "On" in order to restore them later
            guint n_triggers;
            const char** triggerSelectorOptions = arv_camera_dup_available_triggers(m_camera, &n_triggers, &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_dup_available_triggers failed: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

            std::vector<std::string> triggerOn;
            for (unsigned short i = 0; i < n_triggers; ++i) {
                arv_device_set_string_feature_value(m_device, "TriggerSelector", triggerSelectorOptions[i], &error);
                if (error != nullptr) break;

                const std::string mode(arv_device_get_string_feature_value(m_device, "TriggerMode", &error));
                if (error != nullptr) break;

                if (mode == "On") {
                    triggerOn.push_back(triggerSelectorOptions[i]);
                }
            }

            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not get TriggerModes: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

            // N.B. this function will set triggerMode to "Off" on all the selectors
            arv_camera_set_frame_rate(m_camera, frame_rate, &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_frame_rate failed: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

            // restore trigger modes
            for (const std::string& selector : triggerOn) {
                arv_device_set_string_feature_value(m_device, "TriggerSelector", selector.c_str(), &error);
                if (error != nullptr) break;

                arv_device_set_string_feature_value(m_device, "TriggerMode", "On", &error);
                if (error != nullptr) break;
            }

            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not restore TriggerModes: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

            // restore trigger selector
            arv_device_set_string_feature_value(m_device, "TriggerSelector", triggerSelector.c_str(), &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not restore TriggerSelector: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }

        } else { // enable == false
            arv_device_set_boolean_feature_value(m_device, "AcquisitionFrameRateEnable", false, &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not set AcquisitionFrameRateEnable: " << error->message;
                g_clear_error(&error);
                return false; // failure
            }
        }

        return true; // success
    }


    bool AravisCamera::get_gain(double& absGain, double& normGain) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get bounds
        double gmin, gmax;
        arv_camera_get_gain_bounds(m_camera, &gmin, &gmax, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_gain_bounds failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        } else if (gmin >= gmax) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": gmin >= gmax";
            return false;
        }

        const double _gain = arv_camera_get_gain(m_camera, &error); // raw gain
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_gain failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        absGain = _gain;
        normGain = (_gain - gmin) / (gmax - gmin); // normalized gain
        return true; // success
    }


    bool AravisCamera::set_gain(double& absGain, double& normGain, bool normalized) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get bounds
        double gmin, gmax;
        arv_camera_get_gain_bounds(m_camera, &gmin, &gmax, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_gain_bounds failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        if (normalized) { // Convert normalized to absolute gain
            if (normGain < 0) {
                normGain = 0;
                absGain = gmin;
            } else if (normGain < 1.) {
                absGain = gmin + normGain * (gmax - gmin);
            } else {
                normGain = 1;
                absGain = gmax;
            }
        } else {
            if (absGain < gmin) {
                absGain = gmin;
                normGain = 0;
            } else if (absGain < gmax) {
                normGain = (absGain - gmin) / (gmax - gmin);
            } else {
                absGain = gmax;
                normGain = 1.;
            }
        }

        arv_camera_set_gain(m_camera, absGain, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_gain failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }


    bool AravisCamera::set_frame_count(gint64& frame_count) {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get bounds
        gint64 fmin, fmax;
        arv_camera_get_frame_count_bounds(m_camera, &fmin, &fmax, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_get_frame_count_bounds failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        // Apply bounds
        frame_count = max(frame_count, fmin);
        frame_count = min(frame_count, fmax);

        arv_camera_set_frame_count(m_camera, frame_count, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_frame_count failed: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }


    void AravisCamera::configure(karabo::util::Hash& configuration) {
        if (m_camera == nullptr) {
            // cannot configure camera, as we are not connected
            return;
        }

        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();

        if (configuration.has("packetDelay")) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_gv_set_packet_delay(m_camera, configuration.get<long long>("packetDelay"), &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_gv_set_packet_delay failed: " << error->message;
                configuration.erase("packetDelay");
                g_clear_error(&error);
            }
        }

        const bool autoPacketSize = GET_PATH(configuration, "autoPacketSize", bool);
        if (autoPacketSize) {
            const bool success = this->set_auto_packet_size();
            if (!success && configuration.has("autoPacketSize")) {
                configuration.erase("autoPacketSize");
            }
        } else {
            try {
                const guint packetSize = GET_PATH(configuration, "packetSize", int);
                boost::mutex::scoped_lock camera_lock(m_camera_mtx);
                arv_camera_gv_set_packet_size(m_camera, packetSize, &error);
                if (error != nullptr) {
                    KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_gv_set_packet_size failed: " << error->message;
                    if (configuration.has("packetSize")) configuration.erase("packetSize");
                    g_clear_error(&error);
                }
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
            }
        }

        if (configuration.has("rotation")) {
            // Rotation is done on software, thus nothing is set to the camera.
            // Still, schema needs to be updated if rotation is changed by +- 90 degrees.
            const int change = configuration.get<unsigned int>("rotation") - this->get<unsigned int>("rotation");
            if (change % 180 != 0) {
                m_need_schema_update = true;
            }
        }

        if (configuration.has("pixelFormat")) {
            const char* pixelFormat = configuration.get<std::string>("pixelFormat").c_str();
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_set_pixel_format_from_string(m_camera, pixelFormat, &error);
            if (error == nullptr) m_format = arv_camera_get_pixel_format(m_camera, &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": cold not set pixel format to "
                    << pixelFormat << ": " << error->message;
                m_format = 0;
                configuration.erase("pixelFormat");
                g_clear_error(&error);
            }
            m_need_schema_update = true; // Schema update is needes as data type changed
        }

        if (configuration.has("bin") && m_is_binning_available) {
            int bin_x = GET_PATH(configuration, "bin.x", int);
            int bin_y = GET_PATH(configuration, "bin.y", int);

            const bool success = this->set_binning(bin_x, bin_y);
            if (success) { // update values
                configuration.set("bin.x", bin_x);
                configuration.set("bin.y", bin_y);
            } else {
                configuration.erase("bin");
            }
            m_need_schema_update = true; // Schema update is needed as image shape changed
        }

        // The ROI must be applied after binning, as the values for the former
        // are after applying the latter.
        if (configuration.has("roi")) {
            int x = GET_PATH(configuration, "roi.x", int);
            int y = GET_PATH(configuration, "roi.y", int);
            int width = GET_PATH(configuration, "roi.width", int);
            int height = GET_PATH(configuration, "roi.height", int);

            const bool success = this->set_region(x, y, width, height);
            if (success) { // update values
                configuration.set("roi.x", x);
                configuration.set("roi.y", y);
                configuration.set("roi.width", width);
                configuration.set("roi.height", height);
            } else {
                configuration.erase("roi");
            }
            m_need_schema_update = true; // Schema update is needed as image shape changed
        }

        if (configuration.has("exposureTime") && m_is_exposure_time_available) {
            double exposureTime = configuration.get<double>("exposureTime");

            const bool success = this->set_exposure_time(exposureTime);
            if (success) { // update the value
                configuration.set("exposureTime", exposureTime);
            } else {
                configuration.erase("exposureTime");
            }
        }

        if (configuration.has("frameRate") && m_is_frame_rate_available) {
            const bool enable = GET_PATH(configuration, "frameRate.enable", bool);
            double frameRate;
            try {
                frameRate = GET_PATH(configuration, "frameRate.target", float);
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
                frameRate = -1; // i.e. read from camera
            }

            bool success = this->set_frame_rate(enable, frameRate);
            if (!success) {
                configuration.erase("frameRate");
            }
        }

        if (m_arv_camera_trigger) {
            // trigger properties can be accessed with the arv_camera interface
            Result success;

            if (configuration.has("triggerSelector")) {
                std::string triggerSelector = configuration.get<std::string>("triggerSelector");
                success = this->setStringFeature("TriggerSelector", triggerSelector);
                if (success != Result::SUCCESS) {
                    configuration.erase("triggerSelector");
                }
                m_need_schema_update = true; // Schema update is needed as trigger mode, source and activation must be updated.
            }

            if (configuration.has("triggerMode")) {
                std::string triggerMode = configuration.get<std::string>("triggerMode");
                success = this->setStringFeature("TriggerMode", triggerMode);
                if (success != Result::SUCCESS) {
                    configuration.erase("triggerMode");
                }
            }

            if (configuration.has("triggerSource")) {
                std::string triggerSource = configuration.get<std::string>("triggerSource");
                success = this->setStringFeature("TriggerSource", triggerSource);
                if (success != Result::SUCCESS) {
                    configuration.erase("triggerSource");
                }
            }

            if (configuration.has("triggerActivation")) {
                std::string triggerActivation = configuration.get<std::string>("triggerActivation");
                success = this->setStringFeature("TriggerActivation", triggerActivation);
                if (success != Result::SUCCESS) {
                    configuration.erase("triggerActivation");
                }
            }
        }

        if (configuration.has("autoGain") && m_is_gain_auto_available) {
            const std::string& autoGainStr = configuration.get<std::string>("autoGain");
            const ArvAuto autoGain = arv_auto_from_string(autoGainStr.c_str());
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_set_gain_auto(m_camera, autoGain, &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_gain_auto failed: " << error->message;
                configuration.erase("autoGain");
                g_clear_error(&error);
            }
        }

        if (configuration.has("gain") && m_is_gain_available) {
            double absGain = configuration.get<double>("gain");
            double normGain = configuration.get<double>("gain");
            const bool isNormalized = this->get<bool>("isNormGain");
            const bool success = this->set_gain(absGain, normGain, isNormalized);

            if (success) { // update the values
                if (isNormalized) {
                    configuration.set("gain", normGain);
                } else {
                    configuration.set("gain", absGain);
                }
                configuration.set("absGain", absGain);
            } else {
                configuration.erase("gain");
            }
        }

        if (configuration.has("acquisitionMode")) {
            const std::string& acquisitionMode = configuration.get<std::string>("acquisitionMode");
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_set_acquisition_mode(m_camera, arv_acquisition_mode_from_string(acquisitionMode.c_str()), &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_set_acquisition_mode failed: " << error->message;
                configuration.erase("acquisitionMode");
                g_clear_error(&error);
            }
        }

        if (configuration.has("frameCount") && m_is_frame_count_available) {
            gint64 frameCount = configuration.get<long long>("frameCount");

            const bool success = this->set_frame_count(frameCount);
            if (success) { // update value
                configuration.set("frameCount", frameCount);
            } else {
                configuration.erase("frameCount");
            }
        }

        // Filter configuration by tag "genicam" and loop over it
        // XXX possibly need a tag "update_schema" for parameter changing image "size"
        const Hash filtered = this->filterByTags(configuration, "genicam");
        const Schema schema = this->getFullSchema();
        std::vector<std::string> paths;
        filtered.getPaths(paths);
        for (const auto& key : paths) {
            Result success = Result::FAIL;
            const auto feature = this->getAliasFromKey<std::string>(key);
            const auto valueType = this->getValueType(key);
            const auto accessMode = schema.getAccessMode(key);
            bool boolValue;
            long long intValue;
            double floatValue;
            std::string stringValue;

            if (accessMode == AccessType::READ) {
                // Read-Only parameter
                continue;
            }

            switch(valueType) {
                case Types::BOOL:
                    boolValue = configuration.get<bool>(key);
                    success = this->setBoolFeature(feature, boolValue);
                    configuration.set<bool>(key, boolValue); // set read value
                    break;
                case Types::STRING:
                    stringValue = configuration.get<std::string>(key);
                    success = this->setStringFeature(feature, stringValue);
                    configuration.set<std::string>(key, stringValue); // set read value
                    break;
                case Types::INT32:
                    intValue = configuration.get<int>(key);
                    success = this->setIntFeature(feature, intValue);
                    configuration.set<int>(key, intValue); // set read value
                    break;
                case Types::INT64:
                    intValue = configuration.get<long long>(key);
                    success = this->setIntFeature(feature, intValue);
                    configuration.set<long long>(key, intValue); // set read value
                    break;
                case Types::FLOAT:
                    floatValue = configuration.get<float>(key);
                    success = this->setFloatFeature(feature, floatValue);
                    configuration.set<float>(key, floatValue); // set read value
                    break;
                case Types::DOUBLE:
                    floatValue = configuration.get<double>(key);
                    success = this->setFloatFeature(feature, floatValue);
                    configuration.set<double>(key, floatValue); // set read value
                    break;
                default:
                    throw KARABO_NOT_IMPLEMENTED_EXCEPTION(key + " datatype not available in GenICam");
            }

            if (success == Result::FAIL) {
                const std::string message("Setting value for " + key + " may not have been successful");
                KARABO_LOG_WARN << message << ". Value on device updated according to camera.";
                this->set("status", message);
            }
        }

    }


    bool AravisCamera::synchronize_timestamp() {
        // If the camera can provide HW timestamping, then synchronize it with the timeserver
        return true;
    }


    bool AravisCamera::configure_timestamp_chunk() {
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // By default chunk mode is disabled.
        // It can be enabled in the derived class, if the camera provides HW timestamping.
        arv_camera_set_chunk_mode(m_camera, false, nullptr);
        m_chunk_mode = false;

        return true;
    }


    bool AravisCamera::get_region(gint& x, gint& y, gint& width, gint& height) {
        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        arv_camera_get_region(m_camera, &x, &y, &width, &height, &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_WARN << this->getInstanceId() << ": arv_camera_get_region failed: " << error->message;
            m_width = 0;
            m_height = 0;
            g_clear_error(&error);
            return false; // failure
        }

        m_width = width;
        m_height = height;
        return true; // success
    }


    bool AravisCamera::get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) {
        // If the camera provides HW timestamping in chunk data, this function shall be overridden
        return false;
    }


    bool AravisCamera::is_flip_x_available() const {
        // If the camera provides horizontal flip, this function shall be overridden.
        // Also, "genicam" tag and alias shall be provided for "flip.X".
        return false;
    }


    bool AravisCamera::is_flip_y_available() const {
        // If the camera provides vertical flip, this function shall be overridden.
        // Also, "genicam" tag and alias shall be provided for "flip.Y".
        return false;
    }


    void AravisCamera::acquire() {
        GError* error = nullptr;

        m_timer.now();
        m_counter = 0;
        m_sum_latency = 0.;

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            boost::mutex::scoped_lock stream_lock(m_stream_mtx);
            m_stream = arv_camera_create_stream(m_camera, AravisCamera::stream_cb, static_cast<void*>(this), &error);

            if (error != nullptr) {
                std::stringstream ss;
                ss << "arv_camera_create_stream failed: " << error->message;
                this->acquire_failed_helper(ss.str());
                g_clear_error(&error);
                return;
            }

            // Enable emission of signals (it's disabled by default for performance reason)
            arv_stream_set_emit_signals(m_stream, TRUE);

            // Create and push buffers to the stream
            const gint payload = arv_camera_get_payload(m_camera, &error);

            if (error != nullptr) {
                std::stringstream ss;
                ss << "arv_camera_get_payload failed: " << error->message;
                this->acquire_failed_helper(ss.str());
                g_clear_error(&error);
                return;
            }

            for (size_t i = 0; i < 10; i++) {
                arv_stream_push_buffer(m_stream, arv_buffer_new(payload, NULL));
            }
        }

        // Synchronize timestamp.
        // This will be repeated periodically during acquisition
        this->synchronize_timestamp();

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_start_acquisition(m_camera, &error);
            if (error != nullptr) {
                std::stringstream ss;
                ss << "arv_camera_start_acquisition failed: " << error->message;
                this->acquire_failed_helper(ss.str());
                g_clear_error(&error);
                return;
            }
        }

        {
            boost::mutex::scoped_lock stream_lock(m_stream_mtx);
            // Connect the 'new-buffer' signal
            g_signal_connect(m_stream, "new-buffer", G_CALLBACK(AravisCamera::new_buffer_cb), static_cast<void*>(this));
        }

        m_is_acquiring = true;
        this->set("status", "Acquisition started");
        this->updateState(State::ACQUIRING);
    }


    void AravisCamera::acquire_failed_helper(const std::string& detailed_msg) {
        const std::string message("Could not start acquisition");

        KARABO_LOG_ERROR << message;
        KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": " << detailed_msg;
        this->set("status", message);
        this->updateState(State::ERROR);
    }


    void AravisCamera::stop() {
        Hash h;
        h.set("frameRate.actual", 0.);
        h.set("errorCount", 0ull);
        h.set("lastError", "");
        h.set("latency.mean", 0.);
        h.set("latency.min", 0.);
        h.set("latency.max", 0.);

        GError* error = nullptr;
        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_stop_acquisition(m_camera, &error);
        }
        m_is_acquiring = false;
        m_errorCount = 0;
        m_lastError = ARV_BUFFER_STATUS_SUCCESS;

        if (error != nullptr) {
            const std::string message("Could not stop acquisition");
            KARABO_LOG_ERROR << message;
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_camera_stop_acquisition failed: " << error->message;
            g_clear_error(&error);
            h.set("status", message);
            this->set(h);
            this->updateState(State::ERROR);
            return;
        }

        this->clear_stream();

        h.set("status", "Acquisition stopped");
        this->signalEOS(); // End-of-Stream signal
        this->set(h);
        this->updateState(State::ON);
    }


    void AravisCamera::trigger() {
        if (!m_arv_camera_trigger) {
            return;
        }

        GError* error = nullptr;

        const std::string& triggerMode = this->get<std::string>("triggerMode");
        if (triggerMode == "On") {
            const std::string& triggerSource = this->get<std::string>("triggerSource");
            if (triggerSource == "Software") {
                boost::mutex::scoped_lock camera_lock(m_camera_mtx);
                arv_camera_software_trigger(m_camera, &error);
                if (error != nullptr) {
                    KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": arv_camera_software_trigger failed: " << error->message;
                    g_clear_error(&error);
                }
            }
        }
    }


    void AravisCamera::refresh() {
        // Poll parameters and update options
        const bool success = this->updateOutputSchema();
        if (!success) {
            this->updateState(State::ERROR);
        }
    }


    void AravisCamera::reset() {
        if (!m_connect) {
            // Connection task has been stopped
            this->updateState(State::UNKNOWN);
            this->set("status", "");
            m_connect = true;
            m_reconnect_timer.expires_from_now(boost::posix_time::milliseconds(1));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        }

        // Poll parameters and update options
        const bool success = this->updateOutputSchema();
        if (success) {
            this->updateState(State::ON);
        }
    }


    void AravisCamera::resetCamera() {
        // To be implemented in the derived class, if the feature is available.
    }


    void AravisCamera::clear_camera() {
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);
        g_clear_object(&m_camera);
        m_device = nullptr; // Has been clearead by clearing m_camera
        g_clear_object(&m_parser);
    }


    void AravisCamera::clear_stream() {
        if (m_stream != nullptr) {
            // TODO possibly disconnect signal, see https://developer.gnome.org/gobject/stable/gobject-Signals.html#g-signal-handler-disconnect

            // Disable emission of signals and free resource
            boost::mutex::scoped_lock stream_lock(m_stream_mtx);
            arv_stream_set_emit_signals(m_stream, FALSE);
            g_clear_object(&m_stream);
        }
    }


    void AravisCamera::stream_cb(void *context, ArvStreamCallbackType type, ArvBuffer *buffer) {
        Self* self = static_cast<Self*>(context);
        const std::string& deviceId = self->getInstanceId();

        if (type == ARV_STREAM_CALLBACK_TYPE_INIT) {
            KARABO_LOG_FRAMEWORK_DEBUG << deviceId << ": Init stream";
                if (!arv_make_thread_realtime(10) && !arv_make_thread_high_priority(-10)) {
                    KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Failed to make stream thread high priority";
                }
        }
    }


    void AravisCamera::new_buffer_cb(ArvStream* stream, void* context) {
        Self* self = static_cast<Self*>(context);
        boost::mutex::scoped_lock stream_lock(self->m_stream_mtx);

        const karabo::util::Timestamp dev_ts = self->getActualTimestamp();
        const std::string& deviceId = self->getInstanceId();

        ArvBuffer* arv_buffer = arv_stream_pop_buffer(stream);
        if (arv_buffer == nullptr) {
            return;
        }

        ArvBufferStatus lastError = arv_buffer_get_status(arv_buffer);
        if (lastError == ARV_BUFFER_STATUS_SUCCESS) {
            gint width = self->m_width;
            gint height = self->m_height;
            size_t buffer_size;
            ArvPixelFormat pixel_format = self->m_format;

            const void* buffer_data = arv_buffer_get_data(arv_buffer, &buffer_size);

            karabo::util::Timestamp ts;
            if (self->get_timestamp(arv_buffer, ts)) {
                // Latency between the image timestamp and the reception time
                const double latency = dev_ts.getEpochstamp() - ts.getEpochstamp();
                if (self->m_counter == 0) {
                    self->m_min_latency = latency;
                    self->m_max_latency = latency;
                    self->m_sum_latency = latency;
                } else {
                    self->m_min_latency = std::min(latency, self->m_min_latency);
                    self->m_max_latency = std::max(latency, self->m_max_latency);
                    self->m_sum_latency += latency;
                }
            } else {
                // HW timestamp not available: use actual one from device
                ts = dev_ts;
            }

            switch(pixel_format) {
                case ARV_PIXEL_FORMAT_MONO_8:
                    self->writeOutputChannels<unsigned char>(buffer_data, width, height, ts);
                    break;
                case ARV_PIXEL_FORMAT_MONO_10:
                case ARV_PIXEL_FORMAT_MONO_12:
                case ARV_PIXEL_FORMAT_MONO_14:
                case ARV_PIXEL_FORMAT_MONO_16:
                    self->writeOutputChannels<unsigned short>(buffer_data, width, height, ts);
                    break;
                case ARV_PIXEL_FORMAT_MONO_10_PACKED:
                case ARV_PIXEL_FORMAT_MONO_12_PACKED:
                {
                    const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer_data);
                    uint16_t* unpackedData = new uint16_t[width * height];
                    unpackMono12Packed(data, width, height, unpackedData);
                    self->writeOutputChannels<unsigned short>(unpackedData, width, height, ts);
                    delete[] unpackedData;
                }
                    break;
                case ARV_PIXEL_FORMAT_MONO_10_P:
                {
                    const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer_data);
                    uint16_t* unpackedData = new uint16_t[width * height];
                    unpackMono10p(data, width, height, unpackedData);
                    self->writeOutputChannels<unsigned short>(unpackedData, width, height, ts);
                    delete[] unpackedData;
                }
                    break;
               case ARV_PIXEL_FORMAT_MONO_12_P:
                {
                    const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer_data);
                    uint16_t* unpackedData = new uint16_t[width * height];
                    unpackMono12p(data, width, height, unpackedData);
                    self->writeOutputChannels<unsigned short>(unpackedData, width, height, ts);
                    delete[] unpackedData;
                }
                    break;
                // TODO RGB, YUV...
                default:
                    if (self->m_pixelFormatOptions.find(pixel_format) != self->m_pixelFormatOptions.end()) {
                        KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Format " << self->m_pixelFormatOptions[pixel_format]
                            << " is not yet supported";
                    } else {
                        KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Format " << pixel_format << " is not yet supported";
                    }

                    if (self->getState() == State::ACQUIRING) {
                        self->execute("stop");
                    }
            }

            self->m_counter += 1;

        } else {
            self->m_errorCount += 1;
            self->m_lastError = lastError;
        }

        if (self->m_timer.elapsed() >= 1.) {
            // Update frame rate and error count
            self->updateFrameRate();

            // Synchronize camera timestamp with timeserver.
            // This shall be repetead regularly to correct for drift.
            self->synchronize_timestamp();

            self->m_timer.now();
            self->m_counter = 0;
        }

        // Push back the buffer to the stream
        arv_stream_push_buffer(stream, arv_buffer);
    }


    void AravisCamera::control_lost_cb(ArvGvDevice* gv_device, void* context) {
        // Control of the device is lost

        Self* self = static_cast<Self*>(context);

        // TODO what happens with multiple cameras on server?
        // Possibly use arv_gv_device_get_device_address (gv_device) to verify IP address

        const std::string message("Control of the camera " + self->get<std::string>("cameraId") + " is lost");
        KARABO_LOG_FRAMEWORK_WARN << self->getInstanceId() << ": " << message;
        // NOTE calling here g_object_clear(m_camera) would seg fault.
        self->m_is_connected = false;

        self->set("status", message);
        self->updateState(State::UNKNOWN);
    }


    void AravisCamera::pollOnce(karabo::util::Hash& h) {
        bool success;
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();

        const long long packetDelay = arv_camera_gv_get_packet_delay(m_camera, &error);
        if (error == nullptr) {
            h.set("packetDelay", packetDelay);
        } else {
            KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_gv_get_packet_delay failed: " << error->message;
            g_clear_error(&error);
        }

        const guint packetSize = arv_camera_gv_get_packet_size(m_camera, &error);
        if (error == nullptr) {
            h.set("packetSize", packetSize);
        } else {
            KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_gv_get_packet_size failed: " << error->message;
            g_clear_error(&error);
        }

        gint x, y, width, height;
        success = this->get_region(x, y, width, height);
        if (success) {
            h.set("roi.x", x);
            h.set("roi.y", y);
            h.set("roi.width", width);
            h.set("roi.height", height);
        }

        if (m_is_binning_available) {
            gint dx, dy;
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_get_binning(m_camera, &dx, &dy, &error);
            if (error == nullptr) {
                h.set("bin.x", dx);
                h.set("bin.y", dy);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_binning failed: " << error->message;
                g_clear_error(&error);
            }
        }

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const std::string pixelFormat = arv_camera_get_pixel_format_as_string(m_camera, &error);
            if (error == nullptr) m_format = arv_camera_get_pixel_format(m_camera, &error);
            if (error == nullptr) {
                h.set("pixelFormat", pixelFormat);
            } else {
                m_format = 0;
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_pixel_format* failed: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_is_exposure_time_available) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const double exposureTime = arv_camera_get_exposure_time(m_camera, &error);
            if (error == nullptr) {
                h.set("exposureTime", exposureTime);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_exposure_time failed: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_arv_camera_trigger) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const std::string triggerSelector = arv_device_get_string_feature_value(m_device, "TriggerSelector", &error);
            if (error == nullptr) {
                h.set("triggerSelector", triggerSelector);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not get TriggerSelector: " << error->message;
                g_clear_error(&error);
            }

            const std::string triggerMode = arv_device_get_string_feature_value(m_device, "TriggerMode", &error);
            if (error == nullptr) {
                h.set("triggerMode", triggerMode);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not get TriggerMode: " << error->message;
                g_clear_error(&error);
            }

            // Under certain circumstances, nullptr is returned for "TriggerSource"
            const char* triggerSourcePtr = arv_device_get_string_feature_value(m_device, "TriggerSource", &error);
            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not get TriggerSource: " << error->message;
                g_clear_error(&error);
            } else if (triggerSourcePtr != nullptr) {
                h.set("triggerSource", triggerSourcePtr);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": cannot get trigger sources from the camera";

                // Ensure that Karabo device and camera are in sync
                const std::string& triggerSource = this->get<std::string>("triggerSource");
                arv_device_set_string_feature_value(m_device, "TriggerSource", triggerSource.c_str(), &error);
                if (error != nullptr) {
                    KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not set TriggerSource: " << error->message;
                    g_clear_error(&error);
                }
            }

            const std::string triggerActivation = arv_device_get_string_feature_value(m_device, "TriggerActivation", &error);
            if (error == nullptr) {
                h.set("triggerActivation", triggerActivation);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not get TriggerActivation: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_is_frame_rate_available) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const double frameRate = arv_camera_get_frame_rate(m_camera, &error);
            if (error == nullptr) {
                h.set("frameRate.target", frameRate);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_frame_rate failed: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_is_gain_auto_available) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const ArvAuto autoGain = arv_camera_get_gain_auto(m_camera, &error);
            if (error == nullptr) {
                const std::string autoGainStr(arv_auto_to_string(autoGain));
                h.set("autoGain", autoGainStr);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_gain_auto failed: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_is_gain_available) {
            const bool isNormalized = this->get<bool>("isNormGain");

            double absGain, normGain;
            success = this->get_gain(absGain, normGain);
            if (success) {
                h.set("absGain", absGain);
                h.set("gain", isNormalized?normGain:absGain);
            }
        }

        {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const ArvAcquisitionMode acquisitionMode = arv_camera_get_acquisition_mode(m_camera, &error);
            if (error == nullptr) {
                const std::string acquisitionModeStr(arv_acquisition_mode_to_string(acquisitionMode));
                h.set("acquisitionMode", acquisitionModeStr);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_acquisition_mode failed: " << error->message;
                g_clear_error(&error);
            }
        }

        if (m_is_frame_count_available) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const long long frameCount = arv_camera_get_frame_count(m_camera, &error);
            if (error == nullptr) {
                h.set("frameCount", frameCount);
            } else {
                KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_frame_count failed: " << error->message;
                g_clear_error(&error);
            }
        }

        // Filter paths by tag "genicam" and poll features
        std::vector<std::string> paths;
        this->getPathsByTag(paths, "genicam");
        this->pollGenicamFeatures(paths, h);

    }


    void AravisCamera::pollCamera(const boost::system::error_code & ec) {
        if (ec == boost::asio::error::operation_aborted) return;

        if (!m_is_connected) {
            // Not connected
            m_poll_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_poll_timer.async_wait(karabo::util::bind_weak(&AravisCamera::pollCamera, this, boost::asio::placeholders::error));
            return;
        }

        // Filter paths by tag "poll" and poll features
        std::vector<std::string> paths;
        this->getPathsByTag(paths, "poll");
        Hash h;
        this->pollGenicamFeatures(paths, h);

        this->set(h);

        const int pollingInterval = this->get<int>("pollingInterval");
        m_poll_timer.expires_from_now(boost::posix_time::seconds(pollingInterval));
        m_poll_timer.async_wait(karabo::util::bind_weak(&AravisCamera::pollCamera, this, boost::asio::placeholders::error));
    }


    void AravisCamera::pollGenicamFeatures(const std::vector<std::string>& paths, karabo::util::Hash& h) {
        for (const auto& key : paths) {
            const auto feature = this->getAliasFromKey<std::string>(key);
            const auto valueType = this->getValueType(key);
            bool boolValue;
            long long intValue;
            double doubleValue;
            std::string stringValue;
            switch(valueType) {
                case Types::BOOL:
                    if (this->getBoolFeature(feature, boolValue) == Result::SUCCESS) {
                        h.set(key, boolValue);
                    }
                    break;
                case Types::STRING:
                    if (this->getStringFeature(feature, stringValue) == Result::SUCCESS) {
                        h.set(key, stringValue);
                    }
                    break;
                case Types::INT32:
                case Types::INT64:
                    if (this->getIntFeature(feature, intValue) == Result::SUCCESS) {
                        h.set(key, intValue);
                    }
                    break;
                case Types::FLOAT:
                case Types::DOUBLE:
                    if (this->getFloatFeature(feature, doubleValue) == Result::SUCCESS) {
                        h.set(key, doubleValue);
                    }
                    break;
                default:
                    throw KARABO_NOT_IMPLEMENTED_EXCEPTION(key + " datatype not available in GenICam");
            }
        }
    }

    bool AravisCamera::updateOutputSchema() {
        if (m_camera == nullptr || !m_need_schema_update) {
            // cannot query camera, as we are not connected
            // OR no schema update is needed
            return true;
        }

        Hash h;
        this->pollOnce(h);

        const unsigned long long height = h.get<int>("roi.height");
        const unsigned long long width = h.get<int>("roi.width");
        const unsigned int rotation = this->get<unsigned int>("rotation");
        std::vector<unsigned long long> shape;
        switch (rotation) {
            case 90:
            case 270:
                shape = {width, height};
                break;
            default:
                shape = {height, width};
        }

        GError* error = nullptr;
        const std::string errorMsg("Could not update output schema");
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        Types::ReferenceType kType;
        switch(m_format) {
            case ARV_PIXEL_FORMAT_MONO_8:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT8;
                break;
            case ARV_PIXEL_FORMAT_MONO_10:
            case ARV_PIXEL_FORMAT_MONO_10_PACKED:
            case ARV_PIXEL_FORMAT_MONO_10_P:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_MONO_12:
            case ARV_PIXEL_FORMAT_MONO_12_PACKED:
            case ARV_PIXEL_FORMAT_MONO_12_P:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_MONO_14:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_MONO_16:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_RGB_8_PACKED:
            case ARV_PIXEL_FORMAT_RGB_8_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT8;
                break;
            case ARV_PIXEL_FORMAT_RGB_10_PACKED:
            case ARV_PIXEL_FORMAT_RGB_10_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_RGB_12_PACKED:
            case ARV_PIXEL_FORMAT_RGB_12_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                break;
            case ARV_PIXEL_FORMAT_RGB_16_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                break;
            // TODO: YUV
            default:
                m_encoding = Encoding::GRAY;
                kType = Types::UNKNOWN;
                break;
        }

        const unsigned short bpp = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(m_format);
        h.set("bpp", bpp);

        this->set(h);
        CameraImageSource::updateOutputSchema(shape, m_encoding, kType);

        guint n_int_values, n_str_values;
        gint64* int_options;
        const char** str_options;
        Schema schemaUpdate = this->getFullSchema();
        Hash parameterHash = schemaUpdate.getParameterHash(); // Copy

        // get available pixel formats
        int_options = arv_camera_dup_available_pixel_formats(m_camera, &n_int_values, &error);
        if (error != nullptr) {
            KARABO_LOG_ERROR << errorMsg;
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_dup_available_pixel_formats failed: " << error->message;
            g_clear_error(&error);
            this->set("status", errorMsg);
            return false; // failure
        }

        str_options = arv_camera_dup_available_pixel_formats_as_strings(m_camera, &n_str_values, &error);
        if (error != nullptr) {
            KARABO_LOG_ERROR << errorMsg;
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ":arv_camera_dup_available_pixel_formats_as_strings failed: " << error->message;
            g_clear_error(&error);
            this->set("status", errorMsg);
            return false; // failure
        }

        if (n_int_values == n_str_values) {
            // fill-up the pixel_format_options map
            for (unsigned short i = 0; i < n_int_values; ++i) {
                m_pixelFormatOptions[int_options[i]] = str_options[i];
            }
        } else {
            KARABO_LOG_FRAMEWORK_WARN << deviceId << ": Could not fill-up pixel_format_options map: different number of "
                << "int and string options.";
        }
        g_free(int_options);

        const std::vector<std::string> pixelFormatOptions(str_options,
            str_options + n_str_values);
        g_free(str_options);
        OVERWRITE_ELEMENT(schemaUpdate).key("pixelFormat")
            .setNewOptions(pixelFormatOptions)
            .commit();

        if (m_is_device_reset_available) {
            // Make "resetCamera" slot visible in the GUI
            SLOT_ELEMENT(schemaUpdate).key("resetCamera")
                    .displayedName("Reset Camera")
                    .description("'Hardware' reset, i.e. send a 'reset' command to the camera.")
                    .allowedStates(State::ERROR, State::ON)
                    .commit();
        }

        if (m_arv_camera_trigger) {
            // get available trigger selectors
            str_options = arv_camera_dup_available_triggers(m_camera, &n_str_values, &error);
            if (error != nullptr) {
                KARABO_LOG_ERROR << errorMsg;
                KARABO_LOG_FRAMEWORK_ERROR << deviceId<< ": arv_camera_dup_available_triggers failed: " << error->message;
                g_clear_error(&error);
                this->set("status", errorMsg);
                return false; // failure
            }

            const std::vector<std::string> triggerSelectorOptions(str_options,
                str_options + n_str_values);
            g_free(str_options);
            OVERWRITE_ELEMENT(schemaUpdate).key("triggerSelector")
                .setNewOptions(triggerSelectorOptions)
                .commit();

            // get available trigger sources
            str_options = arv_camera_dup_available_trigger_sources(m_camera, &n_str_values, &error);
            if (error != nullptr) {
                KARABO_LOG_ERROR << errorMsg;
                KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": arv_camera_dup_available_trigger_sources failed: " << error->message;
                g_clear_error(&error);
                this->set("status", errorMsg);
                return false; // failure
            }

            std::vector<std::string> triggerSourceOptions(str_options,
                str_options + n_str_values);
            g_free(str_options);

            if (n_str_values == 0) {
                KARABO_LOG_FRAMEWORK_WARN << deviceId
                    << ": could not get available trigger sources from camera. "
                    << "Using defaults.";
                triggerSourceOptions = {"Software", "Line1"};
            }

            OVERWRITE_ELEMENT(schemaUpdate).key("triggerSource")
                .setNewOptions(triggerSourceOptions)
                .commit();

        }

        if (!m_is_binning_available) {
            this->disableElement("bin.x", schemaUpdate);
            this->disableElement("bin.y", schemaUpdate);
        }

        if (!m_is_exposure_time_available) {
            this->disableElement("exposureTime", schemaUpdate);
        }

        if (!m_is_frame_rate_available) {
            this->disableElement("frameRate.target", schemaUpdate);
        }

        const std::string vendor = arv_camera_get_vendor_name(m_camera, &error);
        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_WARN << deviceId << ": arv_camera_get_vendor_name failed: " << error->message;
            g_clear_error(&error);
        }
        if (vendor != "Basler") {
            // Only enable for Basler
            // XXX Check why this is needed...
            this->disableElement("frameRate.enable", schemaUpdate);
        }

        if (!m_is_gain_auto_available) {
            this->disableElement("autoGain", schemaUpdate);
        }

        if (!m_is_gain_available) {
            this->disableElement("gain", schemaUpdate);
        }

        if (!m_is_frame_count_available) {
            std::vector<std::string> acquisitionModeOptions = {
                "Continuous", "SingleFrame"};
            OVERWRITE_ELEMENT(schemaUpdate).key("acquisitionMode")
                .setNewOptions(acquisitionModeOptions)
                .commit();

            this->disableElement("frameCount", schemaUpdate);
        }

        if (!m_is_flip_x_available) {
            // Remove alias and tags so that flip is done on software
            OVERWRITE_ELEMENT(schemaUpdate).key("flip.X")
                .setNewAlias("")
                .setNewTags({})
                .commit();
            // To avoid the parameter is disabled in the following step.
            parameterHash.erase("flip.X");
        }

        if (!m_is_flip_y_available) {
            // Remove alias and tags so that flip is done on software
            // This must be done here to avoid the parameter is disabled
            // in the following step.
            OVERWRITE_ELEMENT(schemaUpdate).key("flip.Y")
                .setNewAlias("")
                .setNewTags({})
                .commit();
            // To avoid the parameter is disabled in the following step.
            parameterHash.erase("flip.Y");
        }

        // Disable features which are unavailable on the camera
        std::vector<std::string> paths;
        const Hash filteredParameters = this->filterByTags(parameterHash,
            "genicam,poll");
        filteredParameters.getPaths(paths);
        camera_lock.unlock(); // must unlock m_camera_mtx before calling isFeatureAvailable(
        for (const auto& key : paths) {
            const std::string feature = this->getAliasFromKey<std::string>(key);
            if (!this->keyHasAlias(key) || !this->isFeatureAvailable(feature)) {
                // This feature is not available on the camera
                this->disableElement(key, schemaUpdate);
            } else if (schemaUpdate.getValueType(key) == Types::STRING) {
                const char** str_options = arv_device_dup_available_enumeration_feature_values_as_strings(
                    m_device, feature.c_str(), &n_str_values, &error);
                if (error == nullptr) {
                    const std::vector<std::string> vec_options(str_options,
                        str_options + n_str_values);
                    OVERWRITE_ELEMENT(schemaUpdate).key(key)
                        .setNewOptions(vec_options)
                        .commit();
                } else {
                    KARABO_LOG_FRAMEWORK_ERROR << "arv_device_dup_available_enumeration_feature_values_as_strings failed: " << error->message;
                    g_clear_error(&error);
                }
                g_free(str_options);
            }
        }

        this->appendSchema(schemaUpdate);
        m_need_schema_update = false;
        return true; // success
    }


    template <class T>
    void AravisCamera::writeOutputChannels(const void* data, gint width, gint height,
            const karabo::util::Timestamp& ts) {
        const Dims shape(height, width);

        // Non-copy NDArray constructor
        karabo::util::NDArray imgArray((T*) data, width*height, karabo::util::NDArray::NullDeleter(), shape);

        const unsigned short bpp = this->get<unsigned short>("bpp");
        Dims binning(this->get<int>("bin.y"), this->get<int>("bin.x"));
        Dims roiOffsets(this->get<int>("roi.y"), this->get<int>("roi.x"));
        const Hash header;

        // Apply flip on software if not available on camera
        const bool flipX = this->get<bool>("flip.X") && !m_is_flip_x_available;
        const bool flipY = this->get<bool>("flip.Y") && !m_is_flip_y_available;
        if (flipX || flipY) {
            util::flip_image<T>(imgArray, flipX, flipY);
        }

        const unsigned int rotation = this->get<unsigned int>("rotation");
        switch (rotation) {
            case 90:
            case 270:
                util::rotate_image<T>(imgArray, rotation);
                binning.reverse();
                roiOffsets.reverse();
                break;
            case 180:
                util::rotate_image<T>(imgArray, rotation);
                break;
            default:
                break;
        }

        // Send image and metadata to output channel
        this->writeChannels(imgArray, binning, bpp, m_encoding, roiOffsets, ts, header);
    }


    void AravisCamera::updateFrameRate() {
        Hash h;

        if (m_counter > 0) { // Only update if available
            // Convert latency to ms
            h.set("latency.min", 1000. * m_min_latency);
            h.set("latency.max", 1000. * m_max_latency);
            h.set("latency.mean", 1000. * m_sum_latency / m_counter);
        }

        // Calculate frame rate
        const float frameRate = m_counter / m_timer.elapsed();
        h.set("frameRate.actual", frameRate);

        if (m_errorCount != this->get<unsigned long long>("errorCount")) {
            h.set("errorCount", m_errorCount);
            if (m_bufferStatus.find(m_lastError) != m_bufferStatus.end()) {
                const std::string& lastError = m_bufferStatus[m_lastError];
                if (lastError != this->get<std::string>("lastError")) {
                    h.set("lastError", lastError);
                }
            }
        }

        this->set(h);
    }


    bool AravisCamera::resolveHostname(const std::string& hostname, std::string&ip_address, std::string& message) {
        bool success = false;
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        const boost::asio::ip::tcp::resolver::query query(hostname, "");
        const boost::asio::ip::tcp::resolver::iterator end;
        boost::system::error_code ec;
        auto it = resolver.resolve(query, ec);
        if (ec != boost::system::errc::success) {
            ip_address = "";
            message = "Boost error in resolveHostname: " + ec.message();
        } else if (it != end) {
            const boost::asio::ip::tcp::endpoint endpoint = it->endpoint();
            success = true;
            ip_address = endpoint.address().to_string();
            message = "IP name resolved: " + hostname + " -> " + ip_address;
        } else {
            ip_address = "";
            message = "Cannot resolve hostname: " + hostname;
        }

        return success;
    }

}
