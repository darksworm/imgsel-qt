cmake_minimum_required(VERSION 3.4)

project(imgsel
        VERSION 0.1.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_MODULE_PATH ${IMGSEL_SOURCE_DIR}/cmake)

configure_file(util/config/project_config.h.in ${PROJECT_BINARY_DIR}/config/project_config.h)

add_executable(${PROJECT_NAME} main.cpp gui/picker/ImagePickerDrawer.cpp gui/picker/ImagePickerDrawer.h
        util/exceptions/OutOfBounds.h gui/drawer/ShapeDrawer.h gui/util/Shape.h gui/util/Dimensions.h
        input/handler/instruction/InputInstructionType.h input/handler/filters/FilteringInputHandler.h
        input/handler/InputHandler.cpp input/handler/filters/FilteringInputHandler.cpp
        input/handler/filters/TextFilteringInputHandler.cpp input/handler/filters/TextFilteringInputHandler.h
        input/handler/InputHandlerFactory.cpp input/handler/InputHandlerFactory.h
        input/handler/SelectionInputHandler.cpp input/handler/SelectionInputHandler.h
        input/handler/instruction/InputInstruction.h input/handler/instruction/MoveInstruction.h
        input/handler/instruction/ModeChangeInstruction.h input/handler/instruction/FilterInstruction.h
        util/config/ConfigManager.cpp util/config/ConfigManager.h util/config/Config.h util/config/ConfigBuilder.h
        gui/drawer/ImageDrawer.cpp gui/drawer/ImageDrawer.h input/handler/instruction/CopyInstruction.h
        util/exceptions/ImageNotLoadable.h gui/picker/ImagePickerMove.h util/lib/CLI11.hpp Application.cpp
        Application.h)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/config)

find_package(Qt5Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

IF (UNIX AND NOT APPLE)
    find_package(X11)
    if (X11_FOUND)
        find_package(Qt5X11Extras REQUIRED)
        find_package(TCL)

        IF (TK_FOUND)
            FIND_FILE(TKPORT tkPort.h PATHS ${TK_INCLUDE_PATH})
            IF(TKPORT)
                add_definitions(-DWITH_X11)
                include_directories(${TK_INCLUDE_PATH})
            else()
                message(WARNING "Could not find tkPort.h, this might mean that the created window will not be positioned correctly!")
            endif()
        endif()

        target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} Qt5::X11Extras ${TK_LIBRARY})
    endif ()

    set(WHICH_CMD "which")
    set(WHICH_ARG "xclip")

    execute_process(COMMAND ${WHICH_CMD} ${WHICH_ARG} RESULT_VARIABLE ret)
    if (ret GREATER 0)
        message(FATAL_ERROR "Could not find xclip! Please install xclip!")
        return()
    endif ()
endif ()

install(TARGETS ${PROJECT_NAME} EXPORT imgsel DESTINATION bin)

set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_NAME imgsel
        COMPILE_FLAGS "${WARNING_FLAGS}"
)
