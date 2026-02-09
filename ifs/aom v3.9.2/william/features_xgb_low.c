#include <math.h>
#include <string.h>

double sigmoid_low(double x) {
  if (x < 0.0) {
    double z = exp(x);
    return z / (1.0 + z);
  }
  return 1.0 / (1.0 + exp(-x));
}

void score_low(double *input, double *output) {
  double var0;
  var0 = sigmoid_low(
      0.935805 + input[0] * 0.000874377 + input[1] * 0.0000000157749 +
      input[2] * -0.0016097 + input[3] * -0.0000091842 + input[4] * 0.00315914 +
      input[5] * 0.0000111448 + input[6] * -0.2515 + input[7] * 0.000000088069 +
      input[8] * -0.426958 + input[9] * 0.00000267069 + input[10] * 1.89358 +
      input[11] * -1.87096 + input[12] * -0.00000154659 +
      input[13] * -0.997648 + input[14] * 0.653952 + input[15] * 0.113239 +
      input[16] * -3.6987 + input[17] * -3.28798 + input[18] * -0.650261 +
      input[19] * -1.26859 + input[20] * 0.322862 + input[21] * -1.89284 +
      input[22] * -0.310017 + input[23] * -1.28492 + input[24] * -1.38011 +
      input[25] * -2.23757 + input[26] * -0.638791 + input[27] * 0.2877 +
      input[28] * 1.3821 + input[29] * 1.11755 + input[30] * -3.35453 +
      input[31] * -1.87739 + input[32] * -2.14836 + input[33] * -2.49794 +
      input[34] * -0.343697 + input[35] * -0.722757 + input[36] * 1.09403 +
      input[37] * 0.252322 + input[38] * -0.0436162 + input[39] * -0.524317 +
      input[40] * 0.36933 + input[41] * 0.30864 + input[42] * -0.636212 +
      input[43] * 0.284228 + input[44] * 0.270819 + input[45] * -0.107911 +
      input[46] * 0.298111 + input[47] * 0.232039 + input[48] * 0.311056);
  memcpy(output, (double[]){ 1.0 - var0, var0 }, 2 * sizeof(double));
}
