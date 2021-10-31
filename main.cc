#include <ns3/network-module.h>

int run_wehe_1path_w_ppb_noise(int argc, char **argv);
int run_wehe_2path_w_ppb_noise(int argc, char **argv);
int run_wehe_Npath_w_ppb_noise(int argc, char **argv);
int run_wehe_Nby1_w_background(int argc, char **argv);
int run_NWeheCS_w_CAIDA(int argc, char **argv);

int run_neut_test_wehe(int argc, char **argv);

int run_wehe_tomo_b2b_w_background(int argc, char **argv);

int run_wehe_with_back_on_1link(int argc, char **argv);

int run_1path_ppb_udp(int argc, char **argv);
int run_queue_testing(int argc, char **argv);
int run_1path_tcp(int argc, char **argv);
int run_weheCS_test(int argc, char **argv);
int run_multipleReplayClient_test(int argc, char **argv);
int run_TcpCwnd_test(int argc, char **argv);

int run_neut_test_poisson(int argc, char **argv);

int run_tomo_test(int argc, char **argv);
int run_neut_test(int argc, char **argv);



int main(int argc, char **argv) {
//    ns3::PacketMetadata::EnableChecking(); // This is added for printing
    ns3::PacketMetadata::Enable(); // This is added for printing

//    return run_wehe_1path_w_ppb_noise(argc, argv);
//    return run_wehe_2path_w_ppb_noise(argc, argv);
//    return run_wehe_tomo_b2b_w_background(argc, argv);
//    return run_wehe_Npath_w_ppb_noise(argc, argv);
//    return run_wehe_Nby1_w_background(argc, argv);
//    return run_wehe_with_back_on_1link(argc, argv);
//    return run_NWeheCS_w_CAIDA(argc, argv);

//    return run_neut_test_wehe(argc, argv);

//    return run_neut_test_poisson(argc, argv);
//    return run_1path_ppb_udp(argc, argv);
//    return run_queue_testing(argc, argv);
//    return run_1path_tcp(argc, argv);
//    return run_weheCS_test(argc, argv);
//    return run_multipleReplayClient_test(argc, argv);
//    return run_TcpCwnd_test(argc, argv);

    // This is the new set of experiments after Sep 2021
//    run_tomo_test(argc, argv);
    run_neut_test(argc, argv);
}

