import os
import subprocess
import numpy as np
import time

__ns3_path = os.popen('locate "ns-3.33" | grep /ns-3.33$').read().splitlines()[0]
__project_path = __ns3_path + "/scratch/wehe_p_tomography"

__duration = "20"
# __results_folder_name = "/17_12_2020/synthesized_traffic/link_540Mbps/policing/rate_4Mbps"
# __results_folder_name = "/17_12_2020/synthesized_traffic/ppb_1540bps_2s_h.8_.05Mbps_bp_.8bps_.03s_150Mbps"
__results_folder_name = "/test_tcp"
__results_path = __ns3_path + "/scratch/wehe_p_tomography/results" + __results_folder_name


def run_ns3_program(seed=3):
    os.system("mkdir -p " + __results_path)
    os.system(__ns3_path + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=1" +
              " --duration=" + __duration +
              " --resultsFolderName=" + __results_folder_name +
              "\"")


def get_ns3_program_output(seed=3):
    os.system("mkdir -p " + __results_path)
    return subprocess.getoutput(__ns3_path + "/waf --run \"wehe_plus_tomography\"  --command-template=\"%s" +
                                " --RngSeed=" + str(seed) +
                                " --duration=" + __duration +
                                " --resultsFolderName=" + __results_folder_name +
                                "\"")


def run_ns3_program_mult(nbRepeat=1, seed=3):
    for i in np.arange(0, nbRepeat):
        os.system("mkdir -p " + __results_path + "/" + str(i))
        os.system(__ns3_path + "/waf --run \"wehe_plus_tomography\"  --command-template=\"%s" +
                  " --RngSeed=" + str(seed) +
                  " --RngRun=" + str(i) +
                  " --duration=" + __duration +
                  " --resultsFolderName=" + __results_folder_name + "/" + str(i) +
                  "\"")
        time.sleep(10)


def get_project_path():
    return __project_path


def get_results_path():
    return __results_path


def get_duration():
    return __duration


def get_ns3_path():
    return __ns3_path

