# AravisCameras Device (C++)
![Karabo Badge](https://img.shields.io/badge/Karabo-aravisCameras-blue?style=social&logo=data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAIRlWElmTU0AKgAAAAgABQESAAMAAAABAAEAAAEaAAUAAAABAAAASgEbAAUAAAABAAAAUgEoAAMAAAABAAIAAIdpAAQAAAABAAAAWgAAAAAAAABIAAAAAQAAAEgAAAABAAOgAQADAAAAAQABAACgAgAEAAAAAQAAABigAwAEAAAAAQAAABgAAAAAEQ8YrgAAAAlwSFlzAAALEwAACxMBAJqcGAAAAVlpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IlhNUCBDb3JlIDYuMC4wIj4KICAgPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICAgICAgPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIKICAgICAgICAgICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iPgogICAgICAgICA8dGlmZjpPcmllbnRhdGlvbj4xPC90aWZmOk9yaWVudGF0aW9uPgogICAgICA8L3JkZjpEZXNjcmlwdGlvbj4KICAgPC9yZGY6UkRGPgo8L3g6eG1wbWV0YT4KGV7hBwAAB4lJREFUSA1tVQlwU9cVPX%2FXZmuxZcuSjWyMwQaDsUvLWmo7bEOHLSnETckQBgITWpO2CSmTlqnatEyazNCWpmEZYEpCS7DLpA3FbVPWTAIJ2GZfHNsgL1iWZGuz9S19Sf%2F1fWfSYdpeaeZ96f13l3PPuY%2FZsuWA0CfdZlFWhsDR22pb28EUHrOT2yt2jcrJpWlVcqWUaFEqMcDprTXQm10RNtF%2FNdfQ%2BeLiN0bvPXZk%2FLHW4%2BH1NhvH%2FPeG9vvNG4PGHXd99p3Xji%2FfZn3j93ZrHkZVFhkIAKMDUkGVVSJsJg00%2B585v73i%2BT0vS6PDCYur56mV1YE6hqE7X5gWwFD5zafdtvKaCpImlUl5pJIjqFCM5pLcRMi41%2FV6anIR9W0ATz8cVDDQjsehen1Qfte3WdfElaIkPqSOESZgMFt6ddlZDxgl0clmZXfwjUdOt930DRUakTFF%2FX6M%2BANIyTLUTAZ9LEfez%2FmRUC%2BnhVyavMAQEOpbSbHwj4H9LAHd5VA0Q4LtiFA4IEoOnSA4THbH10IZgqlmmlUKnFU0ZJv0AqvqbHZic5dkFDlOg4wwylic%2B2Uwxey8F4FFL8GpE5EhBB1yAoiNAkYd5hZO5MrLdCqnN6YFSa%2BC40k6EmIu3HnI1q36Bs%2FHlNSVzjPnViS7vCn3E3Ml5%2FQqTjLbkFGSFAuCRckxxKNhhINBDA08wmg4jEpnAXKnToTRagMr6UEYhiWEsDq9Af23buLRgSO00GIms2JhiOd5od9sz8GN0wcYX0cvlr5qRv6kMqR1OgR6e8BxPLLyXLC4iuGunAllNEobTVvHsEiMxDD84C6sBQWw5ORgqNeLwJV2JGFSMd%2FNyWk1wDM879VlmSiyZaS8qhj%2F3H0YmDwFVfWVkD%2B9gs7rXrjnTYO1uAiumhqIhixEB%2FrRc%2FlT9LV7oUS8KJhZDZ%2FMAZ8HqZ8RTDQaSFDgkQIJsYIk9PCShBxE2Ps3OvHK7k349Y4VCN%2FroM79KJnsQs%2Blh7j%2Bp99AHh4Gy%2FNIRKPoPncMlmwB2TnFiJuy8NOty%2FDR1bew81db8SB%2BFVUuO1SWC%2FOEZfoLCp34BEXc6k215OUXnmVyzCY8tWoZdh9sxv6feFA%2Bax7utxKKzBey4UQBkjQXVjaDjuEUDr2yGasXzKTEiOE7y79OHnhfRbfNDYHjo2yByA2yBkMK65exP%2FvBc0SJjyIwOIgiuwVb1i4FapYg2toPkRaf0ZSlGQ2UR1Lo8N7Fih%2BuQVWxAx237%2BDa9Tuwm41YvmgB00ZFIzGIsjkWw7BKyDCsFhj0OlWWxzAYGEZz8wcoddjw7i82w4cgyqAgnc6M%2B%2BdYFn3K55Qo89CweB7I2Ag4UURl1XRkOAnJRBIOWiUYEmW%2FXzszSsXhAysgGhtRbTkWHD58AuXlZcjONmHJnBn49s5G3EE7skQeVAYQqTPAhKcbFqK6rAjuCS44CvIRDkUotQE5HmcHKa15UYyyFFciZFKPcOEmzXxIpWFRUuwET1mgqiosJj1qZ5QCdd9DfukkpKk%2BDDYb8jZvhSNLD53AYcAXxIWLn%2BHGjbuIyQly9szHTFlkCGlWiLJazVQovRp13953nPTSl1etXIre3gGEwxGwHAdWGxB5FjACLZuWQHUFSa8fz1Y7393thd8fRGGhA5%2B03sTfmvaj1JkHJZ2JjQeQVaZ3%2BdxKtPzlIHP67CUUuSdgWsUkdHY%2BBE%2FxNpuzKb3HQOh80pyDqHSuEiRSaToZeOTbbVi5vA7Fk0px7OSHNKQFyaRCURAj4wF4ke%2BP0D%2FMFJ9dLx4ll9pvw1lUiK%2FOqkJfIIQP%2FnERViFDhaMpmLKJBnFLPPZ7jqG55TyKSidCpYrfd%2BQ9nP%2FzvrTZOBWixQomkx6j8qNMXLvJSjuyUVb0fNh7LX300DlIFpYR9Abm5789ivfe8qBQpmp35cFI8U%2FEIuj61zlEHj3Ah6daESWjePfURfX4xfuZKdVzhOr1a%2BDIsb6zcIp5v9Z0eDwetrt4weZ4PL4r1NNTqFHurC%2BQxuG%2FU%2FBjQulXytHddg3T161BWX09Yo96cea1A8iblAcmMAp%2FuVuZv2Sh6HS5wMgjQ26XY9ubDU80a7413jEehqHswUFP68CxLlPWS4qiND43o9ouz6rBpUPvJLrbung6rHmGNlzDX1utJXYEuryqmOVQVjQ8qTMaKaOSoyfqqmq%2Bu2FRxTCwliOkibC0azRLoLGlRfLMcsrHNtS%2FJqrxqnhgcA8RhPi0b63W2adP4JP4OJGKyxqF6JdB%2BGGnwjmd6vzGdTo9q%2FrsJPHMH7Y3NGjOG%2Fe2SPQy1VRJxiHSAmhW6znPw%2B3lL2zcSG8UYP3bf52SFsQdY7K8yd%2FZhajPl6mor1Pl4SDpuXVLnDJ7NnKzjX%2BcnZ%2F%2F0vOr5vixtonzTAPn8axTtPP0jqCp%2FB9b29QkBu%2Fa1QueuvHhs%2BXk5ZpYOPpjJZV%2BkvISybE4DKoSKMrN3bbn2cUnx13s3St5QqEU7acG97hpAb58%2Ft%2BVbm450EqV9R89YcOJj%2BY37DvV%2BsLB90%2B1tHfatUNNTU3cXgrvl8609bFn7t8fxThrSai%2FQAAAAABJRU5ErkJggg%3D%3D)
![PyPI - Version](https://img.shields.io/pypi/v/karabo-aravis-cameras)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/karabo-aravis-cameras)
![PyPI - License](https://img.shields.io/pypi/l/karabo-aravis-cameras)
![PyPI - Wheel](https://img.shields.io/pypi/wheel/karabo-aravis-cameras)
![GitHub language count](https://img.shields.io/github/languages/count/European-XFEL/Karabo-aravisCameras)
![GitHub top language](https://img.shields.io/github/languages/top/European-XFEL/Karabo-aravisCameras)
![GitHub contributors](https://img.shields.io/github/contributors/European-XFEL/Karabo-aravisCameras)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/European-XFEL/Karabo-aravisCameras)
![GitHub Repo stars](https://img.shields.io/github/stars/European-XFEL/Karabo-aravisCameras)

## Overview

The AravisCamera class provides an interface to GigE Vision and USB3
Vision cameras in the Karabo framework environment. It enables users to
configure the camera settings, acquire images, and dynamically update the
settings during runtime. This class simplifies the interaction with the camera
and streamlines the process making it easier to integrate the camera into
various applications.

## Contact

For questions, please contact opensource@xfel.eu.

## License and Contributing

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

- European XFEL retains all copyrights of the Karabo Aravis Cameras,
- European XFEL may relicense the Karabo Aravis Cameras under other
  appropriate open source licenses which the Free Software Foundation
  classifies as Free Software licenses.

However, you are welcome to already
suggest modifications you'd like to contribute by opening a merge/pull
request before you send the CLA.

## Source Code

The sources for this project can be found at 
https://github.com/European-XFEL/Karabo-aravisCameras

## Dependencies

- aravis (https://github.com/AravisProject/aravis)

  - meson (https://pypi.org/project/meson/)
  - ninja (https://pypi.org/project/ninja/)

- imageSource (https://git.xfel.eu/karaboDevices/imageSource or
  https://github.com/European-XFEL/imageSource)

  - opencv (https://opencv.org/)

## Host Setup

In order to enable users to access a USB3V camera, a file
``/etc/udev/rules.d/69-basler-cameras.rules`` must be added with the following
content

```
# Enable user access to all basler cameras
SUBSYSTEM=="usb", ATTRS{idVendor}=="2676", MODE:="0666", TAG+="uaccess", TAG+="udev-acl"
```

## Installing and Running from PyPI

The device can further be installed from PyPI and ships in the form of 
a self-hosting device server. To install, run

```
pip install karabo-aravis-cameras
```

To run, make sure you have activated a Karabo environment using

```
source PATH/TO/KARABO/activate
```

and then start the self-hosting server with

```
karabo-aravis-cameras-server [serverId=, ... karabo-cpp-server options]
```

You can directly start a device connected to a network camera using e.g.

```
karabo-aravis-cameras-server \
   serverId=MyServer 
   init='{"MyCamera": {"classId": "AravisBaslerCamera", "cameraId": "192.168.0.1"}}'
```


## Compiling in a self-contained Karabo Environment

1. Source activate the Karabo installation against which the device will be
   developed:

   ```
   cd <Karabo installation root directory>
   source ./activate
   ```

2. Go to the device source root directory and generate its build files with
   cmake:

   ```
   cd $KARABO/devices/aravisCameras
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KARABO/extern ..
   ```

   ``CMAKE_BUILD_TYPE`` can also be set to "Release".

3. Build the device:

   ```
   cd $KARABO/devices/aravisCameras
   cmake --build .
   ```

   ``make`` can also be used as long as the Makefile generator is used by
   cmake.

## Testing

After a successfull build, a shared library is generated here:

``dist/<configuration>/<system>/libaravisCameras.so``

And a soft-link to the ``libaravisCameras.so`` file is created in the
``$KARABO/plugins`` folder.

To run the tests, go to the tests directory in your build tree and use
``ctest``:

```
cd $KARABO/devices/aravisCameras/build/aravisCameras
ctest -VV
```

## Running

If you want to manually start a server using this device, simply type:

```
karabo-cppserver serverId=cppServer/1 deviceClasses=AravisCameras
```

Or just use (a properly configured):

```
karabo-start
```
