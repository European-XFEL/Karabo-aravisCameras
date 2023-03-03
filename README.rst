******************************
AravisCameras Device (C++)
******************************

Compiling with Conan
====================

1. Make a build directory::

     mkdir build
     cd build

2. Install the compile-time and run-time dependencies for this project into the local conan cache. This step also generates a cmake toolchain file that will be used by the project to find these dependencies::

     conan install .. --remote=gitlab

3. Compile the project. This step compiles with cmake using the generated toolchain file from the previous step. To control the cmake build options from the conan build command, use the -o arguments (see conanfile.py for which -o options are implemented)::

     conan build .. <-o build_option1=True -o build_option2=False ...>


Testing with Conan
==================

1. There is a way to have conan execute cmake tests. Coming soon ....


Installing with Conan
=====================

1. Create a conan package of the compiled project and install it into the local conan cache. This step uses the output of cmake install to figure out which files are part of the project package. The project is For this step, a version number is needed (1.0.0 here as example)::

     conan export-pkg .. aravisCameras/1.0.0@karaboDevices+aravisCameras/any

2. Install the project and all its dependencies (including the Karabo framework) into the /home/<username>/karabo-fromconan folder. Inspect the folder and it should look like a karabo framework deployment along with your project dependencies and plugins::

     conan install aravisCameras/1.0.0@karaboDevices+aravisCameras/any --install-folder ~/karabo-fromconan --remote=gitlab

``The above conan install command can also be used as a one-line command to install a pre-compiled binary and karabo framework from XFEL's gitlab package repository. Browse the Packages & Registries menu option for this project to see available pre-compiled package versions.``


Running
=======

If you want to manually start a server using this device, activate your Karabo installation in /home/<username>/karabo-fromconan folder and run the cppserver::

     karabo-cppserver serverId=cppServer/1 deviceClasses=AravisCameras

Or just use (a properly configured)::

     karabo-start
