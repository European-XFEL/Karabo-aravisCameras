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

        AravisPhotonicScienceCamera(const karabo::util::Hash& config);

        virtual ~AravisPhotonicScienceCamera();

    private:
        void configure(karabo::util::Hash& configuration); // Over-ride parent's method
        virtual void trigger(); // Over-ride parent's method

    };

} // namespace karabo

#endif // KARABO_ARAVISPHOTONICSCIENCECAMERA_HH
