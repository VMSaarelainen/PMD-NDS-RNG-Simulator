add_executable(
    pmd_nds_dungeon_gen 
    main.cpp
)

add_subdirectory(Generators)
add_subdirectory(PRNG)
target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}
)
