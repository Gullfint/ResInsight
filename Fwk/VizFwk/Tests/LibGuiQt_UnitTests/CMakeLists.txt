cmake_minimum_required(VERSION 2.8.12)

project(LibGuiQt_UnitTests)

# Compile flags should already be setup by caller


find_package(OpenGL)

find_package(Qt5 CONFIG COMPONENTS Core)
if (Qt5Core_FOUND)
	message(STATUS "Found Qt5")
	find_package(Qt5 CONFIG REQUIRED Core OpenGL Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
    include(${QT_USE_FILE})
	if (QT4_FOUND)
		message(STATUS "Found Qt4")
	endif(QT4_FOUND)
endif(Qt5Core_FOUND)

include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibIo_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibGuiQt LibViewing LibRender LibGeometry LibIo LibCore)


set(CEE_SOURCE_FILES
cvfqtUtils-Test.cpp 
../../ThirdParty/gtest/gtest-all.cpp
LibGuiQt_UnitTests.cpp
)

if (Qt5Core_FOUND)
    set(QT_LIBRARIES Qt5::Core Qt5::OpenGL Qt5::Widgets)
endif(Qt5Core_FOUND)


if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES} ${CEE_PLATFORM_LIBS})

