cmake_minimum_required(VERSION 3.22)

project(pmd_nds_dungeon_gen
    VERSION 1.1 
    LANGUAGES CXX)

add_subdirectory(pmd_nds_dungeon_gen)

include(FetchContent)

#We need the JSON and the header file from here
FetchContent_Declare(
	pmd2_dungeon_data_scraper
	GIT_REPOSITORY https://github.com/VMSaarelainen/pmd-sky-dungeon-data-extractor
	GIT_TAG 402940f63326c7a8d369d07e834179a4428fefa6)
FetchContent_MakeAvailable(pmd2_dungeon_data_scraper)
target_include_directories(${PROJECT_NAME} PRIVATE ${pmd2_dungeon_data_scraper_SOURCE_DIR})
file(COPY ${pmd2_dungeon_data_scraper_SOURCE_DIR}/PMD_Sky_dungeon_data.json DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

#CLI arg parsing lib
FetchContent_Declare(
	tclap
	URL https://sourceforge.net/projects/tclap/files/tclap-1.2.5.tar.gz/download    #The current 1.4 release doesn't seem to build on Linux, using 1.2.5 instead
	URL_HASH MD5=346a92acf9b364dfbff0a6df03c8a59e
)
FetchContent_GetProperties(tclap)
if(NOT tclap_POPULATED)
	FetchContent_Populate(tclap)
endif()
add_library(tclap INTERFACE IMPORTED)
target_include_directories(${PROJECT_NAME} PRIVATE ${tclap_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE tclap)

#JSON lib
FetchContent_Declare(json
	GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent     #Lightweight automatic tracking repo for https://github.com/nlohmann/json. If this stops working, replace with that URL.
	GIT_PROGRESS TRUE  GIT_SHALLOW TRUE
	GIT_TAG v3.11.2)
FetchContent_MakeAvailable(json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)



#Extra compiler config
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        ${PROJECT_NAME}_DEBUG=1
    >
)
if (UNIX)
	target_compile_options(${PROJECT_NAME} PRIVATE -std=c++20 -Wall -Werror -Wextra -Wpedantic)
elseif (WIN32)
	target_compile_options(${PROJECT_NAME} PRIVATE /std:c++20 /W4)
endif()