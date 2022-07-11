cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

##################### Part here is to help compile within the ns3 project  #####################
set(target_name wehe_p_tomography)
set(output_directory ${CMAKE_OUTPUT_DIRECTORY}/scratch/wehe_p_tomography/)
set(target_prefix scratch_wehe_p_tomography_)

project(${target_prefix}${target_name})
################################################################################################

find_package(Boost QUIET REQUIRED COMPONENTS program_options) # use boost for hash functions
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_definitions(-DNS3_LOG_ENABLE)
include_directories(/home/shmeis/Desktop/experiments/ns-allinone-3.36.1/ns-3.36.1/build/include)
link_directories(/home/shmeis/Desktop/experiments/ns-allinone-3.36.1/ns-3.36.1/build/include)
include_directories(/home/shmeis/Desktop/experiments/ns-allinone-3.36.1/ns-3.36.1/build)
link_directories(/home/shmeis/Desktop/experiments/ns-allinone-3.36.1/ns-3.36.1/build)

file(GLOB SOURCE_FILES main.cc)
file(GLOB HELPER_CLASSES helper_classes/*)
file(GLOB SCENARIOS_FILES
        localization_experiments_scenarios/*)
file(GLOB MONITOR_FILES monitors_module/*)
file(GLOB TRAFFIC_DIFFERENTATION_FILES traffic_differentiation_module/*)
file(GLOB TRAFFIC_GENERATOR_FILES
        traffic_generator_module/background_replay/*
        traffic_generator_module/infinite_tcp/*
        traffic_generator_module/measurement_replay/*
        traffic_generator_module/packet_probes/*
        traffic_generator_module/wehe_cs/*
        traffic_generator_module/wehe_cs/wehe_udp_cs/*
        traffic_generator_module/wehe_cs/wehe_tcp_cs/*)

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
        libcore
        libwave
        libnetanim
        libwifi
        libnetwork
        libinternet
        libapplications
        libpoint-to-point
        Boost::program_options)

##################### Part here is to help compile within the ns3 project  #####################
set(ns3-exec-outputname ns${NS3_VER}-${target_name}${build_profile_suffix})
set(ns3-execs "${output_directory}${ns3-exec-outputname};${ns3-execs}" CACHE INTERNAL "list of c++ executables")

set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_directory}
                   RUNTIME_OUTPUT_NAME ${ns3-exec-outputname}
)
################################################################################################