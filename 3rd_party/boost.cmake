set(DISPLAY_STRING "# BOOST          # BOOST          # BOOST          # BOOST          # BOOST")
message(STATUS "${DISPLAY_STRING}")
cmake_minimum_required(VERSION 3.10)

set(LIB_TARGET_NAME                                       boost)
#==========================================================
string(TOUPPER ${LIB_TARGET_NAME} TARGET_CNAME)

# ---------------------------------------------------------------------------
option(USE_SYSTEM_${TARGET_CNAME} "Should we use the system ${LIB_TARGET_NAME}?" OFF)

set(${TARGET_CNAME}_VERSION "1.75.0")
set(XCSOAR_${TARGET_CNAME}_VERSION "${LIB_TARGET_NAME}-${${TARGET_CNAME}_VERSION}")  # reset!
set(${TARGET_CNAME}_INSTALL_DIR "${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}")
set(${TARGET_CNAME}_PREFIX "${EP_CMAKE}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}")

if(MSVC)
    set(TOOLSET msvc)
    set(TOOLSETNAME msvc2019)
else()
    set(TOOLSET gcc)
    set(TOOLSETNAME ${TOOLCHAIN})
endif()


# set(${TARGET_CNAME}_BUILD_CMD "cd ../../src/${LIB_TARGET_NAME} & .\\b2 -j4 toolset=${TOOLSET} variant=release link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install")
# set(${TARGET_CNAME}_BUILD_CMD "cd ../../src/build & echo %CD% & ./b2 -j4 toolset=${TOOLSET} variant=release link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install")
# set(${TARGET_CNAME}_BUILD_CMD "cd ../../src/build & echo %CD% & ./b2 -j4 toolset=${TOOLSET} link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install")
set(${TARGET_CNAME}_BUILD_CMD "./b2 -j4 toolset=${TOOLSET} link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install")


set(INSTALL_DIR "${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}")

# if (ON)  # MSVC)  # in this moment not for MinGW enabled!!
if (MSVC)  # in this moment not for MinGW enabled!!
#===========================================
#-------------------
if(NOT EXISTS "${INSTALL_DIR}")

## b2 -j4 toolset=msvc link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=D:/link_libs/boost/boost-1.75.0 --build-dir=D:/Projects/3rd_Party/boost/build/msvc2019 --with-chrono --with-system --with-filesystem --with-headers --with-regex --with-date_time  --includedir=D:/link_libs/boost/boost-1.75.0/include --libdir=D:/link_libs/boost/boost-1.75.0/lib/msvc2019 install
message(STATUS "### ${EP_CMAKE}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/src/${LIB_TARGET_NAME}")
message(STATUS "### ${${TARGET_CNAME}_BUILD_CMD}")


#-------------------
ExternalProject_Add(
   ${LIB_TARGET_NAME}
   GIT_REPOSITORY        "https://github.com/boostorg/boost.git"
   GIT_TAG               "${XCSOAR_${TARGET_CNAME}_VERSION}"

   PREFIX                "${${TARGET_CNAME}_PREFIX}"
#   BINARY_DIR            "${${TARGET_CNAME}_PREFIX}/build/${TOOLCHAIN}"
   INSTALL_DIR           "${INSTALL_DIR}"

   # PATCH_COMMAND         "bootstrap"
   CONFIGURE_COMMAND     bootstrap     ##  "${EP_CMAKE}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/src/${LIB_TARGET_NAME}/TestOutput.cmd"

   # BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Starting $<CONFIG> build"
   # COMMAND       "cd ../../src/${LIB_TARGET_NAME} & b2.exe -j4 toolset=${TOOLSET} variant=release link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install"
   # BUILD_COMMAND         
   # CONFIGURE_COMMAND     "../../src/${LIB_TARGET_NAME}/TestOutput.cmd ${LIB_TARGET_NAME} ${TOOLSET} ${TOOLSETNAME} ${XCSOAR_${TARGET_CNAME}_VERSION}"
#  BUILD_COMMAND "cd ../../src/${LIB_TARGET_NAME} & b2.exe -j4 toolset=${TOOLSET} variant=release link=static runtime-link=shared threading=multi address-model=64 --layout=versioned --prefix=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION} --build-dir=D:/Projects/3rd_Party/${LIB_TARGET_NAME}/build/${TOOLSETNAME} --with-chrono --with-system --with-filesystem --with-headers --with-date_time  --includedir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/include --libdir=${LINK_LIBS}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/lib/${TOOLSETNAME} install"
   BUILD_COMMAND "${${TARGET_CNAME}_BUILD_CMD}"

##     CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
##     "-DINSTALL_BIN_DIR:PATH=<INSTALL_DIR>/bin/${TOOLCHAIN}"
##     "-DINSTALL_LIB_DIR:PATH=<INSTALL_DIR>/lib/${TOOLCHAIN}"
    # BUILD_ALWAYS ${EP_BUILD_ALWAYS}
#    BUILD_ALWAYS ON
    BUILD_IN_SOURCE ON  ## ${EP_BUILD_IN_SOURCE}
    # WORKING_DIRECTORY ${EP_CMAKE}/${LIB_TARGET_NAME}/${XCSOAR_${TARGET_CNAME}_VERSION}/src/${LIB_TARGET_NAME}  # = src_dir!
)
#===========================================
set_target_properties(${LIB_TARGET_NAME} PROPERTIES FOLDER External)

endif()

endif(MSVC)
set(${TARGET_CNAME}_LIB  "${INSTALL_DIR}/lib/${TOOLCHAIN}/${PRE_LIB}${LIB_TARGET_NAME}.${LIB_EXTENSION}")
set(${TARGET_CNAME}_INCLUDE_DIR  "${INSTALL_DIR}/include")
# PARENT_SCOPE only available in Parent, not here...
if(EXISTS "${${TARGET_CNAME}_LIB}")
  set(${TARGET_CNAME}_LIB  ${${TARGET_CNAME}_LIB} PARENT_SCOPE)
else()
  set(${TARGET_CNAME}_LIB  ${LIB_TARGET_NAME} PARENT_SCOPE)
endif()
set(${TARGET_CNAME}_INCLUDE_DIR  ${${TARGET_CNAME}_INCLUDE_DIR} PARENT_SCOPE)

set(THIRDPARTY_INCLUDES ${THIRDPARTY_INCLUDES} ${${TARGET_CNAME}_INCLUDE_DIR})

