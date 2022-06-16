#include <ns3/network-module.h>

int run_wehe_1path_w_ppb_noise(int argc, char **argv);
int run_wehe_2path_w_ppb_noise(int argc, char **argv);
int run_wehe_Npath_w_ppb_noise(int argc, char **argv);
int run_wehe_Nby1_w_background(int argc, char **argv);
int run_wehe_with_back_on_1link(int argc, char **argv);

int run_multipleReplayClient_test(int argc, char **argv);
int run_1path_ppb_udp(int argc, char **argv);
int run_1path_tcp(int argc, char **argv);
int run_TcpCwnd_test(int argc, char **argv);
int run_weheCS_test(int argc, char **argv);

int run_tomo_test(int argc, char **argv);
int run_neut_test(int argc, char **argv);
int run_same_topo_neut_test(int argc, char **argv);

/* -------- These contains experiments after May 2022 -------- */
int run_back_to_back_neut_exp(int argc, char **argv);

int main(int argc, char **argv) {
    ns3::PacketMetadata::Enable(); // This is added for printing

/* -------- These are experiments before Sep 2021 -------- */
//    return run_wehe_1path_w_ppb_noise(argc, argv);
//    return run_wehe_2path_w_ppb_noise(argc, argv);
//    return run_wehe_Npath_w_ppb_noise(argc, argv);
//    return run_wehe_Nby1_w_background(argc, argv);
//    return run_wehe_with_back_on_1link(argc, argv);
//
//    return run_1path_ppb_udp(argc, argv);
//    return run_1path_tcp(argc, argv);
//    return run_weheCS_test(argc, argv);
//    return run_multipleReplayClient_test(argc, argv);
//    return run_TcpCwnd_test(argc, argv);

/* -------- These are experiments between Sep 2021 and May 2022 -------- */
//    run_tomo_test(argc, argv);
//    run_neut_test(argc, argv);
//    run_same_topo_neut_test(argc, argv);

/* -------- These are experiments after May 2022 -------- */
    run_back_to_back_neut_exp(argc, argv);
}

