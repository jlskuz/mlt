add_library(mltsdl2 MODULE
  common.c
  consumer_sdl2_audio.c
  consumer_sdl2.c
  factory.c
)

file(GLOB YML "*.yml")
add_custom_target(Other_sdl2_Files SOURCES
  ${YML}
)

target_compile_options(mltsdl2 PRIVATE ${MLT_COMPILE_OPTIONS})

target_link_libraries(mltsdl2 PRIVATE mlt Threads::Threads PkgConfig::sdl2)

if(MSVC)
  set(PTHREAD_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib/pthreadVC3.lib")
  target_link_libraries(mltsdl2 PRIVATE ${PTHREAD_LIB_PATH})
  set_target_properties(mltsdl2 PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
else()
  target_link_libraries(mltsdl2 PRIVATE m)
endif()

set_target_properties(mltsdl2 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

install(TARGETS mltsdl2 LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

install(FILES consumer_sdl2_audio.yml consumer_sdl2.yml DESTINATION ${MLT_INSTALL_DATA_DIR}/sdl2)
