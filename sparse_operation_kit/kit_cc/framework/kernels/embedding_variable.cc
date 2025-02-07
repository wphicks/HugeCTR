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

#include "embedding_variable.h"

namespace tensorflow {

EmbeddingVariable::EmbeddingVariable(Tensor* handle_tensor)
: handle_tensor_(handle_tensor) 
{}

EmbeddingVariable::~EmbeddingVariable() {}

std::string EmbeddingVariable::DebugString() const {
    return std::string("EmbeddingVariable");
}

void EmbeddingVariable::SetHandle(ResourceHandle& handle) {
    handle_tensor_->scalar<ResourceHandle>()() = handle;
}

Tensor* EmbeddingVariable::tensor() {
    static Tensor tensor(10.0);
    return &tensor;
}

void EmbeddingVariable::set_param(const std::shared_ptr<SparseOperationKit::ParamInterface>& param) {
    param_ = param;
}
void EmbeddingVariable::get_param(std::shared_ptr<SparseOperationKit::ParamInterface>& param) {
    param = param_;
}

mutex* EmbeddingVariable::mu() {
    return &mu_;
}

} // namespace tensorflow
