set(SOURCES
    Generator.cpp
    Beetle.cpp
    Cross.cpp
    Crossroads.cpp
    Line.cpp
    OneRoomMonsterHouse.cpp
    OuterRing.cpp
    OuterRooms.cpp
    Standard.cpp
    TwoRoomsWithMonsterHouse.cpp
    #Fixed.cpp
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})