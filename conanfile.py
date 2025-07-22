import os, re, pathlib
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy


class PlatformConan(ConanFile):
    name = "pyclassifiers"
    version = "X.X.X"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "enable_testing": [True, False],
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "enable_testing": False,
        "shared": False,
        "fPIC": True,
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "pyclfs/*", "tests/*", "LICENSE"

    def set_version(self) -> None:
        cmake = pathlib.Path(self.recipe_folder) / "CMakeLists.txt"
        text = cmake.read_text(encoding="utf-8")

        match = re.search(
            r"""project\s*\([^\)]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)""",
            text,
            re.IGNORECASE | re.VERBOSE,
        )
        if match:
            self.version = match.group(1)
        else:
            raise Exception("Version not found in CMakeLists.txt")
        self.version = match.group(1)

    def requirements(self):
        self.requires("libtorch/2.7.1")
        self.requires("nlohmann_json/3.11.3")
        self.requires("folding/1.1.2")
        self.requires("fimdlp/2.1.1")
        self.requires("bayesnet/1.2.1")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.30]")
        self.test_requires("catch2/3.8.1")
        self.test_requires("arff-files/1.2.1")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if self.options.enable_testing:
            # Run tests only if we're building with testing enabled
            self.run("ctest --output-on-failure", cwd=self.build_folder)

    def package(self):
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["PyClassifiers"]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_target_name", "pyclassifiers::pyclassifiers")

        # Add compiler flags that might be needed
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ["pthread"]
