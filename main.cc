#include <ns3/network-module.h>

int run_tomo_test(int argc, char **argv);
int run_neut_test(int argc, char **argv);
int run_same_topo_neut_test(int argc, char **argv);

/* -------- These contains experiments after May 2022 -------- */
int run_back_to_back_neut_exp(int argc, char **argv);

int main(int argc, char **argv) {
    ns3::PacketMetadata::Enable(); // This is added for printing

/* -------- These are experiments between Sep 2021 and May 2022 -------- */
//    run_tomo_test(argc, argv);
//    run_neut_test(argc, argv);
//    run_same_topo_neut_test(argc, argv);

/* -------- These are experiments after May 2022 -------- */
    run_back_to_back_neut_exp(argc, argv);
}

