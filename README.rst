******************************
AravisCameras Device (C++)
******************************

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
