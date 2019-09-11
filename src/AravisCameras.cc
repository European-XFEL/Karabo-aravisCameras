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
        /* Create a new glib main loop */
        m_gloop = g_main_loop_new(g_main_context_new(), FALSE);

        /* Run the main loop */
        //g_main_loop_run(m_gloop); // Block, and apparently not needed TODO rm
        
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

        /* Instantiation of the available */
        std::string cameraIp = this->get<std::string>("cameraIp");
        m_camera = arv_camera_new(cameraIp.c_str());

        if (m_camera == NULL) {
            KARABO_LOG_ERROR << "Cannot connect to " << cameraIp;
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

        /* And enable emission of this signal (it's disabled by default for performance reason) */
        //arv_stream_set_emit_signals(m_stream, TRUE);

	gint payload = arv_camera_get_payload(m_camera);
	for (size_t i = 0; i < 5; i++)
            arv_stream_push_buffer(m_stream, arv_buffer_new(payload, NULL));

        arv_camera_start_acquisition(m_camera);

        /* Connect the new-buffer signal */
        //g_signal_connect(m_stream, "new-buffer", G_CALLBACK(AravisCameras::new_buffer_cb), static_cast<void*>(this));

        this->updateState(State::ACQUIRING);
    }

    
    void AravisCameras::stop() {
        //if (ARV_IS_STREAM(m_stream))
        //    arv_stream_set_emit_signals(m_stream, FALSE);

	//g_clear_object(m_stream);  // TODO

        arv_camera_stop_acquisition(m_camera);
        this->updateState(State::ON);
    }


    void AravisCameras::stream_cb(void *context, ArvStreamCallbackType type, ArvBuffer *buffer) {
        Self* self = static_cast<Self*>(context);

        // TODO proper logging
        std::cout << "stream_cb" << std::endl;
        if (type == ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE) {
            std::cout << "Buffer done" << std::endl;
            // Do processing here

        } else if (type == ARV_STREAM_CALLBACK_TYPE_INIT) {
            std::cout << "Init stream" << std::endl;
                if (!arv_make_thread_realtime(10) && !arv_make_thread_high_priority(-10)) {
                    std::cout << "Failed to make stream thread high priority" << std::endl;
                }
        }
    }


    void AravisCameras::new_buffer_cb(ArvStream* stream, void* context) {
        Self* self = static_cast<Self*>(context);

        // TODO proper logging
        std::cout << "new_buffer_cb" << std::endl;
    }

}
