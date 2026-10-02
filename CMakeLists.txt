cmake_minimum_required(VERSION 3.16)
project(fcitx5-semoi VERSION 0.1.0 LANGUAGES CXX)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(GNUInstallDirs)
find_package(Fcitx5Core 5.1 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fcitx 와 무관한 모아치기 엔진 (테스트에서도 씀)
add_library(semoi-core STATIC src/semoi.cpp)
set_target_properties(semoi-core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(semoi-core PUBLIC src)

add_fcitx5_addon(semoi src/engine.cpp)
target_link_libraries(semoi Fcitx5::Core semoi-core)
install(TARGETS semoi DESTINATION "${FCITX_INSTALL_ADDONDIR}")
install(FILES data/semoi-addon.conf RENAME semoi.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
install(FILES data/semoi.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  enable_testing()
  add_executable(test_chords test/test_chords.cpp)
  target_link_libraries(test_chords semoi-core)
  add_test(NAME chords COMMAND test_chords "${CMAKE_CURRENT_SOURCE_DIR}/test/chords.tsv")

  # fcitx5 Instance 위에서 끝까지 확인하는 테스트: 빌드 디렉터리의 libsemoi.so 와 testdata/ 의 설정을 읽는다.
  # testfrontend 모듈을 개발 패키지에 넣지 않는 배포판도 있어서, 없으면 건너뛴다.
  find_package(Fcitx5ModuleTestFrontend QUIET)
  if(Fcitx5ModuleTestFrontend_FOUND)
    configure_file(data/semoi-addon.conf "${CMAKE_BINARY_DIR}/testdata/addon/semoi.conf" COPYONLY)
    configure_file(data/semoi.conf "${CMAKE_BINARY_DIR}/testdata/inputmethod/semoi.conf" COPYONLY)
    add_executable(testengine test/testengine.cpp)
    target_compile_definitions(testengine PRIVATE TESTING_BINARY_DIR="${CMAKE_BINARY_DIR}")
    target_link_libraries(testengine Fcitx5::Core Fcitx5::Module::TestFrontend)
    add_dependencies(testengine semoi)
    add_test(NAME engine COMMAND testengine)
  else()
    message(STATUS "Fcitx5ModuleTestFrontend 없음: engine 테스트를 건너뜀")
  endif()
endif()
