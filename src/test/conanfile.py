from conans import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class KaraboDeviceConan(ConanFile):
    generators = "CMakeToolchain", "CMakeDeps"
    settings = "os", "compiler", "build_type", "arch"

    requires = ( "gtest/1.11.0",
                 "libjpeg/9e",
    )

    def requirements(self):
        self.requires(self.tested_reference_str)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)

    def test(self):
        cmake = CMake(self)
        cmake.test(build_tool_args=["ARGS=-V"])
