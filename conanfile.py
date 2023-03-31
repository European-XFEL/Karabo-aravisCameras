from conans import ConanFile
from conans.tools import collect_libs
from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.layout import cmake_layout


class KaraboDeviceConan(ConanFile):
    generators = "CMakeToolchain", "CMakeDeps"
    settings = "os", "compiler", "build_type", "arch"

    name = "aravisCameras"
    requires = ( "karaboFramework/2.16.4@karaboDevices+depends/any",
                 "glib/2.75.2@karaboDevices+depends/any",
                 "aravis/0.8.20@karaboDevices+depends/any",
                 "imageSource/0.9.9@karaboDevices+imageSource/any",
    )
    options = { "build_tests": [True, False] }
    default_options = { "build_tests": False }

    def export_sources(self):
        self.copy("CMakeLists.txt")
        self.copy("cmake/*")
        self.copy("src/*")
        # git refs needed to generate version.hh, self.copy ignores .git/
        self.run(f"cp -r .git/ {self.export_sources_folder}")

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["BUILD_TESTS"] = eval(self.options.build_tests._value)
        toolchain.variables["Boost_NO_BOOST_CMAKE"] = True
        toolchain.variables["Boost_NO_SYSTEM_PATHS"] = True
        toolchain.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include", "include/aravisCameras"]
        self.cpp_info.libdirs = ["plugins"]
        self.cpp_info.libs = collect_libs(self)

    def deploy(self):
        self.copy("*", dst=self.install_folder)
        self.copy_deps("*", dst=self.install_folder)
