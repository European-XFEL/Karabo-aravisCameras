/*
 * Author: <parenti>
 *
 * Created on April 27, 2021, 18:00 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_ARAVISBASLER2CAMERA_HH
#define KARABO_ARAVISBASLER2CAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh"  // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisBasler2Camera : public AravisCamera {
    public:

        KARABO_CLASSINFO(AravisBasler2Camera, "AravisBasler2Camera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::util::Schema& expected);

        explicit AravisBasler2Camera(const karabo::util::Hash& config);

        virtual ~AravisBasler2Camera();

        virtual bool synchronize_timestamp() override;

        virtual bool configure_timestamp_chunk() override;

        virtual bool get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const override;

        virtual bool get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) override;

        virtual bool is_flip_x_available() const override;

        virtual bool is_flip_y_available() const override;

    private:
        virtual void resetCamera() override;

        bool m_ptp_enabled;
        int m_tick_frequency;
        gint64 m_reference_camera_timestamp;

    };

} // namespace karabo

#endif // KARABO_ARAVISBASLER2CAMERA_HH
