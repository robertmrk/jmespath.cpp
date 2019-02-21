# jmespath.cpp

[![Build Status](https://travis-ci.org/robertmrk/jmespath.cpp.svg?branch=develop)](https://travis-ci.org/robertmrk/jmespath.cpp) 
[![Build status](https://ci.appveyor.com/api/projects/status/9rca8iv5e5yslpmo/branch/develop?svg=true)](https://ci.appveyor.com/project/robertmrk/jmespath-cpp/branch/develop)
[![Coverage Status](https://coveralls.io/repos/robertmrk/jmespath.cpp/badge.svg?branch=develop&service=github)](https://coveralls.io/github/robertmrk/jmespath.cpp?branch=develop)
[![MIT license](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

jmespath.cpp is a C++ implementation of [JMESPath](http://jmespath.org/), a query language for JSON. It can be used to extract and transform elements of a JSON document.

## JMESPath expression example
Input JSON document:
```json
{
  "locations": [
    {"name": "Seattle", "state": "WA"},
    {"name": "New York", "state": "NY"},
    {"name": "Bellevue", "state": "WA"},
    {"name": "Olympia", "state": "WA"}
  ]
}
```
JMESPath expression:
```
locations[?state == 'WA'].name | sort(@) | {WashingtonCities: join(', ', @)}
```
Result of evaluating the expression:
```json
{"WashingtonCities": "Bellevue, Olympia, Seattle"}
```
For more examples take a look at the [JMESPath Tutorial](http://jmespath.org/tutorial.html) or the [JMESPath Examples](http://jmespath.org/examples.html) pages.

## Usage
To use the public functions and classes of jmespath.cpp you should include the header file `#include <jmespath/jmespath.h>`. The public interface is declared in the `jmespath` namespace.
```cpp
#include <iostream>
#include <jmespath/jmespath.h>

namespace jp = jmespath;

int main(int argc, char *argv[])
{
    auto data = R"({
        "locations": [
            {"name": "Seattle", "state": "WA"},
            {"name": "New York", "state": "NY"},
            {"name": "Bellevue", "state": "WA"},
            {"name": "Olympia", "state": "WA"}
        ]
    })"_json;
    jp::Expression expression = "locations[?state == 'WA'].name | sort(@) | "
                                "{WashingtonCities: join(', ', @)}";
    std::cout << jp::search(expression, data) << std::endl;
    return 0;
}
```

## Documentation
http://robertmrk.github.io/jmespath.cpp

## Install

### Requirements
To build, install and use the library you must have [CMake](https://cmake.org/) installed, version 3.8 or later.

### Supported compilers
jmespath.cpp needs a compiler that supports at least the c++14 standard. The currently supported and tested compilers are:
- g++ versions: 6, 7, 8
- Clang versions: 4.0, 5.0, 6.0, 7
- XCode versions: 9.0, 9.3, 10.1
- Visual Studio 2017

### Library dependencies
- [boost](https://www.boost.org/) version 1.65 or later
- [nlohmann_json](https://github.com/nlohmann/json) version 3.4.0 or later

### Install from source

#### Build and install
To get the source code of the library either clone it from [github](https://github.com/robertmrk/jmespath.cpp)
```bash
git clone https://github.com/robertmrk/jmespath.cpp.git
```
or download the [latest release](https://github.com/robertmrk/jmespath.cpp/releases) and extract it.

In the terminal change the current directory to the location of the source code
```bash
cd <path_to_source>/jmespath.cpp
```
Generate the project or makefiles for the build system of your choice with CMake, then build and install the library:
```bash
mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DJMESPATH_BUILD_TESTS=OFF
sudo cmake --build . --target install
```

#### Integration
To use the library in your CMake project you should find the library with `find_package` and link your target with `jmespath::jmespath`:
```cmake
cmake_minimum_required(VERSION 3.8)
project(example)

find_package(jmespath 0.1.0 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} jmespath::jmespath)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
```

### Install with Conan
If you are using [Conan](https://www.conan.io/) to manage your dependencies,
then add the followng remote:

    $ conan remote add robertmrk https://api.bintray.com/conan/robertmrk/conan
    
and add jmespath.cpp/x.y.z@robertmrk/stable to your conanfile.py's requires, where x.y.z is the release version you want to use. Please file issues [here](https://github.com/robertmrk/conan-jmespath.cpp/issues) if you experience problems with the packages.
