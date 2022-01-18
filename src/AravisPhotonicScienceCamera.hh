/*
 * Author: <parenti>
 *
 * Created on March 17, 2020, 12:20 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_ARAVISPHOTONICSCIENCECAMERA_HH
#define KARABO_ARAVISPHOTONICSCIENCECAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh"  // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisPhotonicScienceCamera : public AravisCamera {
    public:

        KARABO_CLASSINFO(AravisPhotonicScienceCamera, "AravisPhotonicScienceCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::util::Schema& expected);

        explicit AravisPhotonicScienceCamera(const karabo::util::Hash& config);

        virtual ~AravisPhotonicScienceCamera();

        virtual bool synchronize_timestamp() override;

        virtual bool configure_timestamp_chunk() override;

        virtual bool get_region(gint& x, gint& y, gint& width, gint& height) const override;

        virtual bool get_shape_and_format(ArvBuffer* buffer, gint& width, gint& height, ArvPixelFormat& format) const override;

        virtual bool get_timestamp(ArvBuffer* buffer, karabo::util::Timestamp& ts) override;

        virtual bool is_flip_y_available() const override;

    private:
        void configure(karabo::util::Hash& configuration); // Over-ride parent's method
        virtual void trigger() override; // Over-ride parent's method

        double m_reference_camera_timestamp;

    };

} // namespace karabo

#endif // KARABO_ARAVISPHOTONICSCIENCECAMERA_HH
