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

/**
 * The main Karabo namespace
 */
namespace karabo {

    class BaslerCamera : public AravisCamera {
    public:

        KARABO_CLASSINFO(BaslerCamera, "BaslerCamera", "2.9")

        static void expectedParameters(karabo::util::Schema& expected);

        BaslerCamera(const karabo::util::Hash& config);

        virtual ~BaslerCamera();

    };

} // namespace karabo

#endif // KARABO_BASLERCAMERA_HH
