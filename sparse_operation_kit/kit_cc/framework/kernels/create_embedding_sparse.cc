/*
 * Copyright (c) 2021, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "facade.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "embedding_variable.h"
#include <exception>
#include <string>
#include <vector>

namespace tensorflow {
using GPUDevice = Eigen::GpuDevice;
using CPUDevice = Eigen::ThreadPoolDevice; 

template <typename Device>
class CreateEmbeddingSparseOp : public OpKernel {
public:
    explicit CreateEmbeddingSparseOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
        OP_REQUIRES_OK(ctx, ctx->GetAttr("slot_num", &slot_num_));
        OP_REQUIRES_OK(ctx, ctx->GetAttr("max_nnz", &max_nnz_));
        OP_REQUIRES_OK(ctx, ctx->GetAttr("max_feature_num", &max_feature_num_));
        if (1 == max_feature_num_) max_feature_num_ = slot_num_ * max_nnz_;
        OP_REQUIRES_OK(ctx, ctx->GetAttr("combiner", &combiner_));
        OP_REQUIRES_OK(ctx, ctx->GetAttr("input_dispatcher_subsequent_ops", &input_dispatcher_subsequent_ops_));
        OP_REQUIRES_OK(ctx, ctx->GetAttr("output_dispatcher_subsequent_ops", &output_dispatcher_subsequent_ops_));
    }
    void Compute(OpKernelContext* ctx) override {
        core::RefCountPtr<EmbeddingVariable> embedding_variable;
        OP_REQUIRES_OK(ctx, LookupResource(ctx, HandleFromInput(ctx, 0), &embedding_variable));
        Tensor const *input_dispatcher_tensor = nullptr;
        OP_REQUIRES_OK(ctx, ctx->input("input_dispatcher", &input_dispatcher_tensor));
        Tensor const *embedding_executor_tensor = nullptr;
        OP_REQUIRES_OK(ctx, ctx->input("embedding_executor", &embedding_executor_tensor));
        Tensor const *output_dispatcher_tensor = nullptr;
        OP_REQUIRES_OK(ctx, ctx->input("output_dispatcher", &output_dispatcher_tensor));

        Tensor* emb_handle_tensor = nullptr;
        OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}), &emb_handle_tensor));

        try {
            SparseOperationKit::Facade::instance()->create_embedding_sparse(embedding_variable, 
                                                                         input_dispatcher_tensor->flat<tstring>()(0),
                                                                         input_dispatcher_subsequent_ops_,
                                                                         embedding_executor_tensor->flat<tstring>()(0),
                                                                         output_dispatcher_tensor->flat<tstring>()(0),
                                                                         output_dispatcher_subsequent_ops_,
                                                                         slot_num_, max_nnz_, max_feature_num_, combiner_,
                                                                         emb_handle_tensor);

        } catch (const std::exception& error) {
            ctx->SetStatus(errors::Aborted(error.what()));
            return;
        }
    }
private:
    int32_t slot_num_;
    int32_t max_nnz_;
    int32_t max_feature_num_;
    std::string combiner_;
    std::vector<std::string> input_dispatcher_subsequent_ops_;
    std::vector<std::string> output_dispatcher_subsequent_ops_;
};

REGISTER_KERNEL_BUILDER(Name("CreateEmbeddingSparse")
                        .Device(DEVICE_GPU)
                        .HostMemory("var_handle")
                        .HostMemory("input_dispatcher")
                        .HostMemory("embedding_executor")
                        .HostMemory("output_dispatcher")
                        .HostMemory("emb_handle"),
                        CreateEmbeddingSparseOp<GPUDevice>);


} // namespace tensorflow