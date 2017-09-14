#include "simulator.h"
#include "nn_branch_predictor.h"
NNBranchPredictor::NNBranchPredictor(String name, core_id_t core_id)
   : BranchPredictor(name, core_id)
   , last_sum(0)
   , last_bias_index(0)
   , last_indexes(128 / 8, 0)
   , last_path(128, PathInfo(false, 0))
   , last_path_index(0)
   , path(128, PathInfo(false, 0))
   , path_index(0)
   , path_length(128)
   , num_entries(512)
   , num_bias_entries(2048)
   , weight_bits(7)
   , weights(512, std::vector<SInt32>(128,  0))
   , bias_weights(2048, 0)
   , theta(2.14 * (128 + 1) + 20.58)
   , tc(0)
   , block_size(8)
   , coefficients(128, 0)
{
   double a = 0.04;
   double b = 0.05;
   bias_coefficient = 1.0 / a;
   for (int i = 0; i < 128; i++) {
      coefficients[i] = 1.0 / (a + b * (i + 1));
   }
}

NNBranchPredictor::~NNBranchPredictor()
{
}

bool NNBranchPredictor::predict(IntPtr ip, IntPtr target)
{
   double sum = 0;
   UInt32 bias_index = bias_hash (ip);
   sum += bias_coefficient * bias_weights[bias_index];
   last_bias_index = bias_index;
   for (UInt32 i = 0; i < path_length; i += block_size) {
      UInt32 index = index_entries(ip, i);
      last_indexes[i / block_size] = index;
      for (UInt32 j = 0; j < block_size; j++) {
         SInt32 h = path[(path_index - i - j + path_length - 1) % path_length].taken ? 1 : -1;
         sum += h * weights[index][i + j] * coefficients[i + j];
      }
   }
   bool result = ((SInt32) sum) >= 0;
   last_sum = (SInt32) sum;

   last_path.assign(path.begin(), path.end());
   last_path_index = path_index;
   update_path(result, ip, target);
   
   return result;
}

void NNBranchPredictor::update(bool predicted, bool actual, IntPtr ip, IntPtr target)
{
   updateCounters(predicted, actual);

   if (predicted != actual) {
      tc++;
      if (tc >= 1) {
         theta++;
         tc = 0;
      }
   }
   if (predicted == actual && abs(last_sum) < theta) {
      tc--;
      if (tc <= -1) {
         theta--;
         tc = 0;
      }
   }
   if (!(predicted == actual && abs(last_sum) >= theta)) {
      UInt32 bias_index = last_bias_index;
      if (actual) {
         if (bias_weights[bias_index] < (1 << weight_bits) - 1) bias_weights[bias_index]++;
      } else {
         if (bias_weights[bias_index] > - (1 << weight_bits)) bias_weights[bias_index]--;
      }
      for (UInt32 i = 0; i < path_length; i += block_size) {
         UInt32 index = last_indexes[i / block_size];
         for (UInt32 j = 0; j < block_size; j++) {
            bool taken = last_path[(last_path_index - i - j + path_length - 1) % path_length].taken;
            if (taken == actual) {
               if (weights[index][i + j] < (1 << weight_bits) - 1) weights[index][i + j]++;
            } else {
               if (weights[index][i + j] > - (1 << weight_bits)) weights[index][i + j]--;
            }
         }
      }
   }

    if (predicted != actual) {
      path.assign(last_path.begin(), last_path.end());
      path_index = last_path_index;
      update_path(actual, ip, target);
   }
}

UInt32 NNBranchPredictor::bias_hash(IntPtr ip) const
{
   return (ip >> 4) % num_bias_entries;
}
UInt32 NNBranchPredictor::index_entries(IntPtr ip, UInt32 i) const
{
   IntPtr z = ip >> 4;
   for (UInt32 j = 0; j < block_size; j++)
   {
      z ^= (path[(path_index - i - j + path_length - 1) % path_length].target >> 4);
   }
   return z % num_entries;
}
void NNBranchPredictor::update_path(bool taken, IntPtr ip, IntPtr target)
{
   path[path_index] = PathInfo(taken, target);
   path_index = (path_index + 1) % path_length;
}