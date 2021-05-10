/*
 * Author: <parenti>
 *
 * Created on March 17, 2020, 12:20 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#ifndef KARABO_PHOTONICSCIENCECAMERA_HH
#define KARABO_PHOTONICSCIENCECAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh"  // provides PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class PhotonicScienceCamera : public AravisCamera {
    public:

        KARABO_CLASSINFO(PhotonicScienceCamera, "AravisPhotonicScienceCamera", PACKAGE_VERSION)

        static void expectedParameters(karabo::util::Schema& expected);

        PhotonicScienceCamera(const karabo::util::Hash& config);

        virtual ~PhotonicScienceCamera();

    private:
        void configure(karabo::util::Hash& configuration); // Over-ride parent's method
        virtual void trigger(); // Over-ride parent's method

    };

} // namespace karabo

#endif // KARABO_PHOTONICSCIENCECAMERA_HH
