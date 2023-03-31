******************************
AravisCameras Device (C++)
******************************

Introduction
============

The AravisCamera class provides a way to interface with a GigE Vision camera
in a Karabo framework environment. It enables users to configure the camera
settings, acquire images, and dynamically update the settings during runtime.
This class simplifies the interaction with the camera and streamlines the process
making it easier to integrate the camera into various applications.

This software is released by the European XFEL GmbH as is and without any warranty
under the GPLv3 license. If you have questions on contributing to the project,
please get in touch at opensource@xfel.eu. Before contributing you are required
to sign either a Contributors License Agreement, or Entity Contributor License
Agreement, which you can find in the root directory of this project. Please mail
the signed agreement to opensource@xfel.eu. By signing the CLA you acknowledge that
copyright and all intellectual property rights of your contribution are transferred
to the European X-ray Free Electron Laser Facility GmbH.
You are free to use this software under the terms of the GPLv3 without signing a CLA.

Dependencies
============

- aravis

  - meson
  - ninja

- imageSource

  - opencv

Compiling
=========

1. Source activate the Karabo installation against which the device will be
   developed:

    ``cd <Karabo installation root directory>``
    ``source ./activate``

2. Goto the device source root directory and generate its build files with cmake:

     ``cd $KARABO/devices/aravisCameras``
     ``mkdir build``
     ``cd build``
     ``cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KARABO/extern ..``

   CMAKE_BUILD_TYPE can also be set to "Release".

3. Build the device:

     ``cd $KARABO/devices/aravisCameras``
     ``cmake --build . ``

   ``make`` can also be used as long as the Makefile generator is used by cmake.

Testing
=======

After a successfull build, a shared library is generated here:

``dist/<configuration>/<system>/libaravisCameras.so``

And a soft-link to the ``libaravisCameras.so`` file is created in the
``$KARABO/plugins`` folder.

To run the tests, go to the tests directory in your build tree and use ``ctest``:

    ``cd $KARABO/devices/aravisCameras/build/aravisCameras``
    ``ctest -VV``

Running
=======

If you want to manually start a server using this device, simply type:

``karabo-cppserver serverId=cppServer/1 deviceClasses=AravisCameras``

Or just use (a properly configured):

``karabo-start``
