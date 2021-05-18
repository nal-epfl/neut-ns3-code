import matlab.engine
import pandas as pd
import numpy as np
import math
import ast
import scipy.stats as stats
import statsmodels.tsa.stattools as stattools
import os
import re


def read_df(filename):
    return pd.read_csv(filename).sort_values(by=['SentTime'], ascending=True).reset_index(drop=True)


# returns loss events with the times as ndarray
def extract_loss_events(df):
    return 1-df['IsReceived'].values, df['SentTime'].values * 1e-9


def compute_loss_ratio(loss_events):
    return sum(loss_events) / len(loss_events) if len(loss_events) != 0 else 0


# returns the confidence interval as a tuple
def compute_iid_ci(loss_events, confidence_level=0.95):
    return stats.t.interval(confidence_level,
                            len(loss_events)-1,
                            loc=compute_loss_ratio(loss_events),
                            scale=stats.sem(loss_events))


def compute_sail_ci(loss_events_vals, loss_events_ts, confidence_level=0.95):
    eng = matlab.engine.start_matlab()
    eng.addpath('/home/nal/Desktop/experiments/SAIL-v2/LIBRARY')
    eng.addpath('/home/nal/Desktop/experiments/SAIL-v2')

    m_loss_events_val = matlab.int32([(1-loss_events_vals).tolist()])
    m_loss_events_ts = matlab.double([loss_events_ts.tolist()])
    m_ks, m_thetas, m_var = eng.SAIL(m_loss_events_val, m_loss_events_ts, nargout=3)
    eng.quit()

    z_value = stats.norm.ppf((1+confidence_level)/2)
    estimated_loss = compute_loss_ratio(loss_events_vals)

    ci_lower = estimated_loss - z_value * math.sqrt(m_var)
    ci_upper = estimated_loss + z_value * math.sqrt(m_var)

    print(m_var)

    return ci_lower, ci_upper


def compute_empirical_variance_ci(loss_events, confidence_level=0.95):
    sample_autocov = stattools.acovf(loss_events)
    n = len(loss_events)
    emp_VAR = sum([(1-abs(h)/n) * sample_autocov[abs(h)] for h in np.arange(-n+1, n)]) * (1 / n)

    z_value = stats.norm.ppf((1+confidence_level)/2)
    estimated_loss = compute_loss_ratio(loss_events)

    ci_lower = estimated_loss - z_value * math.sqrt(emp_VAR)
    ci_upper = estimated_loss + z_value * math.sqrt(emp_VAR)

    return ci_lower, ci_upper


# convert ratio to percentage
def to_pct(x):
    return x * 100


def to_numeric(s):
    try:
        return ast.literal_eval(s)
    except:
        return np.nan


def compute_ci_precision(ci):
    return (ci[1] - ci[0]) / 2


def generate_stat_files(data_path):
    for file in os.listdir(data_path):
        if bool(re.match(r"^[a-z|0-9]+_packets_[a-z]+.csv$", file)):
            loss_vals, loss_ts = extract_loss_events(read_df(data_path + '/' + file))

            nb_sent_pkts = len(loss_vals)
            nb_lost_pkts = sum(loss_vals)
            loss_ratio = to_pct(nb_lost_pkts / nb_sent_pkts) if nb_sent_pkts != 0 else 0

            out_fname = file.split('_packets_')[0] + '_stats_' + file.split('_packets_')[1].split('.csv')[0] + '.csv'
            with open(data_path + '/' + out_fname, 'w') as f:
                print('Nb of sent packets: ', nb_sent_pkts, file=f)
                print('Nb of lost packets: ', nb_lost_pkts, file=f)
                print('Loss Ratio: ', loss_ratio, file=f)


def build_link_packets(int_file1, in_file2, out_file):
    df1 = read_df(int_file1)
    df2 = read_df(in_file2)

    start_t = df1.iloc[0]['SentTime']
    end_t = df1.iloc[-1]['SentTime']
    pd.concat([df2[(df2['SentTime'] >= start_t) & (df2['SentTime'] <= end_t)], df1]).to_csv(out_file)


def compute_CI_models(df):
    loss_vals, loss_ts = extract_loss_events(df)
    loss_ratio = to_pct(compute_loss_ratio(loss_vals))
    CIs = {}
    CIs['iid'] = to_pct(compute_ci_precision(compute_iid_ci(loss_vals)))
    CIs['empvar'] = to_pct(compute_ci_precision(compute_empirical_variance_ci(loss_vals)))
    CIs['sail'] = to_pct(compute_ci_precision(compute_sail_ci(loss_vals, loss_ts)))
    return loss_ratio, CIs