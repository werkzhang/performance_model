#ifndef NN_BRANCH_PREDICTOR_H
#define NN_BRANCH_PREDICTOR_H

#include "branch_predictor.h"

#include <vector>

class NNBranchPredictor : public BranchPredictor
{
public:
   NNBranchPredictor(String name, core_id_t core_id);
   ~NNBranchPredictor();

   bool predict(IntPtr ip, IntPtr target); //预测方法
   void update(bool predicted, bool actual, IntPtr ip, IntPtr target);//训练策略

private:

   void update_path(bool taken, IntPtr ip, IntPtr target);//更新历史
   UInt32 bias_hash(IntPtr ip) const; //偏向权重的索引方式
   UInt32 index_entries(IntPtr ip, UInt32 i) const; // 权重的索引方式


   struct PathInfo {
      bool taken;
      IntPtr target;
      PathInfo(bool bt, IntPtr it): taken(bt), target(it) {}
   };

   SInt32 last_sum;
   UInt32 last_bias_index;
   std::vector<UInt32> last_indexes;
   std::vector<PathInfo> last_path;
   UInt32 last_path_index;  // last_*用于存储中间结果


   std::vector<PathInfo> path;
   UInt32 path_index; 

   UInt32 path_length; //128
   UInt32 num_entries; //512
   UInt32 num_bias_entries; // 2048
   UInt32 weight_bits; //[-128,127]
   std::vector<std::vector<SInt32> > weights; //权重，512*128
   std::vector<SInt32> bias_weights; //偏向权重2048
   UInt32 theta;//阈值，自适应更新
   SInt64  tc;//用于更新阈值的计数器
   UInt32 block_size;//权重8列=1个小表

   std::vector<double> coefficients; //系数
   double bias_coefficient;//偏向系数

};

#endif
