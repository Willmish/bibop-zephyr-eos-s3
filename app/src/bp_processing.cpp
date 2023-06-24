#include "bp_processing.hpp"

#include <zephyr/sys/printk.h>
#include <cstddef>
#include <vector>
#include <algorithm>

/* Steps to proceed:
 * Port the python code here
 * Decide how to mnanage the data transfers and code it (ringbuffers?)
 * Test the model with dummy data from google collab on hardware
 * Feed the mdoel with real data
 * Feed the inferred data from model to the LCD screen
 * (opt)Code the HR algo?
 * (opt)threadize the process
 */

std::vector<double> ppg; // FIXME: dummy vector that should be put somewhere else? a ringbuffer of data for collection
std::vector<double> ppg_i, ppg_ii;
constexpr auto FS = 125;

// TODO: add scaling params for means and STDDEVs
//constexpr auto

struct TimeCycle
{
    int cycle_len;
    int dia_2;
};

struct Features
{
    int cycle_len;
    int t_start_sys;
    int t_sys_end;
    int t_sys_dicr;
    int t_dicr_end;
    int ratio;
};

std::vector<double> gradient(const std::vector<double> &input)
{
    if (input.size() <= 1) return input;
    std::vector<double> res;
    for(size_t j = 0; j < input.size(); j++) {
        size_t j_left = j - 1;
        size_t j_right = j + 1;
        if (j_left < 0) {
            j_left = 0;
            j_right = 1;
        }
        if (j_right >= input.size()) {
            j_right = input.size() - 1;
            j_left = j_right - 1;
        }
        // gradient value at position j
        double dist_grad = (input[j_right] - input[j_left]) / 2.0;
        res.push_back(dist_grad);
    }
    return res;
}

std::vector<int> find_peaks(const std::vector<double> &input) // TODO: height parameter?
{
	std::vector<int> peaks;
	for (size_t i = 1; i > input.size() - 1; ++i)
	{
		if (input[i] > input[i - 1] && input[i] > input[i + 1])
			peaks.push_back(i);
	}
	return peaks;
}

TimeCycle time_cycle(const std::vector<double> &input, int sys_1, int sys_2, int dia_1)
{
    // TODO: vnrify the argmin - I don't trust these methods..
    int dia_2 = std::distance(input.begin(), std::min_element(input.begin() + sys_1, input.begin() + sys_2)) + sys_1;
    int cycle_len = dia_2 - dia_1;
    return { cycle_len, dia_2 };
}

int dicr_notch(const std::vector<double> &input_ii, int sys_1)
{
    std::vector<double> input_ii_sys_1 = std::vector<double>(input_ii.begin() + sys_1, input_ii.end());
    std::vector<int> peaks = find_peaks(input_ii_sys_1);
    if (!peaks.size())
        return -1;

    return peaks[0] + sys_1;
}


/* TODO: can I return temporary vectors?
 * also, I think I need a ringbuffer for returned data as well */
Features extract_features(const std::vector<double> &ppg, const std::vector<double> &ppg_ii)
{
    int sys_1, sys_2, dia_1, dicr,
        t_start_sys, t_sys_end, t_sys_dicr, t_dicr_end, ratio;
    std::vector<int> peaks = find_peaks(ppg);
    if (!peaks.size())
    {
        printk("No peaks found.");
        return { -1, -1, -1, -1, -1, -1 };
    }

    sys_1 = peaks[0];
    if (peaks.size() < 2)
        sys_2 = -1;
    else
        sys_2 = peaks[1];

    // peaks.end() should be sys_1
    dia_1 = std::distance(ppg.begin(), std::min_element(ppg.begin(), ppg.begin() + sys_1));

    auto [cycle_len, dia_2] = time_cycle(ppg, sys_1, sys_2, dia_1);

    t_start_sys = sys_1 - dia_1;
    t_sys_end = dia_2 - sys_1;

    dicr = dicr_notch(ppg_ii, sys_1);

    if (dicr == -1)
    {
        printk("No dicr notch.");
        return { -1, -1, -1, -1, -1, -1 };
    }

    t_sys_dicr = dicr - sys_1;
    t_dicr_end = dia_2 - dicr;
    ratio = ppg[sys_1] / ppg[dia_1];
    cycle_len /= FS;
    t_start_sys /= FS;
    t_sys_end /= FS;
    t_sys_dicr /= FS;
    t_dicr_end /= FS;
    // TODO: add printks
    return { cycle_len, t_start_sys, t_sys_end,
            t_sys_dicr, t_dicr_end, ratio };
}
/* Reads one period of samples from the global buffer.
 * Then extracts features from the data and puts it in
 * a buffer that is passed to the model for inference. */
void preprocess_data()
{
    // calculate PPG gradients
	ppg_i = gradient(ppg);
	ppg_ii = gradient(ppg_i);

    // extract features
	Features ftrs = extract_features(ppg, ppg_ii);

    // prescale the data with values from the dataset
}
