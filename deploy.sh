#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DEPLOY_CMAKE_DIR="${SCRIPT_DIR}/deploy_cmake"
DEPLOY_BUILD_DIR="${SCRIPT_DIR}/deploy-build"
PACKAGE_DIR="${SCRIPT_DIR}/HerdmindSimulatorPackage"

rm -rf DEPLOY_BUILD_DIR
rm -rf PACKAGE_DIR

QT_VERSION="6.10.2"
QT_ROOT="${HOME}/Qt/${QT_VERSION}/gcc_64"

APP_NAME="HerdmindSimulator"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 [debug|release]"
    exit 1
fi

case "${1,,}" in
    debug)
        BUILD_TYPE="Debug"
        ;;
    release)
        BUILD_TYPE="Release"
        ;;
    *)
        echo "Invalid argument: $1"
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

# Convert 6.10.2 → 6_10_2 (Qt Creator build directory format)
QT_VERSION_UNDERSCORE="${QT_VERSION//./_}"

APP_EXE="${SCRIPT_DIR}/build/Desktop_Qt_${QT_VERSION_UNDERSCORE}-${BUILD_TYPE}/${APP_NAME}"

echo "App        : ${APP_NAME}"
echo "Build type : ${BUILD_TYPE}"
echo "Qt version : ${QT_VERSION}"
echo "Qt root    : ${QT_ROOT}"
echo "Executable : ${APP_EXE}"

if [[ ! -d "${QT_ROOT}" ]]; then
    echo "Qt installation not found:"
    echo "  ${QT_ROOT}"
    exit 1
fi

if [[ ! -x "${APP_EXE}" ]]; then
    echo "Executable not found:"
    echo "  ${APP_EXE}"
    exit 1
fi

rm -rf "${DEPLOY_BUILD_DIR}" "${PACKAGE_DIR}"

cmake -S "${DEPLOY_CMAKE_DIR}" -B "${DEPLOY_BUILD_DIR}" \
    -DQT_ROOT="${QT_ROOT}" \
    -DAPP_EXE="${APP_EXE}" \
    -DAPP_NAME="${APP_NAME}" \
    -DCMAKE_INSTALL_PREFIX="${PACKAGE_DIR}"

cmake --install "${DEPLOY_BUILD_DIR}"

echo
echo "Deployment finished."
echo "Package directory:"
echo "  ${PACKAGE_DIR}"
echo
echo "Run with:"
echo "  ${PACKAGE_DIR}/bin/${APP_NAME}"