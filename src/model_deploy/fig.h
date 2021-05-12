#ifndef CONFIG_H_
#define CONFIG_H_

// The number of labels (without negative)
#define label_num 4
struct Config {
  int seq_length = 64;
  int consecutiveInferenceThresholds[label_num]  = {20, 10};
  char* output_message[label_num] = {"30:\n\r","40:\n\r","50:\n\r","60:\n\r"};
};
struct Config config;
#endif // CONFIG_H_
