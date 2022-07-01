#include <ns3/network-module.h>

int run_back_to_back_neut_exp(int argc, char **argv);
int run_bottleneck_detection_exp(int argc, char **argv);

int main(int argc, char **argv) {
    ns3::PacketMetadata::Enable(); // This is added for printing

//    run_back_to_back_neut_exp(argc, argv);
    run_bottleneck_detection_exp(argc, argv);
}

