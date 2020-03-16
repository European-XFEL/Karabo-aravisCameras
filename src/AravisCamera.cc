/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include <boost/algorithm/string/trim.hpp>

#include "AravisCamera.hh"

using namespace std;

USING_KARABO_NAMESPACES;

#define GET_PATH(hash, path, type) hash.has(path) ? hash.get<type>(path) : this->get<type>(path);

namespace karabo {


    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera)

    void AravisCamera::expectedParameters(Schema& expected) {
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
                .description("Sets the 'absolute' value of the acquisition frame rate on the camera. "
                "The 'absolute' value is a float value that sets the acquisition frame rate in frames per second.")
                .assignmentOptional().noDefaultValue()
                .minExc(0.)
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
                .readOnly().initialValue("")
                .commit();

        STRING_ELEMENT(expected).key("model")
                .displayedName("Model Name")
                .readOnly().initialValue("")
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

        STRING_ELEMENT(expected).key("pixelFormat")
                .displayedName("Pixel Format")
                .description("This enumeration sets the format of the pixel data transmitted for acquired images. "
                "For example Mono8 means monochromatic, 8 bits-per-pixel.")
                .assignmentOptional().noDefaultValue()
                // options will be injected on connection
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
                .assignmentOptional().defaultValue(0.)
                .minInc(0.)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerMode")
                .displayedName("Trigger Mode")
                .description("This enumeration enables or disables the trigger. "
                "When this is set to 'On', the target frame rate parameter will be ignored.")
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

        DOUBLE_ELEMENT(expected).key("gain")
                .displayedName("Gain")
                .description("Sets the gain of the ADC converter.")
                .assignmentOptional().noDefaultValue()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
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

    }


    AravisCamera::AravisCamera(const karabo::util::Hash& config) : CameraImageSource(config),
            m_connect(true), m_reconnect_timer(EventLoop::getIOService()), m_failed_connections(0u),
            m_poll_timer(EventLoop::getIOService()),
            m_camera(NULL), m_stream(NULL) {
        KARABO_SLOT(acquire);
        KARABO_SLOT(stop);
        KARABO_SLOT(trigger);

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
        this->updateOutputSchema();
    }


    void AravisCamera::getPathsByTag(std::vector<std::string>& paths, const std::string& tags) {
        // N.B. Device::getCurrentConfiguration(tags)) cannot be used, as it
        // does not return parameters with no value set

        const Schema schema = this->getFullSchema();
        const Hash& parameters = schema.getParameterHash();
        const Hash filteredParameters = this->filterByTags(parameters, tags);

        filteredParameters.getPaths(paths);
    }


    bool AravisCamera::getBoolFeature(const std::string& feature, bool& value) {
        value = arv_device_get_boolean_feature_value(m_device, feature.c_str());
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::getStringFeature(const std::string& feature, std::string& value) {
        value = arv_device_get_string_feature_value(m_device, feature.c_str());
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::getIntFeature(const std::string& feature, long long& value) {
        value = arv_device_get_integer_feature_value(m_device, feature.c_str());
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::getFloatFeature(const std::string& feature, double& value) {
        value = arv_device_get_float_feature_value(m_device, feature.c_str());
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::setBoolFeature(const std::string& feature, bool value) {
        arv_device_set_boolean_feature_value(m_device, feature.c_str(), value);
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::setStringFeature(const std::string& feature, const std::string& value) {
        arv_device_set_string_feature_value(m_device, feature.c_str(), value.c_str());
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::setIntFeature(const std::string& feature, long long value) {
        arv_device_set_integer_feature_value(m_device, feature.c_str(), value);
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
    }


    bool AravisCamera::setFloatFeature(const std::string& feature, double value) {
        arv_device_set_float_feature_value(m_device, feature.c_str(), value);
        return (arv_device_get_status(m_device) == ARV_DEVICE_STATUS_SUCCESS);
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

        if (m_camera) {
            // Already connected
            m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        }

        const std::string& idType = this->get<std::string>("idType");
        const std::string& cameraId = this->get<std::string>("cameraId");
        std::string cameraIp;

        if (idType == "IP") { // IP address
            cameraIp = cameraId;

        } else if (idType == "HOST") { // IP name
            cameraIp = this->resolveHostname(cameraId);
            if (cameraIp.size() > 0 && m_failed_connections < 1) {
                KARABO_LOG_INFO << "IP name resolved: " << cameraId << " -> " <<  cameraIp;
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

            if (cameraIp.size() == 0 && m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Could not discover any camera with serial (" << cameraId << ")";
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

            if (cameraIp.size() == 0 &&  m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Could not discover any camera with MAC (" << cameraId << ")";
            }

        }

        if (cameraIp.size() == 0) {
            ++m_failed_connections;
            m_camera = NULL;
            m_device = NULL;

            m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        }

        this->clear_stream();
        this->clear_camera();

        m_camera = arv_camera_new(cameraIp.c_str());

        if (!ARV_IS_CAMERA(m_camera)) {
            if (m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Cannot connect to " << cameraIp;
            }
            ++m_failed_connections;
            m_camera = NULL;
            m_device = NULL;

            m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        }

        // ArvDevice gives more complete access to camera features
        m_device = arv_camera_get_device(m_camera);

        KARABO_LOG_INFO << "Connected to " << cameraIp;

        // Connect the control-lost signal
        g_signal_connect(arv_camera_get_device(m_camera), "control-lost", G_CALLBACK(AravisCamera::control_lost_cb), static_cast<void*>(this));

        Hash h;

        // Read immutable properties
        h.set("camId", std::string(arv_camera_get_device_id(m_camera)));
        h.set("vendor", std::string(arv_camera_get_vendor_name(m_camera)));
        h.set("model", std::string(arv_camera_get_model_name(m_camera)));

        gint width, height;
        arv_camera_get_sensor_size(m_camera, &width, &height);
        h.set("width", width);
        h.set("height", height);

        this->set(h);
        
        // Apply initial configuration
        Hash initialConfiguration = this->getCurrentConfiguration();
        this->configure(initialConfiguration);

        this->updateOutputSchema();

        this->updateState(State::ON);
        m_failed_connections = 0;
        m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
    }


    void AravisCamera::configure(karabo::util::Hash& configuration) {
        if (m_camera == NULL) {
            // cannot configure camera, as we are not connected
            return;
        }

        if (configuration.has("packetDelay")) {
            arv_camera_gv_set_packet_delay(m_camera, configuration.get<long long>("packetDelay"));
        }

        const bool autoPacketSize = GET_PATH(configuration, "autoPacketSize", bool);
        if (autoPacketSize) {
            const guint packetSize = arv_camera_gv_auto_packet_size(m_camera);
            arv_camera_gv_set_packet_size(m_camera, packetSize);
        } else {
            try {
                const guint packetSize = GET_PATH(configuration, "packetSize", int);
                arv_camera_gv_set_packet_size(m_camera, packetSize);
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
            }
        }

        if (configuration.has("pixelFormat")) {
            const char* pixelFormat = configuration.get<std::string>("pixelFormat").c_str();
            arv_camera_set_pixel_format_from_string(m_camera, pixelFormat);
        }

        if (configuration.has("roi")) {
            gint x = GET_PATH(configuration, "roi.x", int);
            gint y = GET_PATH(configuration, "roi.y", int);
            gint width = GET_PATH(configuration, "roi.width", int);
            gint height = GET_PATH(configuration, "roi.height", int);

            // Get bounds
            gint xmin, xmax, ymin, ymax, wmin, wmax, hmin, hmax;
            arv_camera_get_x_offset_bounds(m_camera, &xmin, &xmax);
            arv_camera_get_y_offset_bounds(m_camera, &ymin, &ymax);
            arv_camera_get_width_bounds(m_camera, &wmin, &wmax);
            arv_camera_get_height_bounds(m_camera, &hmin, &hmax);

            // Apply bounds
            x = max(x, xmin);
            x = min(x, xmax);
            y = max(y, ymin);
            y = min(y, ymax);
            if (width == 0) {
                // Whole sensor width
                width = wmax;
            } else {
                width = max(width, wmin);
                width = min(width, wmax);
            }
            if (height == 0) {
                // Whole sensor width
                height = hmax;
            } else {
                height = max(height, hmin);
                height = min(height, hmax);
            }

            arv_camera_set_region(m_camera, x, y, width, height);
        }

        if (configuration.has("bin") && arv_camera_is_binning_available(m_camera)) {
            gint x = GET_PATH(configuration, "bin.x", int);
            gint y = GET_PATH(configuration, "bin.y", int);

            // Get bounds
            gint xmin, xmax, ymin, ymax;
            arv_camera_get_x_binning_bounds(m_camera, &xmin, &xmax);
            arv_camera_get_y_binning_bounds(m_camera, &ymin, &ymax);

            // Apply bounds
            x = max(x, xmin);
            x = min(x, xmax);
            y = max(y, ymin);
            y = min(y, ymax);

            arv_camera_set_binning(m_camera, x, y);
        }

        if (configuration.has("exposureTime") && arv_camera_is_exposure_time_available(m_camera)) {
            double exposureTime = configuration.get<double>("exposureTime");

            // Get bounds
            double tmin, tmax;
            arv_camera_get_exposure_time_bounds(m_camera, &tmin, &tmax);

            // exposure time must be multiple of tmin
            exposureTime = tmin * floor(exposureTime / tmin);

            // Apply bounds
            exposureTime = max(exposureTime, tmin);
            exposureTime = min(exposureTime, tmax);

            arv_camera_set_exposure_time(m_camera, exposureTime);
        }

        bool enable = GET_PATH(configuration, "frameRate.enable", bool);
        if (enable && arv_camera_is_frame_rate_available(m_camera)) {
            // set frame rate
            double frameRate;
            try {
                frameRate = GET_PATH(configuration, "frameRate.target", float);
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
                frameRate = arv_camera_get_frame_rate(m_camera);
            }

            // N.B. this function will set triggerMode to "Off" on the camera
            arv_camera_set_frame_rate(m_camera, frameRate);
        }

        const std::string& triggerMode = GET_PATH(configuration, "triggerMode", std::string);
        if (triggerMode == "On") {
            std::string triggerSource;
            try {
                triggerSource = GET_PATH(configuration, "triggerSource", std::string);
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
                triggerSource = arv_camera_get_trigger_source(m_camera);
            }

            // N.B. This function will internally set "TriggerActivation" to "RisingEdge".
            //      On Basler it will also set "AcquisitionFrameRateEnable" to 0.
            arv_camera_set_trigger(m_camera, triggerSource.c_str()); // configures the camera in trigger mode

            const std::string& triggerActivation = GET_PATH(configuration, "triggerActivation", std::string);
            arv_device_set_string_feature_value(m_device, "TriggerActivation", triggerActivation.c_str());
        } else {
            arv_camera_clear_triggers(m_camera); // disable all triggers
        }

        if (this->get<std::string>("vendor") == "Basler") {
            // Needed to allow frame rate control, in triggerMode "On"
            arv_device_set_integer_feature_value(m_device, "AcquisitionFrameRateEnable", enable);
        }

        if (configuration.has("autoGain") && arv_camera_is_gain_auto_available(m_camera)) {
            const std::string& autoGainStr = configuration.get<std::string>("autoGain");
            const ArvAuto autoGain = arv_auto_from_string(autoGainStr.c_str());

            arv_camera_set_gain_auto(m_camera, autoGain);
        }

        if (configuration.has("gain") && arv_camera_is_gain_available(m_camera)) {
            double gain = configuration.get<double>("gain");

            // Get bounds
            double gmin, gmax;
            arv_camera_get_gain_bounds(m_camera, &gmin, &gmax);

            // Apply bounds
            gain = max(gain, gmin);
            gain = min(gain, gmax);

            arv_camera_set_gain(m_camera, gain);
        }

        if (configuration.has("acquisitionMode")) {
            const std::string& acquisitionMode = configuration.get<std::string>("acquisitionMode");
            arv_camera_set_acquisition_mode(m_camera, arv_acquisition_mode_from_string(acquisitionMode.c_str()));
        }

        if (configuration.has("frameCount")) {
            gint64 frameCount = configuration.get<long long>("frameCount");

            // Get bounds
            gint64 fmin, fmax;
            arv_camera_get_frame_count_bounds(m_camera, &fmin, &fmax);

            // Apply bounds
            frameCount = max(frameCount, fmin);
            frameCount = min(frameCount, fmax);

            arv_camera_set_frame_count(m_camera, frameCount);
        }

        // Filter configuration by tag "genicam" and loop over it
        Hash filtered = this->filterByTags(configuration, "genicam");
        std::vector<std::string> paths;
        filtered.getPaths(paths);
        for (const auto& key : paths) {
            bool success = false;
            const auto feature = this->getAliasFromKey<std::string>(key);
            const auto valueType = this->getValueType(key);
            switch(valueType) {
                case Types::BOOL:
                    success = this->setBoolFeature(feature, configuration.get<bool>(key));
                    break;
                case Types::STRING:
                    success = this->setStringFeature(feature, configuration.get<std::string>(key));
                    break;
                case Types::INT64:
                    success = this->setIntFeature(feature, configuration.get<long long>(key));
                    break;
                case Types::FLOAT:
                    success = this->setFloatFeature(feature, configuration.get<float>(key));
                    break;
                case Types::DOUBLE:
                    success = this->setFloatFeature(feature, configuration.get<double>(key));
                    break;
                default:
                    throw KARABO_NOT_IMPLEMENTED_EXCEPTION(key + " datatype not available in GenICam");
            }

            if (!success) {
                // Failed -> erase key from full configuration
                KARABO_LOG_ERROR << "Could not set value for " << key;
                configuration.erase(key);
            }
        }
    }


    void AravisCamera::acquire() {
        m_timer.now();
        m_counter = 0;
        m_stream = arv_camera_create_stream(m_camera, AravisCamera::stream_cb, static_cast<void*>(this));

        if (!ARV_IS_STREAM(m_stream)) {
            KARABO_LOG_ERROR << "Stream could not be created";
            return;
        }

        // Enable emission of signals (it's disabled by default for performance reason)
        arv_stream_set_emit_signals(m_stream, TRUE);

        // Create and push buffers to the stream
	const gint payload = arv_camera_get_payload(m_camera);
	for (size_t i = 0; i < 10; i++)
            arv_stream_push_buffer(m_stream, arv_buffer_new(payload, NULL));

        arv_camera_start_acquisition(m_camera);

        // Connect the 'new-buffer' signal
        g_signal_connect(m_stream, "new-buffer", G_CALLBACK(AravisCamera::new_buffer_cb), static_cast<void*>(this));

        this->updateState(State::ACQUIRING);
    }

    
    void AravisCamera::stop() {
        this->clear_stream();

        arv_camera_stop_acquisition(m_camera);

        this->set("frameRate.actual", 0.);
        this->updateState(State::ON);
    }


    void AravisCamera::trigger() {
        const std::string& triggerMode = this->get<std::string>("triggerMode");
        if (triggerMode == "On") {
            const std::string& triggerSource = this->get<std::string>("triggerSource");
            if (triggerSource == "Software") {
                arv_camera_software_trigger(m_camera);
            }
        }
    }


    void AravisCamera::clear_camera() {
        if (ARV_IS_CAMERA(m_camera)) {
            g_clear_object(&m_camera);
        }
    }


    void AravisCamera::clear_stream() {
        if (ARV_IS_STREAM(m_stream)) {
            // TODO possibly disconnect signal, see https://developer.gnome.org/gobject/stable/gobject-Signals.html#g-signal-handler-disconnect

            // Disable emission of signals and free resource
            arv_stream_set_emit_signals(m_stream, FALSE);
            g_clear_object(&m_stream);
        }
    }


    void AravisCamera::stream_cb(void *context, ArvStreamCallbackType type, ArvBuffer *buffer) {
        Self* self = static_cast<Self*>(context);

        if (type == ARV_STREAM_CALLBACK_TYPE_INIT) {
            KARABO_LOG_FRAMEWORK_DEBUG << "Init stream";
                if (!arv_make_thread_realtime(10) && !arv_make_thread_high_priority(-10)) {
                    KARABO_LOG_FRAMEWORK_WARN << "Failed to make stream thread high priority";
                }
        }
    }


    void AravisCamera::new_buffer_cb(ArvStream* stream, void* context) {
        Self* self = static_cast<Self*>(context);

        ArvBuffer* arv_buffer = arv_stream_pop_buffer(stream);
	if (!ARV_IS_BUFFER(arv_buffer))
		return;

        if (arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS) {
            gint x, y, width, height;
            size_t buffer_size;

            const void* buffer_data = arv_buffer_get_data(arv_buffer, &buffer_size);
            arv_buffer_get_image_region(arv_buffer, &x, &y, &width, &height);
            const ArvPixelFormat pixel_format = arv_buffer_get_image_pixel_format(arv_buffer); // e.g. ARV_PIXEL_FORMAT_MONO_8
            const guint32 frame_id = arv_buffer_get_frame_id(arv_buffer);
            //KARABO_LOG_FRAMEWORK_DEBUG << "Got frame " << frame_id;

            switch(pixel_format) {
                case ARV_PIXEL_FORMAT_MONO_8:
                    self->writeOutputChannels<unsigned char>(buffer_data, width, height);
                    break;
                case ARV_PIXEL_FORMAT_MONO_10:
                case ARV_PIXEL_FORMAT_MONO_12:
                case ARV_PIXEL_FORMAT_MONO_14:
                case ARV_PIXEL_FORMAT_MONO_16:
                    self->writeOutputChannels<unsigned short>(buffer_data, width, height);
                    break;
                case ARV_PIXEL_FORMAT_MONO_10_PACKED:
                case ARV_PIXEL_FORMAT_MONO_12_PACKED:
                {
                    const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer_data);
                    uint16_t* unpackedData = new uint16_t[width * height];
                    unpackMono12Packed(data, width, height, unpackedData);
                    self->writeOutputChannels<unsigned short>(unpackedData, width, height);
                    delete[] unpackedData;
                }
                    break;
                // TODO RGB, YUV...
                default:
                    KARABO_LOG_FRAMEWORK_ERROR << "Format " << pixel_format << " is not supported"; // TODO pixel_format as string
                    self->execute("stop");
            }
        }

        // Push back the buffer to the stream
        arv_stream_push_buffer(stream, arv_buffer);
    }


    void AravisCamera::control_lost_cb(ArvGvDevice* gv_device, void* context) {
        // Control of the device is lost

        Self* self = static_cast<Self*>(context);

        // TODO what happens with multiple cameras on server?
        // Possibly use arv_gv_device_get_device_address (gv_device) to verify IP address

        KARABO_LOG_FRAMEWORK_WARN << "Control of the camera " << self->get<std::string>("cameraId") << " is lost";
        // TODO possibly release resources
        // NOTE 'self->clear_camera();' will seg fault
        self->m_camera = NULL;
        self->m_device = NULL;

        self->updateState(State::UNKNOWN);
    }


    void AravisCamera::pollOnce(karabo::util::Hash& h) {
        const long long packetDelay = arv_camera_gv_get_packet_delay(m_camera);
        h.set("packetDelay", packetDelay);

        const guint packetSize = arv_camera_gv_get_packet_size(m_camera);
        h.set("packetSize", packetSize);

        gint x, y, width, height;
        arv_camera_get_region(m_camera, &x, &y, &width, &height);
        h.set("roi.x", x);
        h.set("roi.y", y);
        h.set("roi.width", width);
        h.set("roi.height", height);

        if (arv_camera_is_binning_available(m_camera)) {
            gint dx, dy;
            arv_camera_get_binning(m_camera, &dx, &dy);
            h.set("bin.x", dx);
            h.set("bin.y", dy);
        }

        h.set("pixelFormat", arv_camera_get_pixel_format_as_string(m_camera));

        if (arv_camera_is_exposure_time_available(m_camera)) {
            h.set("exposureTime", arv_camera_get_exposure_time(m_camera));
        }

        const std::string& triggerMode = this->get<std::string>("triggerMode");
        if (triggerMode == "On") {
            h.set("triggerSource", arv_camera_get_trigger_source(m_camera));
        } else {
            h.set("frameRate.target", arv_camera_get_frame_rate(m_camera));
        }

        if (arv_camera_is_gain_auto_available(m_camera)) {
            const ArvAuto autoGain = arv_camera_get_gain_auto(m_camera);
            const std::string autoGainStr(arv_auto_to_string(autoGain));
            h.set("autoGain", autoGainStr);
        }

        if (arv_camera_is_gain_available(m_camera)) {
            const double gain = arv_camera_get_gain(m_camera);
            h.set("gain", gain);
        }

        const ArvAcquisitionMode acquisitionMode = arv_camera_get_acquisition_mode(m_camera);
        const std::string acquisitionModeStr(arv_acquisition_mode_to_string(acquisitionMode));
        h.set("acquisitionMode", acquisitionModeStr);

        const long long frameCount = arv_camera_get_frame_count(m_camera);
        h.set("frameCount", frameCount);

        // Filter paths by tag "genicam" and poll features
        std::vector<std::string> paths;
        this->getPathsByTag(paths, "genicam");
        this->pollGenicamFeatures(paths, h);
    }


    void AravisCamera::pollCamera(const boost::system::error_code & ec) {
        if (ec == boost::asio::error::operation_aborted) return;

        if (!m_camera) {
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
                    if (this->getBoolFeature(feature, boolValue)) {
                        h.set(key, boolValue);
                    }
                    break;
                case Types::STRING:
                    if (this->getStringFeature(feature, stringValue)) {
                        h.set(key, stringValue);
                    }
                    break;
                case Types::INT64:
                    if (this->getIntFeature(feature, intValue)) {
                        h.set(key, intValue);
                    }
                    break;
                case Types::FLOAT:
                case Types::DOUBLE:
                    if (this->getFloatFeature(feature, doubleValue)) {
                        h.set(key, doubleValue);
                    }
                    break;
                default:
                    throw KARABO_NOT_IMPLEMENTED_EXCEPTION(key + " datatype not available in GenICam");
            }
        }
    }

    void AravisCamera::updateOutputSchema() {
        if (m_camera == NULL) {
            // cannot query camera, as we are not connected
            return;
        }

        Hash h;
        this->pollOnce(h);

        const unsigned long long height = h.get<int>("roi.height");
        const unsigned long long width = h.get<int>("roi.width");
        const std::vector<unsigned long long> shape = {height, width};

        const ArvPixelFormat pixelFormat = arv_camera_get_pixel_format(m_camera);
        unsigned short bpp;
        Types::ReferenceType kType;
        switch(pixelFormat) {
            case ARV_PIXEL_FORMAT_MONO_8:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT8;
                bpp = 8;
                break;
            case ARV_PIXEL_FORMAT_MONO_10:
            case ARV_PIXEL_FORMAT_MONO_10_PACKED:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                bpp = 10;
                break;
            case ARV_PIXEL_FORMAT_MONO_12:
            case ARV_PIXEL_FORMAT_MONO_12_PACKED:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                bpp = 12;
                break;
            case ARV_PIXEL_FORMAT_MONO_14:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                bpp = 14;
                break;
            case ARV_PIXEL_FORMAT_MONO_16:
                m_encoding = Encoding::GRAY;
                kType = Types::UINT16;
                bpp = 16;
                break;
            case ARV_PIXEL_FORMAT_RGB_8_PACKED:
            case ARV_PIXEL_FORMAT_RGB_8_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT8;
                bpp = 24;
                break;
            case ARV_PIXEL_FORMAT_RGB_10_PACKED:
            case ARV_PIXEL_FORMAT_RGB_10_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                bpp = 30;
                break;
            case ARV_PIXEL_FORMAT_RGB_12_PACKED:
            case ARV_PIXEL_FORMAT_RGB_12_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                bpp = 36;
                break;
            case ARV_PIXEL_FORMAT_RGB_16_PLANAR:
                m_encoding = Encoding::RGB;
                kType = Types::UINT16;
                bpp = 48;
                break;
            // TODO: YUV
            default:
                m_encoding = Encoding::GRAY;
                kType = Types::UNKNOWN;
                bpp = 0;
                break;
        }
        h.set("bpp", bpp);

        this->set(h);
        CameraImageSource::updateOutputSchema(shape, m_encoding, kType);

        guint n_values;
        const char** options;

        // get available pixel formats
        options= arv_camera_get_available_pixel_formats_as_strings(m_camera, &n_values);
        std::string pixelFormatOptions;
        for (unsigned short i = 0; i < n_values; ++i) {
            if (i > 0) pixelFormatOptions.append(",");
            pixelFormatOptions.append(options[i]);
        }
        g_free(options);

        // get available trigger sources
        options = arv_camera_get_available_trigger_sources(m_camera, &n_values);
        std::string triggerSourceOptions;
        for (unsigned short i = 0; i < n_values; ++i) {
            if (i > 0) triggerSourceOptions.append(",");
            triggerSourceOptions.append(options[i]);
        }
        g_free(options);

        Schema schemaUpdate;
        STRING_ELEMENT(schemaUpdate).key("pixelFormat")
                .displayedName("Pixel Format")
                .assignmentOptional().noDefaultValue()
                .options(pixelFormatOptions)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(schemaUpdate).key("triggerSource")
                .displayedName("Trigger Source")
                .assignmentOptional().noDefaultValue()
                .options(triggerSourceOptions)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        // Disable setting not available properties
        std::string notAvailable("Not available for this camera.");

        if (!arv_camera_is_binning_available(m_camera)) {
            NODE_ELEMENT(schemaUpdate).key("bin")
                    .displayedName("Image Binning")
                    .commit();

            INT32_ELEMENT(schemaUpdate).key("bin.x")
                    .displayedName("X Binning")
                    .description(notAvailable)
                    .readOnly()
                    .commit();

            INT32_ELEMENT(schemaUpdate).key("bin.y")
                    .displayedName("Y Binning")
                    .description(notAvailable)
                    .readOnly()
                    .commit();
        }

        if (!arv_camera_is_exposure_time_available(m_camera)) {
            DOUBLE_ELEMENT(schemaUpdate).key("exposureTime")
                    .displayedName("Exposure Time")
                    .description(notAvailable)
                    .readOnly()
                    .commit();
        }

        NODE_ELEMENT(schemaUpdate).key("frameRate")
                .displayedName("Frame Rate")
                .commit();

        if (!arv_camera_is_frame_rate_available(m_camera)) {
            FLOAT_ELEMENT(schemaUpdate).key("frameRate.target")
                    .displayedName("Target Frame Rate")
                    .description(notAvailable)
                    .readOnly()
                    .commit();

            FLOAT_ELEMENT(schemaUpdate).key("frameRate.actual")
                    .displayedName("Actual Frame Rate")
                    .description("The measured frame rate.")
                    .unit(Unit::HERTZ)
                    .readOnly()
                    .initialValue(0.)
                    .commit();
        } else {
            FLOAT_ELEMENT(schemaUpdate).key("frameRate.target")
                    .displayedName("Target Frame Rate")
                    .description("Sets the 'target' value of the acquisition frame rate on the camera. "
                    "Please be aware that if you enable this feature in combination with external trigger, "
                    "the resulting 'actual' frame rate will most likely be smaller.")
                    .assignmentOptional().noDefaultValue()
                    .minExc(0.)
                    .unit(Unit::HERTZ)
                    .reconfigurable()
                    .allowedStates(State::UNKNOWN, State::ON)
                    .commit();

            FLOAT_ELEMENT(schemaUpdate).key("frameRate.actual")
                    .displayedName("Actual Frame Rate")
                    .description("The measured frame rate.")
                    .unit(Unit::HERTZ)
                    .readOnly()
                    .initialValue(0.)
                    .commit();
        }

        if (std::string(arv_camera_get_vendor_name(m_camera)) == "Basler") {
            BOOL_ELEMENT(schemaUpdate).key("frameRate.enable")
                    .displayedName("Frame Rate Enable")
                    .description("Enable frame rate control when camera is in trigger mode.")
                    .assignmentOptional().defaultValue(false)
                    .reconfigurable()
                    .allowedStates(State::UNKNOWN, State::ON)
                    .commit();
        } else {
            BOOL_ELEMENT(schemaUpdate).key("frameRate.enable")
                    .displayedName("Frame Rate Enable")
                    .description(notAvailable)
                    .readOnly()
                    .commit();
        }

        if (!arv_camera_is_gain_auto_available(m_camera)) {
            STRING_ELEMENT(schemaUpdate).key("autoGain")
                    .displayedName("Auto Gain")
                    .description(notAvailable)
                    .readOnly()
                    .commit();
        }

        if (!arv_camera_is_gain_available(m_camera)) {
            DOUBLE_ELEMENT(schemaUpdate).key("gain")
                    .displayedName("Gain")
                    .description(notAvailable)
                    .readOnly()
                    .commit();
        }

        this->appendSchema(schemaUpdate);
    }


    template <class T>
    void AravisCamera::writeOutputChannels(const void* data, gint width, gint height) {
        const Dims shape(height, width);

        // Non-copy NDArray constructor
        const karabo::util::NDArray imgArray((T*) data, width*height, karabo::util::NDArray::NullDeleter(), shape);

        const unsigned short bpp = this->get<unsigned short>("bpp");
        const Dims binning(this->get<int>("bin.y"), this->get<int>("bin.x"));
        const Dims roiOffsets(this->get<int>("roi.y"), this->get<int>("roi.x"));
        const Timestamp ts;
        const Hash header;

        // Send image and metadata to output channel
        this->writeChannels(imgArray, binning, bpp, m_encoding, roiOffsets, ts, header);

        m_counter += 1;
        if (m_timer.elapsed() >= 1.) {
            const float frameRate = m_counter / m_timer.elapsed();
            m_counter = 0;
            m_timer.now();
            this->set("frameRate.actual", frameRate);
        }
    }


    const std::string AravisCamera::resolveHostname(const std::string& hostname) {
        std::string ipAddress;
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        const boost::asio::ip::tcp::resolver::query query(hostname, "");
        const boost::asio::ip::tcp::resolver::iterator end;
        boost::system::error_code ec;
        auto it = resolver.resolve(query, ec);
        if (ec != boost::system::errc::success) {
            if (m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Boost error in resolveHostname: " << ec.message();
            }
        } else if (it != end) {
            const boost::asio::ip::tcp::endpoint endpoint = it->endpoint();
            ipAddress = endpoint.address().to_string();
        } else {
            if (m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Cannot resolve hostname: " << hostname;
            }
        }

        return ipAddress;
    }

}
