/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#ifndef KARABO_ARAVISCAMERA_HH
#define KARABO_ARAVISCAMERA_HH

#include <unordered_map>

extern "C" {
#include <arv.h>
}

#include <image_source/CameraImageSource.hh>
#include <karabo/karabo.hpp>

#include "version.hh" // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    enum class Result { SUCCESS, FAIL, NOT_AVAILABLE };

    class AravisCamera : public CameraImageSource {
       public:
        KARABO_CLASSINFO(AravisCamera, "AravisCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::data::Schema& expected);

        explicit AravisCamera(const karabo::data::Hash& config);

        virtual ~AravisCamera() = default;

        virtual void preDestruction() final;

        virtual void preReconfigure(karabo::data::Hash& incomingReconfiguration) override;

        virtual void postReconfigure() override;

       protected:
        bool m_is_base_class; // False for derived classes
        bool m_is_gv_device;  // True for GEV cameras
        bool m_is_uv_device;  // True for USB3V cameras
        bool m_is_device_reset_available;
        bool m_use_arv_camera_trigger; // Use arv_camera to access trigger

        bool m_chunk_mode;
        gint m_width;
        gint m_height;
        guint m_buffer_size;
        ArvPixelFormat m_format;

        unsigned int m_max_correction_time;
        double m_min_latency;
        double m_max_latency;

        // ArvCamera and ArvDevice are not thread safe, thus a lock is needed
        mutable boost::mutex m_camera_mtx;
        ArvCamera* m_camera;
        ArvDevice* m_device;
        ArvChunkParser* m_parser;

        bool isFeatureAvailable(const std::string& feature) const;
        bool is_frame_count_available() const;
        virtual bool is_flip_x_available() const;
        virtual bool is_flip_y_available() const;

        virtual void configure(karabo::data::Hash& configuration);

        unsigned long long m_timestampErrorCount;
        std::string m_timestampError;
        karabo::data::Timestamp m_reference_karabo_time;
        virtual int get_tick_frequency();
        virtual bool synchronize_timestamp();
        virtual bool configure_timestamp_chunk();
        virtual bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts);

        virtual bool get_region(gint& x, gint& y, gint& width, gint& height);
        virtual std::string get_frame_rate_enable_parameter_name() const;

       private:
        bool m_is_frame_count_available;
        bool m_is_binning_available;
        bool m_is_exposure_time_available;
        bool m_is_flip_x_available;
        bool m_is_flip_y_available;
        bool m_is_frame_rate_available;
        bool m_is_gain_available;
        bool m_is_gain_auto_available;

        std::string m_exposure_time_feature;

        karabo::xms::Encoding m_encoding;
        std::vector<unsigned long long> m_shape;

        static const std::set<ArvPixelFormat> m_supportedPixelFormats;
        std::unordered_map<ArvPixelFormat, std::string> m_pixelFormatOptions;

        bool m_connect; // Set to false to quit connection loop
        bool m_is_connected;
        boost::asio::deadline_timer m_reconnect_timer;
        unsigned short m_failed_connections;
        static boost::mutex m_connect_mtx; // A class lock is needed for ArvInterface

        bool resolveHostname(const std::string& hostname, std::string& ip_address, std::string& message);
        void connect(const boost::system::error_code& ec);
        void connection_failed_helper(const std::string& message, const std::string& detailed_msg = "");
        bool verify_vendor_and_model(const std::string& vendor, const std::string& model);
        static void control_lost_cb(ArvGvDevice* gv_device, void* context);
        void clear_camera();

        bool m_need_schema_update;
        void getPathsByTag(std::vector<std::string>& paths, const std::string& tags);
        void disableElement(const std::string& key, karabo::data::Schema& schemaUpdate);
        bool updateOutputSchema();

        void initialize();
        void refresh();
        void reset();
        virtual void resetCamera();

        mutable boost::mutex m_stream_mtx; // Object lock for ArvStream
        bool m_need_stream_clear;          // After a reconfiguration the stream need to be cleared
        ArvStream* m_stream;
        static void stream_cb(void* context, ArvStreamCallbackType type, ArvBuffer* buffer);
        void clear_stream();

        bool set_auto_packet_size();
        bool set_exposure_time(double& exposure_time);
        bool set_frame_rate(bool enable, double frame_rate = 0.);
        bool get_gain(double& absGain, double& normGain);
        bool set_gain(double& absGain, double& normGain, bool normalized);
        bool set_frame_count(gint64& frame_count);
        bool set_region(int& x, int& y, int& width, int& height);
        bool set_binning(int& bin_x, int& bin_y);

        boost::asio::deadline_timer m_poll_timer;
        void pollOnce(karabo::data::Hash& h);
        void pollCamera(const boost::system::error_code& ec);
        void pollGenicamFeatures(const std::vector<std::string>& paths, karabo::data::Hash& h);

        bool m_is_acquiring;
        bool m_isContinuousMode;
        unsigned long long m_imgsToBeAcquired; // Images to be acquired in multiframe mode
        std::vector<uint16_t> m_unpackedData;
        karabo::net::Strand::Pointer m_outputStrand;
        void acquire();
        void acquire_failed_helper(const std::string& detailed_msg);
        void stop();
        virtual void trigger();
        void process_buffer(ArvBuffer* buffer);
        template <class T>
        void writeOutputChannels(const void* data, gint width, gint height, const karabo::data::Timestamp& ts);
        void updateFrameRate();

        Result getBoolFeature(const std::string& feature, bool& value);
        Result getStringFeature(const std::string& feature, std::string& value);
        Result getIntFeature(const std::string& feature, long long& value);
        Result getFloatFeature(const std::string& feature, double& value);

        Result setBoolFeature(const std::string& feature, bool& value);
        Result setStringFeature(const std::string& feature, std::string& value);
        Result setIntFeature(const std::string& feature, long long& value);
        Result setFloatFeature(const std::string& feature, double& value);

        unsigned long long m_errorCount;
        ArvBufferStatus m_lastError;
        std::unordered_map<ArvBufferStatus, std::string> m_bufferStatus;

        // Image latency
        karabo::data::Epochstamp m_timer;
        unsigned long m_counter;
        double m_mean_latency;
    };
} // namespace karabo

#endif
