cmake_minimum_required(VERSION 3.22)

# Set SDK and JUCE options before project() so compiler and subdirectories see them.
# Stops juceaide build and fixes 'algorithm' not found when using JUCE source + Command Line Tools.
if(DEFINED JUCE_DIR AND JUCE_DIR AND NOT EXISTS "${JUCE_DIR}/JUCEConfig.cmake")
  set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Do not build JUCE extras (juceaide, etc.)" FORCE)
  if(APPLE AND (NOT CMAKE_OSX_SYSROOT OR CMAKE_OSX_SYSROOT STREQUAL ""))
    if(EXISTS "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk")
      set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk" CACHE STRING "macOS SDK" FORCE)
    endif()
  endif()
endif()

# C and CXX: JUCE subprojects may use C, so both are required (fixes CMAKE_C_COMPILE_OBJECT missing).
project(MatildaPiano VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use install prefix (JUCEConfig.cmake) or source directory (add_subdirectory)
set(JUCE_DIR "${JUCE_DIR}" CACHE PATH "JUCE install prefix or source directory")
if(NOT JUCE_DIR)
  message(FATAL_ERROR "JUCE_DIR not set. Set -DJUCE_DIR=/path/to/JUCE (install prefix or source folder).")
endif()
if(EXISTS "${JUCE_DIR}/JUCEConfig.cmake")
  find_package(JUCE CONFIG REQUIRED)
else()
  add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE-build")
endif()

# Create the plugin (AU for DAWs; Standalone for fast UI testing without a host)
juce_add_plugin(MatildaPiano
    COMPANY_NAME "MatildaAudio"
    PLUGIN_MANUFACTURER_CODE MatA
    PLUGIN_CODE MatP
    FORMATS AU Standalone
    PRODUCT_NAME "Matilda Piano"
    DESCRIPTION "A beautiful piano VST plugin"
    VERSION 1.0.0
    PLUGIN_MANUFACTURER "Matilda Audio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Generate JuceHeader.h into the build tree and add its include path (required for #include <JuceHeader.h>)
juce_generate_juce_header(MatildaPiano)

# Add source files
target_sources(MatildaPiano PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters.cpp
    Source/MatildaSamplerVoice.cpp
    Source/MatildaSamplerSound.cpp
    Source/TapeModule.cpp
    Source/DelayModule.cpp
    Source/ReverbModule.cpp
    Source/XYPadComponent.cpp
    Source/ChickenHeadKnob.cpp
    Source/MatildaKeyboardComponent.cpp
)

# Add header files (for IDE support)
target_sources(MatildaPiano PRIVATE
    Source/PluginProcessor.h
    Source/PluginEditor.h
    Source/Parameters.h
    Source/MatildaSamplerVoice.h
    Source/MatildaSamplerSound.h
    Source/TapeModule.h
    Source/DelayModule.h
    Source/ReverbModule.h
    Source/XYPadComponent.h
    Source/ChickenHeadKnob.h
    Source/MatildaKeyboardComponent.h
)

# Link JUCE modules
target_link_libraries(MatildaPiano
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
)

# BinaryData: embed assets when present. Use a separate target to avoid name clash with juce_add_plugin(MatildaPiano).
file(GLOB ASSET_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Images/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Images/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/Jacquard_24/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/Kode_Mono/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/Kode_Mono/static/*.ttf"
)

if(ASSET_FILES)
    juce_add_binary_data(MatildaPiano_Assets ASSETS
        SOURCES ${ASSET_FILES}
        HEADER_NAME BinaryData.h
    )
    target_link_libraries(MatildaPiano PRIVATE MatildaPiano_Assets)
    # BinaryData.h is generated in juce_binarydata_MatildaPiano_Assets/JuceLibraryCode; ensure MatildaPiano can find it.
    target_include_directories(MatildaPiano PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_MatildaPiano_Assets/JuceLibraryCode)
    target_compile_definitions(MatildaPiano PRIVATE MATILDA_HAS_BINARY_DATA=1)
else()
    target_compile_definitions(MatildaPiano PRIVATE MATILDA_HAS_BINARY_DATA=0)
endif()

# macOS specific settings
if(APPLE)
    set_target_properties(MatildaPiano PROPERTIES
        OSX_ARCHITECTURES "arm64;x86_64"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.matildaaudio.matildapiano"
        MACOSX_BUNDLE_BUNDLE_NAME "Matilda Piano"
    )
    
    # Set minimum macOS version
    set_target_properties(MatildaPiano PROPERTIES
        OSX_DEPLOYMENT_TARGET "12.0"
    )
    
    # Copy keySamples into Standalone app bundle (plugin also looks for keySamples next to the .app)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/keySamples")
        add_custom_command(TARGET MatildaPiano_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/keySamples"
                "$<TARGET_FILE_DIR:MatildaPiano_Standalone>/../Resources/keySamples"
            COMMENT "Copying keySamples into Standalone app bundle"
        )
    endif()
    # Copy Assets (Images + Fonts) so left-panel image and fonts load from Resources when BinaryData names differ
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
        add_custom_command(TARGET MatildaPiano_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
                "$<TARGET_FILE_DIR:MatildaPiano_Standalone>/../Resources/Assets"
            COMMENT "Copying Assets into Standalone app bundle (Images, Fonts)"
        )
    endif()
endif()

# Unit tests (parameter layout, etc.)
juce_add_console_app(MatildaPianoTests
    PRODUCT_NAME "MatildaPiano Tests"
)
juce_generate_juce_header(MatildaPianoTests)
target_sources(MatildaPianoTests PRIVATE
    Tests/MatildaPianoTests.cpp
    Source/Parameters.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/MatildaSamplerVoice.cpp
    Source/MatildaSamplerSound.cpp
    Source/TapeModule.cpp
    Source/DelayModule.cpp
    Source/ReverbModule.cpp
    Source/XYPadComponent.cpp
    Source/ChickenHeadKnob.cpp
    Source/MatildaKeyboardComponent.cpp
)
target_include_directories(MatildaPianoTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(MatildaPianoTests PRIVATE
    MATILDA_HAS_BINARY_DATA=0
    JucePlugin_Name="Matilda Piano"
    JucePlugin_IsMidiEffect=0
    JucePlugin_IsSynth=1
    JucePlugin_WantsMidiInput=1
    JucePlugin_ProducesMidiOutput=0
)
target_link_libraries(MatildaPianoTests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_utils
    juce::juce_data_structures
)
