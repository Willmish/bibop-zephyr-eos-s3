#include "bp_processing.hpp"

#include <zephyr/sys/printk.h>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <limits>

/* Steps to proceed:
 * Port the python code here
 * Decide how to mnanage the data transfers and code it (ringbuffers?)
 * Test the model with dummy data from google collab on hardware
 * Feed the mdoel with real data
 * Feed the inferred data from model to the LCD screen
 * (opt)Code the HR algo?
 * (opt)threadize the process
 */

std::vector<float> ppg; // FIXME: dummy vector that should be put somewhere else? a ringbuffer of data for collection
std::vector<float> ppg_i, ppg_ii;
constexpr auto FS = 125;

constexpr float DATA_MEANS[6] = { 0.438804, 0.125859, 0.312945,
                                  0.094671, 0.218274, 1.916834 };
constexpr float DATA_STDDEVS[6] = { 0.143371, 0.042210, 0.115670,
                                    0.021226, 0.106670, 0.406563 };

std::vector<float> DUMMY_PPG = { 1.75953079, 1.71847507, 1.68426197, 1.65786901, 1.63734115,
									1.61583578, 1.59335288, 1.57086999, 1.54936461, 1.52688172,
									1.50342131, 1.47898338, 1.45356794, 1.42815249, 1.40273705,
									1.3773216 , 1.35777126, 1.34115347, 1.32355816, 1.30596285,
									1.28836755, 1.27174976, 1.25806452, 1.24340176, 1.228739  ,
									1.21212121, 1.19354839, 1.17302053, 1.15835777, 1.15249267,
									1.15835777, 1.18963832, 1.26099707, 1.37829912, 1.54154448,
									1.73802542, 1.9540567 , 2.17106549, 2.37047898, 2.53958944,
									2.67253177, 2.76148583, 2.82404692, 2.87585533, 2.90615836,
									2.91788856, 2.91495601, 2.89931574, 2.87194526, 2.83284457,
									2.78103617, 2.71652004, 2.64125122, 2.56500489, 2.485826  ,
									2.39296188, 2.29227761, 2.18963832, 2.08993157, 1.99608993,
									1.91300098, 1.84066471, 1.77908113, 1.72922776, 1.68817204,
									1.65493646, 1.63147605, 1.61290323, 1.59335288, 1.57282502,
									1.55131965, 1.52883675, 1.50537634, 1.4799609 , 1.45552297,
									1.43010753, 1.40469208, 1.38025415, 1.36265885, 1.34799609,
									1.33040078, 1.31085044, 1.29325513, 1.27761486, 1.26197458,
									1.24731183, 1.23167155, 1.21505376, 1.19648094, 1.17399804,
									1.15151515, 1.13685239, 1.13587488, 1.15347019, 1.20527859,
									1.30107527, 1.44477028, 1.63049853, 1.84457478, 2.06744868,
									2.28054741, 2.46823069, 2.62170088, 2.72727273, 2.79569892,
									2.84652981, 2.87781036, 2.89051808, 2.88856305, 2.87487781,
									2.85043988, 2.81524927, 2.76637341, 2.70576735, 2.63343109,
									2.55131965, 2.47116325, 2.38905181, 2.29618768, 2.19843597,
									2.10166178, 2.00977517, 1.9257087 , 1.8514174 , 1.78690127 };

constexpr float DUMMY_RESULTS[6] = { 0.504, 0.128, 0.376, 0.128, 0.248, 2.5318066157760812 };

struct TimeCycle
{
    float cycle_len;
    float dia_2;
};

struct Features
{
    float cycle_len;
    float t_start_sys;
    float t_sys_end;
    float t_sys_dicr;
    float t_dicr_end;
    float ratio;
};

// this is the better algorithm
std::vector<float> gradient(const std::vector<float> &input)
{
    if (input.size() <= 1) return input;
    std::vector<float> res;
    res.reserve(FS);

    // Handle first element
    res.push_back(input[1] - input[0]);

    // Handle elements in the middle
    for(size_t j = 1; j < input.size() - 1; j++) {
        size_t j_left = j - 1;
        size_t j_right = j + 1;
        float dist_grad = (input[j_right] - input[j_left]) / 2.0;
        res.push_back(dist_grad);
    }

    // Handle last element
    res.push_back(input[input.size() - 1] - input[input.size() - 2]);

    return res;
}
//std::vector<float> gradient(const std::vector<float> &input)
//{
//    if (input.size() <= 1) return input;
//    std::vector<float> res;
//    res.reserve(FS);
//    for(size_t j = 0; j < input.size(); j++) {
//        size_t j_left = j - 1;
//        size_t j_right = j + 1;
//        if (j_left < 0) {
//            j_left = 0;
//            j_right = 1;
//        }
//        if (j_right >= input.size()) {
//            j_right = input.size() - 1;
//            j_left = j_right - 1;
//        }
//        // gradient value at position j
//        float dist_grad = (input[j_right] - input[j_left]) / 2.0;
//        res.push_back(dist_grad);
//    }
//    return res;
//}

// FIXME: add height param here, something wrong!
std::vector<int> find_peaks(const std::vector<float> &input, float height = std::numeric_limits<float>::lowest()) // TODO: height parameter?
{
	std::vector<int> peaks;
	for (size_t i = 1; i < input.size() - 1; ++i)
	{
		if (input[i] > input[i - 1] && input[i] > input[i + 1] && input[i] > height)
        {
            printk("pushed back peak idx: %d\n", i); // TODO: debug where the inacurracy is between Python and C++
			peaks.push_back(i);
        }
	}
    printk("Done pushing\n");
	return peaks;
}

TimeCycle time_cycle(const std::vector<float> &input, int sys_1, int sys_2, int dia_1)
{
    float dia_2 = std::distance(input.begin() + sys_1,
                  std::min_element(input.begin() + sys_1, input.begin() + sys_2)) +
                  sys_1; // adjust here
    return { dia_2 - dia_1, dia_2 };
}

int dicr_notch(const std::vector<float> &input_ii, int sys_1)
{
    std::vector<float> input_ii_sys_1 = std::vector<float>(input_ii.begin() + sys_1, input_ii.end());
    std::vector<int> peaks = find_peaks(input_ii_sys_1, 0); // still does not get 2 guys here, have to fix it
    if (!peaks.size())
        return -1;

    return peaks[0] + sys_1;
}


/* TODO: can I return temporary vectors?
 * also, I think I need a ringbuffer for returned data as well */
Features extract_features(const std::vector<float> &ppg, const std::vector<float> &ppg_ii)
{
    float sys_1, sys_2, dia_1, dicr, t_start_sys,
          t_sys_end, t_sys_dicr, t_dicr_end, ratio;
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
    ppg_i.reserve(FS);
    ppg_ii.reserve(FS);
    // calculate PPG gradients
	ppg_i = gradient(DUMMY_PPG);
	ppg_ii = gradient(ppg_i);

    //printk("PPG\n");
    //for (int i = 0; i < 10; i+=5)
    //{
    //    printk("%f %f %f %f %f\n",
    //            DUMMY_PPG[i], DUMMY_PPG[i+1], DUMMY_PPG[i+2], DUMMY_PPG[i+3], DUMMY_PPG[i+4]);
    //}

    //printk("PPG_I\n");
    //for (int i = 0; i < 10; i+=5)
    //{
    //    printk("%f %f %f %f %f\n",
    //            ppg_i[i], ppg_i[i+1], ppg_i[i+2], ppg_i[i+3], ppg_i[i+4]);
    //}

    //printk("PPG_II\n");
    //for (int i = 0; i < 10; i+=5)
    //{
    //    printk("%f %f %f %f %f\n",
    //            ppg_ii[i], ppg_ii[i+1], ppg_ii[i+2], ppg_ii[i+3], ppg_ii[i+4]);
    //}

    // extract features
	Features ftrs = extract_features(DUMMY_PPG, ppg_ii);

	// FIXME: dummy
    printk("Before rescaling\n");
    printk("cycle_len: %f orig: %f\n", ftrs.cycle_len, DUMMY_RESULTS[0]);
    printk("t_start_sys: %f orig: %f\n", ftrs.t_start_sys, DUMMY_RESULTS[1]);
    printk("t_sys_end: %f orig: %f\n", ftrs.t_sys_end, DUMMY_RESULTS[2]);
    printk("t_sys_dicr: %f orig: %f\n", ftrs.t_sys_dicr, DUMMY_RESULTS[3]);
    printk("t_dicr_end: %f orig: %f\n", ftrs.t_dicr_end, DUMMY_RESULTS[4]);
    printk("ratio: %f orig: %f\n", ftrs.ratio, DUMMY_RESULTS[5]);

    // prescale the data with values from the dataset
    ftrs.cycle_len = (ftrs.cycle_len - DATA_MEANS[0]) / DATA_STDDEVS[0];
    ftrs.t_start_sys = (ftrs.t_start_sys - DATA_MEANS[1]) / DATA_STDDEVS[1];
    ftrs.t_sys_end = (ftrs.t_sys_end - DATA_MEANS[2]) / DATA_STDDEVS[2];
    ftrs.t_sys_dicr = (ftrs.t_sys_dicr - DATA_MEANS[3]) / DATA_STDDEVS[3];
    ftrs.t_dicr_end = (ftrs.t_dicr_end - DATA_MEANS[4]) / DATA_STDDEVS[4];
    ftrs.ratio = (ftrs.ratio - DATA_MEANS[5]) / DATA_STDDEVS[5];
}
