cmake_minimum_required(VERSION 3.16)
project(wehe_p_tomography)

set(CMAKE_CXX_STANDARD 17)

# Add boost library for the hash functions
find_package(Boost QUIET REQUIRED COMPONENTS program_options)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
##########################################

add_definitions(-DNS3_LOG_ENABLE)
include_directories(~/Desktop/experiments2/ns-allinone-3.35/ns-3.35/build)
link_directories(~/Desktop/experiments2/ns-allinone-3.35/ns-3.35/build)


set(SOURCE_FILES main.cc)
set(PROJECT_ENV_FILES project_run_env/RunConfig.py)
file(GLOB HELPER_CLASSES helper_classes/*)
file(GLOB SCENARIOS_FILES
        experiments_scenarios/*
        testing_scenarios/*
        old_code/very_old_experiments_with_ppb_background/* )
file(GLOB MONITOR_FILES monitors_module/*)
file(GLOB TRAFFIC_DIFFERENTATION_FILES traffic_differentiation_module/*)
file(GLOB TRAFFIC_GENERATOR_FILES
        traffic_generator_module/infinite_tcp/*
        traffic_generator_module/measurement_replay/*
        traffic_generator_module/poisson/*
        traffic_generator_module/poisson_bursts/*
        traffic_generator_module/ppb/*
        traffic_generator_module/trace_replay/*
        traffic_generator_module/utility/*
        traffic_generator_module/wehe_cs/*)

add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${PROJECT_ENV_FILES}
        ${HELPER_CLASSES}
        ${SCENARIOS_FILES}
        ${MONITOR_FILES}
        ${TRAFFIC_DIFFERENTATION_FILES}
        ${TRAFFIC_GENERATOR_FILES})


set(NS3_EMU ON)
target_link_libraries(${PROJECT_NAME}
        libns3.35-core-debug
        ns3.35-wave-debug
        ns3.35-wifi-debug
        ns3.35-netanim-debug
        ns3.35-network-debug
        ns3.35-internet-debug
        ns3.35-mobility-debug
        ns3.35-visualizer-debug
        ns3.35-applications-debug
        ns3.35-applications-debug
        ns3.35-point-to-point-debug
        Boost::program_options)
