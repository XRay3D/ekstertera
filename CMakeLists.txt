cmake_minimum_required(VERSION 3.5)

project(ekstertera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

find_package(QT NAMES
    # Qt6
    Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

#set(PROJECT_SOURCES
#main.cpp
#mainwindow.cpp
#mainwindow.h
#mainwindow.ui
#)

set(DIRS "src")#"3dparty/json")
foreach(X IN LISTS DIRS)
    file(GLOB_RECURSE PROJECT_SOURCES
        ${X}/*.h
        ${X}/*.cpp
        ${X}/*.ui
        ${X}/*.qrc
        ${PROJECT_SOURCES})
endforeach()

include_directories(src)



##################################################
# # Change 'myapp_en' to be the base file name of the qrc file.
# set(TRANS_FILE resource)
# # set( TRANS_SRCFILE src/resource)
# # message("==>" ${TRANS_SRCFILE})
# # get_filename_component(QRC_FILE ${TRANS_SRCFILE} NAME_WE)
# # message("==>" ${QRC_FILE})
# set(TRANS_SRCFILE ${CMAKE_CURRENT_SOURCE_DIR}/src/${TRANS_FILE}.qrc)
# set(TRANS_INFILE ${CMAKE_CURRENT_BINARY_DIR}/${TRANS_FILE}.qrc)
# set(TRANS_OUTFILE ${CMAKE_CURRENT_BINARY_DIR}/qrc_${TRANS_FILE}.cxx)

# # Copy the QRC file to the output directory, because the files listed in the
# # qrc file are relative to that directory.
# add_custom_command(
#     OUTPUT ${TRANS_INFILE}
#     COMMAND ${CMAKE_COMMAND} -E copy ${TRANS_SRCFILE} ${TRANS_INFILE}
#     MAIN_DEPENDENCY ${TRANS_SRCFILE}
# )

# # Run the resource compiler (rcc_options should already be set). We can't
# # use QT4_ADD_RESOURCES because the qrc file may not exist yet.
# # Use ${Qt5Core_RCC_EXECUTABLE} instead of ${QT_RCC_EXECUTABLE} if you use Qt 5.
# add_custom_command(
#     OUTPUT ${TRANS_OUTFILE}
#     COMMAND ${Qt5Core_RCC_EXECUTABLE}
#     ARGS ${rcc_options} -name ${TRANS_FILE} -o ${TRANS_OUTFILE} ${TRANS_INFILE}
#     MAIN_DEPENDENCY ${TRANS_INFILE}
#     DEPENDS ${QM_FILES}
# )

# # Add compiled resources to executable dependency list
# # add_executable( ${APP_NAME} ... ${TRANS_OUTFILE} )
# set(PROJECT_SOURCES ${TRANS_OUTFILE} ${PROJECT_SOURCES})
##################################################

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/sysheaders.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# include(GNUInstallDirs)
# install(TARGETS ${PROJECT_NAME}
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
