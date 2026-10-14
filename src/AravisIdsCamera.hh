#ifndef KARABO_ARAVISIDSCAMERA_HH
#define KARABO_ARAVISIDSCAMERA_HH

#include <string>

#include "AravisCamera.hh"
#include "version.hh"

namespace karabo {

    class AravisIdsCamera final : public AravisCamera {
       public:
        KARABO_CLASSINFO(AravisIdsCamera, "AravisIdsCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::data::Schema& expected);
        explicit AravisIdsCamera(const karabo::data::Hash& config);
        bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) override;
        ~AravisIdsCamera() override = default;

       private:
        bool m_is_timestamp_latch_available;
        double m_reference_camera_timestamp;

        void resetCamera() override;

        bool is_flip_x_available() const override;
        bool is_flip_y_available() const override;
        std::string get_frame_rate_enable_parameter_name() const override;
        bool synchronize_timestamp() override;
        int get_tick_frequency() override;
    };

} // namespace karabo

#endif // KARABO_ARAVISIDSCAMERA_HH
