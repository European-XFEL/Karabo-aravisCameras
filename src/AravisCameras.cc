/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "AravisCameras.hh"

using namespace std;

USING_KARABO_NAMESPACES;

namespace karabo {


    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, AravisCameras)

    void AravisCameras::expectedParameters(Schema& expected) {
        STRING_ELEMENT(expected).key("cameraIp")
                .displayedName("Camera IP")
                .description("The IP address of the network camera (eg 131.169.140.193).")  // TODO: hostname
                .assignmentMandatory()
                .init() // cannot be changed after device instantiation
                .commit();

        SLOT_ELEMENT(expected).key("connect")
                .displayedName("Connect")
                .allowedStates(State::UNKNOWN)
                .commit();

        SLOT_ELEMENT(expected).key("acquire")
                .displayedName("Acquire")
                .allowedStates(State::ON)
                .commit();

        SLOT_ELEMENT(expected).key("stop")
                .displayedName("Stop")
                .allowedStates(State::ACQUIRING)
                .commit();

        STRING_ELEMENT(expected).key("cameraId")
                .displayedName("Camera ID")
                .readOnly()
                .commit();

        STRING_ELEMENT(expected).key("pixelFormat")
                .displayedName("Pixel Format")
                .assignmentOptional().defaultValue("Mono8")
                .options("Mono8 Mono12 Mono12Packed YUV422Packed YUV422_YUYV_Packed")
                .reconfigurable()
                .allowedStates(State::ON)
                .commit();

        // TODO more properties
    }


    AravisCameras::AravisCameras(const karabo::util::Hash& config) : Device<>(config) {        
        KARABO_SLOT(connect);
        KARABO_SLOT(acquire);
        KARABO_SLOT(stop);
    }


    AravisCameras::~AravisCameras() {
    }


    void AravisCameras::preReconfigure(karabo::util::Hash& incomingReconfiguration) {
        if (incomingReconfiguration.has("pixelFormat")) {
            const char* pixelFormat = incomingReconfiguration.get<std::string>("pixelFormat").c_str();
            arv_camera_set_pixel_format_from_string(m_camera, pixelFormat);
        }
    }


    void AravisCameras::postReconfigure() {
    }


    void AravisCameras::connect() {
        // TODO connect worker

        std::string cameraIp = this->get<std::string>("cameraIp");
        m_camera = arv_camera_new(cameraIp.c_str());

        if (m_camera == NULL) {
            KARABO_LOG_ERROR << "Cannot connect to " << cameraIp;
            // TODO free m_camera?
            return;
        }
        
        KARABO_LOG_INFO << "Connected to " << cameraIp;

        Hash h;
        h.set("cameraId", std::string(arv_camera_get_device_id(m_camera)));
        // TODO more properties...
        this->set(h);

        this->updateState(State::ON);
    }

    
    void AravisCameras::acquire() {
        m_stream = arv_camera_create_stream(m_camera, AravisCameras::stream_cb, static_cast<void*>(this));

        // Enable emission of signals (it's disabled by default for performance reason)
        arv_stream_set_emit_signals(m_stream, TRUE);

        // Create and push buffers to the stream
	gint payload = arv_camera_get_payload(m_camera);
	for (size_t i = 0; i < 10; i++)
            arv_stream_push_buffer(m_stream, arv_buffer_new(payload, NULL));

        arv_camera_start_acquisition(m_camera);

        // Connect the 'new-buffer' signal
        g_signal_connect(m_stream, "new-buffer", G_CALLBACK(AravisCameras::new_buffer_cb), static_cast<void*>(this));

        this->updateState(State::ACQUIRING);
    }

    
    void AravisCameras::stop() {
        if (ARV_IS_STREAM(m_stream)) {
            // Disable emission of signals
            arv_stream_set_emit_signals(m_stream, FALSE);
            // TODO free m_stream
        }

        arv_camera_stop_acquisition(m_camera);

        this->updateState(State::ON);
    }


    void AravisCameras::stream_cb(void *context, ArvStreamCallbackType type, ArvBuffer *buffer) {
        Self* self = static_cast<Self*>(context);

        // TODO proper logging
        if (type == ARV_STREAM_CALLBACK_TYPE_INIT) {
            std::cout << "Init stream" << std::endl;
                if (!arv_make_thread_realtime(10) && !arv_make_thread_high_priority(-10)) {
                    std::cout << "Failed to make stream thread high priority" << std::endl;
                }
        }
    }


    void AravisCameras::new_buffer_cb(ArvStream* stream, void* context) {
        Self* self = static_cast<Self*>(context);

        // TODO proper logging

        ArvBuffer* arv_buffer = arv_stream_pop_buffer(stream);
	if (arv_buffer == NULL)
		return;

        if (arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS) {
            char *buffer_data;
            int x, y, width, height;
            size_t buffer_size;
            ArvPixelFormat pixel_format;
            guint32 frame_id;
            
            buffer_data = (char *) arv_buffer_get_data(arv_buffer, &buffer_size);
            arv_buffer_get_image_region(arv_buffer, &x, &y, &width, &height);
            pixel_format = arv_buffer_get_image_pixel_format(arv_buffer); // e.g. ARV_PIXEL_FORMAT_MONO_8
            frame_id = arv_buffer_get_frame_id(arv_buffer);
            std::cout << "Got frame " << frame_id << std::endl;
            // TODO something with the buffer
        }

        // Push back the buffer to the stream
        arv_stream_push_buffer(stream, arv_buffer);
    }

}
