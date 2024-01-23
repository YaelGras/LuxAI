cmake_minimum_required( VERSION 3.0 )
project( Unan C CXX )

#add_definitions(-DDEBUG_FRAMEWORK)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Filters
include( Filters.cmake )


FILE(GLOB SOURCE
	 "*.hpp"
     "*.cpp"
     "lux/*.hpp"
     "lux/*.cpp"
)

SET( AIBOT_BUILDFILES 
	
	CleanAll.bat
	CMakeLists.txt
	Filters.cmake
	GenSubmit.bat
	GenSubmit.py
	PlayLocalMatch.bat
	PlayLocalMatch.py
	rebuild_vis_sln_x64.bat
)

SET( AIBOT_HEADERS 

	#Framework
	lux/agent.hpp
	lux/kit.hpp
	#Blackboard	
	lux/Blackboard/Blackboard.hpp
	#BehaviourTree
	lux/BehaviourTree/BehaviourTree.hpp
	lux/BehaviourTree/FactoryLeaf.hpp
	# World
	lux/World/Unit.h
	lux/World/Tiles.h
	lux/World/Map.h
	lux/World/City.h
	lux/World/DayNightCycle.h
	lux/World/Interactions.h
	lux/World/Coordinates.h
	lux/World/InfluenceMap.h
	#Utility
	lux/Utility/Cluster.h
	lux/Utility/Clustering.h
	# kit
	lux/kit/annotate.hpp
	#Goap
	lux/Goap/Planner.h
	lux/Goap/Node.h
	lux/Goap/Action.h
	lux/Goap/FactoryAction.h
	lux/Goap/WorldState.h
	# Multithread
	lux/Thread/DataManager.h
	lux/Thread/ThreadManager.h
	# Commander-Squad-Unit
	lux/CSU/CSU.h
)

SET( AIBOT_SRC 

	#Framework
	lux/agent.cpp
	main.cpp
	#Blackboard
	lux/Blackboard/Blackboard.cpp
	#BehaviourTree
	lux/BehaviourTree/BehaviourTree.cpp
	lux/BehaviourTree/FactoryLeaf.cpp
	#Utility
	lux/Utility/Cluster.cpp
	lux/Utility/Clustering.cpp
	#Goap
	lux/Goap/Planner.cpp
	lux/Goap/Node.cpp
	lux/Goap/Action.cpp
	lux/Goap/FactoryAction.cpp
	lux/Goap/WorldState.cpp
	# Multithread
	lux/Thread/DataManager.cpp
	lux/Thread/ThreadManager.cpp
	# World
	lux/World/InfluenceMap.cpp
	lux/World/Tiles.cpp
	# Commander-Squad-Unit
	lux/CSU/CSU.cpp
)

SET(jobfiles "${AIBOT_HEADERS};${AIBOT_SRC};${AIBOT_BUILDFILES}")
file(WRITE jobfiles.txt "${jobfiles}")


add_executable(${PROJECT_NAME} ${AIBOT_SRC} ${AIBOT_HEADERS})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set(CMAKE_SUPPRESS_REGENERATION true)

target_include_directories (${PROJECT_NAME} PUBLIC ${AIBOT_HEADERS})

