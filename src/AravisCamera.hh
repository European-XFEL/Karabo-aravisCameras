/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_ARAVISCAMERA_HH
#define KARABO_ARAVISCAMERA_HH

extern "C" {
  #include <arv.h>
}

#include <karabo/karabo.hpp>

#include <image_source/CameraImageSource.hh>

/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisCamera : public CameraImageSource {

    public:

        // Add reflection information and Karabo framework compatibility to this class
        KARABO_CLASSINFO(AravisCamera, "AravisCamera", "2.6")

        /**
         * Necessary method as part of the factory/configuration system
         * @param expected Will contain a description of expected parameters for this device
         */
        static void expectedParameters(karabo::util::Schema& expected);

        /**
         * Constructor providing the initial configuration in form of a Hash object.
         * If this class is constructed using the configuration system the Hash object will
         * already be validated using the information of the expectedParameters function.
         * The configuration is provided in a key/value fashion.
         */
        AravisCamera(const karabo::util::Hash& config);

        /**
         * The destructor will be called in case the device gets killed
         */
        virtual ~AravisCamera();

        /**
         * This function acts as a hook and is called after an reconfiguration request was received,
         * but BEFORE this reconfiguration request is actually merged into this device's state.
         *
         * The reconfiguration information is contained in the Hash object provided as an argument.
         * You have a chance to change the content of this Hash before it is merged into the device's current state.
         *
         * NOTE: (a) The incomingReconfiguration was validated before
         *       (b) If you do not need to handle the reconfigured data, there is no need to implement this function.
         *           The reconfiguration will automatically be applied to the current state.
         * @param incomingReconfiguration The reconfiguration information as was triggered externally
         */
        virtual void preReconfigure(karabo::util::Hash& incomingReconfiguration);


        /**
         * This function acts as a hook and is called after an reconfiguration request was received,
         * and AFTER this reconfiguration request got merged into this device's current state.
         * You may access any (updated or not) parameters using the usual getters and setters.
         * @code
         * int i = get<int>("myParam");
         * @endcode
         */
        virtual void postReconfigure();


    private:
        void initialize();

        bool m_connect;
        boost::asio::deadline_timer m_reconnect_timer;
        unsigned short m_failed_connections;
        void connect(const boost::system::error_code & ec);

        void configure(const karabo::util::Hash& configuration);
        void acquire();
        void stop();
        void trigger();

        void clear_camera();
        void clear_stream();

        static void stream_cb(void* context, ArvStreamCallbackType type, ArvBuffer* buffer);
        static void new_buffer_cb(ArvStream* stream, void* context);
        static void control_lost_cb(ArvGvDevice* gv_device, void* context);

        void pollOnce(karabo::util::Hash& h);
        void updateOutputSchema();
        template <class T>
        void writeOutputChannels(const void* data, gint width, gint height);

        ArvCamera* m_camera;
        ArvStream* m_stream;

        karabo::util::Epochstamp m_timer;
        unsigned long m_counter;

        karabo::xms::EncodingType m_encoding;
    };
}

#endif
