# Setup the project
#
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
SET(PROJECT_NAME geant4-boilerplate)
project(${PROJECT_NAME})

# Set this to the proper PSF directory to make a valid symlink on running 'make install'
SET(PSF_PATH ../PSF)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(${PROJECT_NAME} main.cc ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} ${Geant4_LIBRARIES})
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(SCRIPTS
  )

foreach(_script ${SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to '<project_name>' directory under CMAKE_INSTALL_PREFIX
file(GLOB input_files
    ${PROJECT_SOURCE_DIR}/*.in
    ${PROJECT_SOURCE_DIR}/*.mac
    ${PROJECT_SOURCE_DIR}/mcgeo*.txt
    ${PROJECT_SOURCE_DIR}/tracked_beamlets.txt
)
install(FILES
    ${input_files}
    DESTINATION .
)
install(FILES
    ${PROJECT_SOURCE_DIR}/batch_runs.sh
    DESTINATION .
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
# install(DIRECTORY
#     ${PROJECT_SOURCE_DIR}/analysis
#     DESTINATION ${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    ${PSF_PATH} ${CMAKE_INSTALL_PREFIX}/PSF)"
)
