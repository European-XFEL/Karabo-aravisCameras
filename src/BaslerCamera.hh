/*
 * Author: <parenti>
 *
 * Created on October 20, 2020, 16:34 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_BASLERCAMERA_HH
#define KARABO_BASLERCAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh"  // provides PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class BaslerCamera : public AravisCamera {
    public:

        KARABO_CLASSINFO(BaslerCamera, "AravisBaslerCamera", PACKAGE_VERSION)

        static void expectedParameters(karabo::util::Schema& expected);

        BaslerCamera(const karabo::util::Hash& config);

        virtual ~BaslerCamera();

        virtual bool synchronize_timestamp();

        virtual bool configure_timestamp_chunk();

        virtual bool get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const;

        virtual bool get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) const;

    private:
        bool m_ptp_enabled;
        int m_tick_frequency;
        gint64 m_reference_camera_timestamp;
        mutable boost::mutex m_sync_lock;

    };

} // namespace karabo

#endif // KARABO_BASLERCAMERA_HH
