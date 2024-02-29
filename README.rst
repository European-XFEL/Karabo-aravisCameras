******************************
AravisCameras Device (C++)
******************************

Overview
========

The AravisCamera class provides an interface to GigE Vision and USB3
Vision cameras in the Karabo framework environment. It enables users to
configure the camera settings, acquire images, and dynamically update the
settings during runtime. This class simplifies the interaction with the camera
and streamlines the process making it easier to integrate the camera into
various applications.

Contact
========

For questions, please contact opensource@xfel.eu.

License and Contributing
=========================

This software is released by the European XFEL GmbH as is and without any
warranty under the GPLv3 license.
If you have questions on contributing to the project, please get in touch at
opensource@xfel.eu.

External contributors, i.e. anyone not contractually associated to
the European XFEL GmbH, are asked to sign a Contributor License
Agreement (CLA):

- people contributing as individuals should sign the Individual CLA
- people contributing on behalf of an organization should sign
  the Entity CLA.

The CLAs can be found in the `contributor_license_agreement.md` and
`entity_contributor_license_agreement.md` documents located in
the root folder of this repository.
Please send signed CLAs to opensource [at] xfel.eu. We'll get in
touch with you then.
We ask for your understanding that we cannot accept external
contributions without a CLA in place. Importantly, with signing the CLA
you acknowledge that

* European XFEL retains all copyrights of the Karabo Aravis Cameras,
* European XFEL may relicense the Karabo Aravis Cameras under other
  appropriate open source licenses which the Free Software Foundation
  classifies as Free Software licenses.

However, you are welcome to already
suggest modifications you'd like to contribute by opening a merge/pull
request before you send the CLA.

Dependencies
============

- aravis (https://github.com/AravisProject/aravis)

  - meson (https://pypi.org/project/meson/)
  - ninja (https://pypi.org/project/ninja/)

- imageSource (https://git.xfel.eu/karaboDevices/imageSource)

  - opencv (https://opencv.org/)

Host Setup
==========

In order to enable users to access a USB3V camera, a file
``/etc/udev/rules.d/69-basler-cameras.rules`` must be added with the following
content


    ``# Enable user access to all basler cameras``

    ``SUBSYSTEM=="usb", ATTRS{idVendor}=="2676", MODE:="0666", TAG+="uaccess", TAG+="udev-acl"``

Compiling
=========

1. Source activate the Karabo installation against which the device will be
   developed:

    ``cd <Karabo installation root directory>``

    ``source ./activate``

2. Go to the device source root directory and generate its build files with
   cmake:

     ``cd $KARABO/devices/aravisCameras``

     ``mkdir build``

     ``cd build``

     ``cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KARABO/extern ..``

   CMAKE_BUILD_TYPE can also be set to "Release".

3. Build the device:

     ``cd $KARABO/devices/aravisCameras``

     ``cmake --build .``

   ``make`` can also be used as long as the Makefile generator is used by
   cmake.

Testing
=======

After a successfull build, a shared library is generated here:

``dist/<configuration>/<system>/libaravisCameras.so``

And a soft-link to the ``libaravisCameras.so`` file is created in the
``$KARABO/plugins`` folder.

To run the tests, go to the tests directory in your build tree and use
``ctest``:

    ``cd $KARABO/devices/aravisCameras/build/aravisCameras``

    ``ctest -VV``

Running
=======

If you want to manually start a server using this device, simply type:

``karabo-cppserver serverId=cppServer/1 deviceClasses=AravisCameras``

Or just use (a properly configured):

``karabo-start``
