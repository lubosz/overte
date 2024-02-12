vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ValveSoftware/openvr
    REF v2.2.3
    SHA512 65c1546825934c4933f37e07ece33eb95dd8fae741c8ac99347fef6ee28143bb816cd60f7d7050e20bfa7c7fc1221373fd6e937f220688ae89ed8c294594eb05
    HEAD_REF master
)

set(VCPKG_LIBRARY_LINKAGE dynamic)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    if(WIN32)
        set(ARCH_PATH "win64")
    else()
        set(ARCH_PATH "linux64")
    endif()
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
    if(WIN32)
        set(ARCH_PATH "win32")
    else()
        set(ARCH_PATH "linux32")
    endif()
else()
    message(FATAL_ERROR "Package only supports x64 and x86 Windows and Linux.")
endif()

if(VCPKG_CMAKE_SYSTEM_NAME AND NOT (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    message(FATAL_ERROR "Package only supports Windows or Linux desktop.")
endif()

file(MAKE_DIRECTORY
    ${CURRENT_PACKAGES_DIR}/lib
    ${CURRENT_PACKAGES_DIR}/bin
    ${CURRENT_PACKAGES_DIR}/debug/lib
    ${CURRENT_PACKAGES_DIR}/debug/bin
)

if(WIN32)
    file(COPY ${SOURCE_PATH}/lib/${ARCH_PATH}/openvr_api.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${SOURCE_PATH}/lib/${ARCH_PATH}/openvr_api.lib DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
    file(COPY
        ${SOURCE_PATH}/bin/${ARCH_PATH}/openvr_api.dll
        ${SOURCE_PATH}/bin/${ARCH_PATH}/openvr_api.pdb
        DESTINATION ${CURRENT_PACKAGES_DIR}/bin
    )
    file(COPY
        ${SOURCE_PATH}/bin/${ARCH_PATH}/openvr_api.dll
        ${SOURCE_PATH}/bin/${ARCH_PATH}/openvr_api.pdb
        DESTINATION ${CURRENT_PACKAGES_DIR}/debug/bin
    )
else()
    file(COPY ${SOURCE_PATH}/lib/${ARCH_PATH}/libopenvr_api.so DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${SOURCE_PATH}/lib/${ARCH_PATH}/libopenvr_api.so DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
    file(COPY
        ${SOURCE_PATH}/bin/${ARCH_PATH}/libopenvr_api.so
        ${SOURCE_PATH}/bin/${ARCH_PATH}/libopenvr_api.so.dbg
        DESTINATION ${CURRENT_PACKAGES_DIR}/bin
    )
    file(COPY
        ${SOURCE_PATH}/bin/${ARCH_PATH}/libopenvr_api.so
        ${SOURCE_PATH}/bin/${ARCH_PATH}/libopenvr_api.so.dbg
        DESTINATION ${CURRENT_PACKAGES_DIR}/debug/bin
    )
endif()

file(COPY ${SOURCE_PATH}/headers DESTINATION ${CURRENT_PACKAGES_DIR})
file(RENAME ${CURRENT_PACKAGES_DIR}/headers ${CURRENT_PACKAGES_DIR}/include)

file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/openvr)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/openvr/LICENSE ${CURRENT_PACKAGES_DIR}/share/openvr/copyright)
