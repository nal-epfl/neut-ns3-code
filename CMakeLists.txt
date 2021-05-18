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
find_package(Matlab)
if(NOT Matlab_FOUND)
    message(FATAL_ERROR "Matlab Not found")
endif()


add_definitions(-DNS3_LOG_ENABLE)
include_directories(~/Desktop/experiments2/ns-allinone-3.33/ns-3.33/build)
link_directories(~/Desktop/experiments2/ns-allinone-3.33/ns-3.33/build)

set(SOURCE_FILES main.cc)
set(PROJECT_ENV_FILES project_run_env/RunConfig.py loss_analysis/loss_analysis_funcs.py)
file(GLOB SCENARIOS_FILES ploss_infer_scenarios/* testing_scenarios/* neut_infer_scenarios/*)
file(GLOB MONITOR_FILES monitors_module/*)
file(GLOB TRAFFIC_GENERATOR_FILES traffic_generator_module/poisson_udp/* traffic_generator_module/trace_replay/* traffic_generator_module/ppb/* traffic_generator_module/utility/* traffic_generator_module/poisson_bursts/* traffic_generator_module/wehe_cs/*)
file(GLOB TRAFFIC_DIFFERENTATION_FILES traffic_differentiation_module/*)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${PROJECT_ENV_FILES} ${SCENARIOS_FILES} ${MONITOR_FILES} ${TRAFFIC_GENERATOR_FILES} ${TRAFFIC_DIFFERENTATION_FILES})

set(NS3_EMU ON)
target_link_libraries(${PROJECT_NAME}
        libns3.33-core-debug
        ns3.33-wave-debug
        ns3.33-wifi-debug
        ns3.33-netanim-debug
        ns3.33-network-debug
        ns3.33-internet-debug
        ns3.33-mobility-debug
        ns3.33-visualizer-debug
        ns3.33-applications-debug
        ns3.33-applications-debug
        ns3.33-point-to-point-debug
        Boost::program_options)
