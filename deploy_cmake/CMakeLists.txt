cmake_minimum_required(VERSION 3.22)
project(QtQmakeDeploy LANGUAGES CXX)

# Inputs from deploy.sh
set(QT_ROOT "" CACHE PATH "Path to Qt 6.10.x gcc_64 root")
set(APP_EXE "" CACHE FILEPATH "Absolute path to the built executable")
set(APP_NAME "" CACHE STRING "Application file name")

if(NOT QT_ROOT)
    message(FATAL_ERROR "QT_ROOT not set")
endif()

if(NOT APP_EXE)
    message(FATAL_ERROR "APP_EXE not set")
endif()

if(NOT EXISTS "${APP_EXE}")
    message(FATAL_ERROR "Executable not found: ${APP_EXE}")
endif()

if(NOT APP_NAME)
    get_filename_component(APP_NAME "${APP_EXE}" NAME)
endif()

list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT}")

find_package(Qt6 6.10 REQUIRED COMPONENTS Core)
include(GNUInstallDirs)

install(PROGRAMS "${APP_EXE}"
        DESTINATION "${CMAKE_INSTALL_BINDIR}")

qt_generate_deploy_script(
    OUTPUT_SCRIPT deploy_script
    CONTENT "
include(\"${QT_DEPLOY_SUPPORT}\")

message(STATUS \"Deploying runtime dependencies\")

qt_deploy_runtime_dependencies(
    EXECUTABLE \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${APP_NAME}\"
    GENERATE_QT_CONF
    VERBOSE
)
"
)

install(SCRIPT "${deploy_script}")