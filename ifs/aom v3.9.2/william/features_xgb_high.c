#include <math.h>
#include <string.h>

double sigmoid_high(double x) {
  if (x < 0.0) {
    double z = exp(x);
    return z / (1.0 + z);
  }
  return 1.0 / (1.0 + exp(-x));
}

void score_high(double *input, double *output) {
  double var0;
  var0 = sigmoid_high(
      0.0441582 + input[0] * 0.000922761 + input[1] * 0.00000000685823 +
      input[2] * -0.00095569 + input[3] * -0.00000386135 +
      input[4] * -0.0337991 + input[5] * 0.0000000208774 +
      input[6] * 0.000000992765 + input[7] * 0.185731 + input[8] * -4.08987 +
      input[9] * -4.2971 + input[10] * -0.861844 + input[11] * -1.63982 +
      input[12] * -0.321852 + input[13] * -2.45576 + input[14] * -0.742767 +
      input[15] * -1.58181 + input[16] * -1.51445 + input[17] * -2.52619 +
      input[18] * -0.982841 + input[19] * -0.568831 + input[20] * 0.863846 +
      input[21] * 0.637639 + input[22] * -4.36437 + input[23] * -2.27726 +
      input[24] * -2.28682 + input[25] * -2.83687 + input[26] * -0.727786 +
      input[27] * -1.12208 + input[28] * 0.585536 + input[29] * -0.180809 +
      input[30] * -1.01042 + input[31] * 0.0476199 + input[32] * -0.719225 +
      input[33] * -1.58005 + input[34] * 0.74608 + input[35] * 0.182025 +
      input[36] * 0.739979 + input[37] * -0.344918 + input[38] * -0.502221 +
      input[39] * -0.930049 + input[40] * -0.0246612 + input[41] * 0.0340566 +
      input[42] * -1.35029 + input[43] * -0.106883 + input[44] * 0.0188809 +
      input[45] * -0.28491 + input[46] * 0.0432745 + input[47] * 0.000987391 +
      input[48] * 0.197411);
  memcpy(output, (double[]){ 1.0 - var0, var0 }, 2 * sizeof(double));
}
