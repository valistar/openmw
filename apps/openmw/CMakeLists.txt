
# config file
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/config.hpp")

# local files
set(GAME
    main.cpp
    engine.cpp
)
set(GAME_HEADER
    engine.hpp
    config.hpp
)
source_group(game FILES ${GAME} ${GAME_HEADER})

add_openmw_dir (mwrender
    renderingmanager debugging sky player animation npcanimation creatureanimation actors objects
    renderinginterface localmap occlusionquery terrain terrainmaterial water shadows
    compositors characterpreview externalrendering globalmap
    )

add_openmw_dir (mwinput
    inputmanagerimp
    )

add_openmw_dir (mwgui
    text_input widgets race class birth review windowmanagerimp console dialogue
    dialogue_history window_base stats_window messagebox journalwindow charactercreation
    map_window window_pinnable_base cursorreplace tooltips scrollwindow bookwindow list
    formatting inventorywindow container hud countdialog tradewindow settingswindow
    confirmationdialog alchemywindow referenceinterface spellwindow mainmenu quickkeysmenu
    itemselection spellbuyingwindow loadingscreen levelupdialog waitdialog spellcreationdialog
    enchantingdialog
    )

add_openmw_dir (mwdialogue
    dialoguemanagerimp journalimp journalentry quest topic
    )

add_openmw_dir (mwscript
    locals scriptmanagerimp compilercontext interpretercontext cellextensions miscextensions
    guiextensions soundextensions skyextensions statsextensions containerextensions
    aiextensions controlextensions extensions globalscripts ref dialogueextensions
    animationextensions transformationextensions consoleextensions userextensions
    )

add_openmw_dir (mwsound
    soundmanagerimp openal_output audiere_decoder mpgsnd_decoder ffmpeg_decoder
    )

add_openmw_dir (mwworld
    refdata worldimp physicssystem scene globals class action nullaction actionteleport
    containerstore actiontalk actiontake manualref player cellfunctors
    cells localscripts customdata weather inventorystore ptr actionopen actionread
    actionequip timestamp actionalchemy cellstore actionapply actioneat
    )

add_openmw_dir (mwclass
    classes activator creature npc weapon armor potion apparatus book clothing container door
    ingredient creaturelevlist itemlevlist light lockpick misc probe repair static
    )

add_openmw_dir (mwmechanics
    mechanicsmanagerimp stat creaturestats magiceffects movement actors drawstate spells
    activespells npcstats aipackage aisequence
    )

add_openmw_dir (mwbase
    environment world scriptmanager dialoguemanager journal soundmanager mechanicsmanager
    inputmanager windowmanager
    )

# Main executable
IF(OGRE_STATIC)
IF(WIN32)
ADD_DEFINITIONS(-DENABLE_PLUGIN_CgProgramManager -DENABLE_PLUGIN_OctreeSceneManager -DENABLE_PLUGIN_ParticleFX -DENABLE_PLUGIN_-DENABLE_PLUGIN_Direct3D9 -DENABLE_PLUGIN_GL)
set(OGRE_STATIC_PLUGINS ${OGRE_Plugin_CgProgramManager_LIBRARIES} ${OGRE_Plugin_OctreeSceneManager_LIBRARIES} ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_RenderSystem_Direct3D9_LIBRARIES} ${OGRE_RenderSystem_GL_LIBRARIES})
ELSE(WIN32)
ADD_DEFINITIONS(-DENABLE_PLUGIN_CgProgramManager -DENABLE_PLUGIN_OctreeSceneManager -DENABLE_PLUGIN_ParticleFX -DENABLE_PLUGIN_GL)
set(OGRE_STATIC_PLUGINS ${OGRE_Plugin_CgProgramManager_LIBRARIES} ${Cg_LIBRARIES} ${OGRE_Plugin_OctreeSceneManager_LIBRARIES} ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_RenderSystem_GL_LIBRARIES})
ENDIF(WIN32)
ENDIF(OGRE_STATIC)
add_executable(openmw
    ${OPENMW_LIBS} ${OPENMW_LIBS_HEADER}
    ${OPENMW_FILES}
    ${GAME} ${GAME_HEADER}
    ${APPLE_BUNDLE_RESOURCES}
)

# Sound stuff - here so CMake doesn't stupidly recompile EVERYTHING
# when we change the backend.
include_directories(${SOUND_INPUT_INCLUDES} ${BULLET_INCLUDE_DIRS})
add_definitions(${SOUND_DEFINE})

target_link_libraries(openmw
    ${OGRE_LIBRARIES}
    ${OGRE_Terrain_LIBRARY}
    ${OGRE_STATIC_PLUGINS}
    ${OIS_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${SOUND_INPUT_LIBRARY}
    ${BULLET_LIBRARIES}
    ${MYGUI_LIBRARIES}
    ${MYGUI_PLATFORM_LIBRARIES}
    "shiny"
    "shiny.OgrePlatform"
    "oics"
    components
)

# Fix for not visible pthreads functions for linker with glibc 2.15
if (UNIX AND NOT APPLE)
target_link_libraries(openmw ${CMAKE_THREAD_LIBS_INIT})
endif()

if(APPLE)
    find_library(CARBON_FRAMEWORK Carbon)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    target_link_libraries(openmw ${CARBON_FRAMEWORK} ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK})
endif(APPLE)

if(DPKG_PROGRAM)
    INSTALL(TARGETS openmw RUNTIME DESTINATION games COMPONENT openmw)
endif(DPKG_PROGRAM)

if (BUILD_WITH_CODE_COVERAGE)
  add_definitions (--coverage)
  target_link_libraries(openmw gcov)
endif()
