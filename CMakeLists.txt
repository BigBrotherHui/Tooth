cmake_minimum_required(VERSION 3.22)
project(Demo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(MITK 2023.04 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(ITK REQUIRED)
find_package(VTK REQUIRED)
file(GLOB_RECURSE SOURCE_FILES "*.cpp")
file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE UI_FILES "*.ui")
file(GLOB_RECURSE QRC_FILES "*.qrc")

set(PROJECT_SOURCES
        ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES} ${QRC_FILES}
)

source_group("Header Files" REGULAR_EXPRESSION ".h$")
source_group("Source Files" FILES ${SRC_FILES})
source_group("Forms" REGULAR_EXPRESSION ".ui$")
source_group("Generated Files" REGULAR_EXPRESSION "ui_.*\\.h$|moc.*\\.cpp$|\\.stamp$|\\.rule$")
include_directories(${CMAKE_SOURCE_DIR}/CommonHeader)
#add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core ${ITK_LIBRARIES} ${VTK_LIBRARIES}
MitkCore MitkQtWidgets MitkDataTypesExt
)
