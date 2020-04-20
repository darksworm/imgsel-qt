cmake_minimum_required(VERSION 3.8)

project(emojigun VERSION 1.3.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_MODULE_PATH ${EMOJIGUN_SOURCE_DIR}/cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/util/lib/QHotkey/QHotkey)
    message(STATUS "not exists")
    find_package(Git)

    if(NOT GIT_FOUND)
        message(FATAL_ERROR "git not found, you might have to initialize modules manually")
    endif()

    execute_process(COMMAND git submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    else()
        message(STATUS "initialized modules")
    endif()
endif()

set(ADD_EXECUTABLE_PRE "")

if (WIN32)
    set(ADD_EXECUTABLE_PRE WIN32 ${PROJECT_BINARY_DIR}/config/version.rc)
endif ()

set(hotkey_SRCS util/lib/QHotkey/QHotkey/qhotkey.cpp)

if (WIN32)
    list(APPEND hotkey_SRCS util/lib/QHotkey/QHotkey/qhotkey_win.cpp)
endif()

if (APPLE)
    list(APPEND hotkey_SRCS util/lib/QHotkey/QHotkey/qhotkey_mac.cpp)
endif()

IF (UNIX AND NOT APPLE)
    list(APPEND hotkey_SRCS util/lib/QHotkey/QHotkey/qhotkey_x11.cpp)
endif()

add_executable(${PROJECT_NAME} ${ADD_EXECUTABLE_PRE} main.cpp gui/picker/ImagePickerDrawer.cpp
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
        Application.h util/validators/IntXIntValidator.h util/StringTools.h util/Size.h
        input/handler/instruction/PreprocessorFlags.cpp input/handler/instruction/PreprocessorFlags.h
        gui/SettingsWindow.cpp gui/SettingsWindow.h gui/MainWindow.cpp
        gui/MainWindow.h qtres.qrc gui/picker/ImagePickerDrawer.h ${hotkey_SRCS})

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Concurrent REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Concurrent)

target_include_directories(${PROJECT_NAME} PUBLIC util/lib/QHotkey/QHotkey)

configure_file(util/config/project_config.h.in ${PROJECT_BINARY_DIR}/config/project_config.h)
configure_file(util/config/version.rc.in ${PROJECT_BINARY_DIR}/config/version.rc)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/config)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::QWindowsIntegrationPlugin)
    # workaround for QWindowsIntegrationPlugin dependencies not linking
    target_link_libraries(${PROJECT_NAME}
            -lgdi32 -lcomdlg32 -loleaut32 -limm32 -ldwmapi -lwinspool -lopengl32 -lshlwapi -lwtsapi32
            -lmpr -lnetapi32 -luserenv -lversion -lws2_32 -lkernel32 -luser32 -lshell32 -luuid -lole32
            -ladvapi32 -lwinmm -lssl -lcrypto -lcrypt32 -liphlpapi
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5EventDispatcherSupport.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5FontDatabaseSupport.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5ThemeSupport.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5AccessibilitySupport.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5WindowsUIAutomationSupport.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5Core.a
            /mxe/usr/i686-w64-mingw32.static/qt5/lib/libQt5Gui.a
            /mxe/usr/i686-w64-mingw32.static/lib/libdwrite.a
            /mxe/usr/i686-w64-mingw32.static/lib/libd2d1.a
            /mxe/usr/i686-w64-mingw32.static/lib/libpng16.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libharfbuzz.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libcairo.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libgobject-2.0.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libfontconfig.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libfreetype.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libm.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libusp10.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libmsimg32.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libgdi32.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libpixman-1.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libffi.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libexpat.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libbz2.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libpng16.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libz.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libharfbuzz_too.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libfreetype_too.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libglib-2.0.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libws2_32.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libole32.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libwinmm.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libshlwapi.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libpcre.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libintl.a
            /mxe/usr/i686-w64-mingw32.static/lib/../lib/libiconv.a
            /mxe/usr/i686-w64-mingw32.static/lib/libopengl32.a
            /mxe/usr/i686-w64-mingw32.static/lib/libz.a
            /mxe/usr/i686-w64-mingw32.static/lib/libpcre2-16.a
            /mxe/usr/i686-w64-mingw32.static/lib/libzstd.a
            /mxe/usr/i686-w64-mingw32.static/lib/libopengl32.a
            /mxe/usr/i686-w64-mingw32.static/lib/libssl.a
            /mxe/usr/i686-w64-mingw32.static/lib/libcrypto.a
            /mxe/usr/i686-w64-mingw32.static/lib/libcrypt32.a
            /mxe/usr/i686-w64-mingw32.static/lib/libiphlpapi.a
            )
endif ()

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

# export compile commands for YCM
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt LICENSE.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/eyes.ico config/eyes.ico COPYONLY)

install(TARGETS ${PROJECT_NAME} EXPORT emojigun DESTINATION bin)

set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_NAME emojigun
        COMPILE_FLAGS "${WARNING_FLAGS}"
)
