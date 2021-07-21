/*
 * Author: <parenti>
 *
 * Created on October 20, 2020, 16:34 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_ARAVISBASLERCAMERA_HH
#define KARABO_ARAVISBASLERCAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh"  // provides ARAVISCAMERAS_PACKAGE_VERSION


/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisBaslerCamera : public AravisCamera {
    public:

        KARABO_CLASSINFO(AravisBaslerCamera, "AravisBaslerCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::util::Schema& expected);

        AravisBaslerCamera(const karabo::util::Hash& config);

        virtual ~AravisBaslerCamera();

        virtual bool synchronize_timestamp();

        virtual bool configure_timestamp_chunk();

        virtual bool get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const;

        virtual bool get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts);

    private:
        virtual void resetCamera();
        virtual void reset_roi_and_binning(int x, int y, int width, int height, int bin_x, int bin_y);

        bool m_ptp_enabled;
        int m_tick_frequency;
        gint64 m_reference_camera_timestamp;
        unsigned int m_max_correction_time;

    };

} // namespace karabo

#endif // KARABO_ARAVISBASLERCAMERA_HH
