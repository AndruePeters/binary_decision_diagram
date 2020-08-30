add_executable(bdd_test main.cpp bdd.cpp)
target_link_libraries(bdd_test PUBLIC project_options project_warnings
        CONAN_PKG::fmt CONAN_PKG::spdlog)