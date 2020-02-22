/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisCamera.hh"

using namespace std;

USING_KARABO_NAMESPACES;

#define GET_PATH(hash, path, type) hash.has(path) ? hash.get<type>(path) : this->get<type>(path);

namespace karabo {


    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera)

    void AravisCamera::expectedParameters(Schema& expected) {
        STRING_ELEMENT(expected).key("cameraIp")
                .displayedName("Camera IP")
                .description("The IP address of the network camera (eg 131.169.140.193).")  // TODO: hostname
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

        FLOAT_ELEMENT(expected).key("frameRate.target")
                .displayedName("Target Frame Rate")
                .description("Sets the 'absolute' value of the acquisition frame rate on the camera. "
                "The 'absolute' value is a float value that sets the acquisition frame rate in frames per second. "
                "This parameter will only have an effect if trigger mode is 'Off'.")
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

        STRING_ELEMENT(expected).key("cameraId")
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

        // TODO more properties: acquisitionMode, frameCount
    }


    AravisCamera::AravisCamera(const karabo::util::Hash& config) : CameraImageSource(config),
            m_connect(true), m_reconnect_timer(EventLoop::getIOService()), m_failed_connections(0u),
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
    }


    void AravisCamera::preReconfigure(karabo::util::Hash& incomingReconfiguration) {
        this->configure(incomingReconfiguration);
    }


    void AravisCamera::postReconfigure() {
        this->updateOutputSchema();
    }


    void AravisCamera::initialize() {
        m_reconnect_timer.expires_from_now(boost::posix_time::milliseconds(1));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
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

        this->clear_stream();
        this->clear_camera();

        const std::string& cameraIp = this->get<std::string>("cameraIp");
        m_camera = arv_camera_new(cameraIp.c_str());

        if (!ARV_IS_CAMERA(m_camera)) {
            if (m_failed_connections < 1) {
                KARABO_LOG_ERROR << "Cannot connect to " << cameraIp;
            }
            ++m_failed_connections;
            m_camera = NULL;

            m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
            m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
            return;
        }

        KARABO_LOG_INFO << "Connected to " << cameraIp;

        // Connect the control-lost signal
        g_signal_connect(arv_camera_get_device(m_camera), "control-lost", G_CALLBACK(AravisCamera::control_lost_cb), static_cast<void*>(this));

        Hash h;

        // Read immutable properties
        h.set("cameraId", std::string(arv_camera_get_device_id(m_camera)));
        h.set("vendor", std::string(arv_camera_get_vendor_name(m_camera)));
        h.set("model", std::string(arv_camera_get_model_name(m_camera)));

        gint width, height;
        arv_camera_get_sensor_size(m_camera, &width, &height);
        h.set("width", width);
        h.set("height", height);

        // TODO more properties...

        this->set(h);
        
        // Apply configuration
        this->configure(this->getCurrentConfiguration());

        this->updateOutputSchema();

        this->updateState(State::ON);
        m_failed_connections = 0;
        m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5l));
        m_reconnect_timer.async_wait(karabo::util::bind_weak(&AravisCamera::connect, this, boost::asio::placeholders::error));
    }


    void AravisCamera::configure(const karabo::util::Hash& configuration) {
        if (m_camera == NULL) {
            // cannot configure camera, as we are not connected
            return;
        }

        if (configuration.has("packetDelay")) {
            arv_camera_gv_set_packet_delay(m_camera, configuration.get<long long>("packetDelay"));
        }

        bool autoPacketSize = GET_PATH(configuration, "autoPacketSize", bool);
        if (autoPacketSize) {
            guint packetSize = arv_camera_gv_auto_packet_size(m_camera);
            arv_camera_gv_set_packet_size(m_camera, packetSize);
        } else {
            try {
                guint packetSize = GET_PATH(configuration, "packetSize", int);
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

        std::string triggerMode = GET_PATH(configuration, "triggerMode", std::string);
        if (triggerMode == "On") {
            std::string triggerSource;
            try {
                triggerSource = GET_PATH(configuration, "triggerSource", std::string);
            } catch (const karabo::util::ParameterException& e) {
                // key neither in configuration nor on device
                triggerSource = arv_camera_get_trigger_source(m_camera);
            }

            arv_camera_set_trigger(m_camera, triggerSource.c_str()); // configures the camera in trigger mode
        } else {
            arv_camera_clear_triggers(m_camera); // disable all triggers

            if (arv_camera_is_frame_rate_available(m_camera)) {
                // set frame rate
                double frameRate;
                try {
                    frameRate = GET_PATH(configuration, "frameRate.target", float);
                } catch (const karabo::util::ParameterException& e) {
                    // key neither in configuration nor on device
                    frameRate = arv_camera_get_frame_rate(m_camera);
                }

                arv_camera_set_frame_rate(m_camera, frameRate);
            }
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
                // TODO PACKED formats, RGB, YUV...
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

        KARABO_LOG_FRAMEWORK_WARN << "Control of the camera " << self->get<std::string>("cameraIp") << " is lost";
        // TODO possibly release resources
        // NOTE 'self->clear_camera();' will seg fault
        self->m_camera = NULL;

        self->updateState(State::UNKNOWN);
    }


    void AravisCamera::pollOnce(karabo::util::Hash& h) {
        long long packetDelay = arv_camera_gv_get_packet_delay(m_camera);
        h.set("packetDelay", packetDelay);

        guint packetSize = arv_camera_gv_get_packet_size(m_camera);
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

        if (!arv_camera_is_frame_rate_available(m_camera)) {
            NODE_ELEMENT(schemaUpdate).key("frameRate")
                    .displayedName("Frame Rate")
                    .commit();

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

}
