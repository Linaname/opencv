// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "precomp.hpp"

#include "legacy_backend.hpp"

#include "op_inf_engine.hpp"
#include "ie_ngraph.hpp"
#include "op_vkcom.hpp"
#include "op_cuda.hpp"
#include "op_webnn.hpp"
#include "op_timvx.hpp"
#include "op_cann.hpp"

namespace cv {
namespace dnn {
CV__DNN_INLINE_NS_BEGIN


BackendNode::BackendNode(int backendId)
    : backendId(backendId)
{}

BackendNode::~BackendNode() {}

BackendWrapper::BackendWrapper(int backendId, int targetId)
    : backendId(backendId)
    , targetId(targetId)
{}

BackendWrapper::BackendWrapper(int targetId, const cv::Mat& m)
{
    CV_Error(Error::StsNotImplemented,
            "Constructor of backend wrapper must be implemented");
}

BackendWrapper::BackendWrapper(const Ptr<BackendWrapper>& base, const MatShape& shape)
{
    CV_Error(Error::StsNotImplemented,
            "Constructor of backend wrapper must be implemented");
}

BackendWrapper::~BackendWrapper() {}



inline namespace detail {


Ptr<BackendWrapper> wrapMat(int backendId, int targetId, cv::Mat& m)
{
    if (backendId == DNN_BACKEND_OPENCV)
    {
        if (targetId == DNN_TARGET_CPU)
            return Ptr<BackendWrapper>();
#ifdef HAVE_OPENCL
        else if (IS_DNN_OPENCL_TARGET(targetId))
            return OpenCLBackendWrapper::create(m);
#endif
        else
            CV_Error(Error::StsNotImplemented, "Unknown/unsupported target identifier");
    }
    else if (backendId == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019)
    {
        CV_ERROR_DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019;
    }
    else if (backendId == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
    {
        CV_Assert(0 && "Internal error: DNN_BACKEND_INFERENCE_ENGINE_NGRAPH must be implemented through inheritance");
    }
    else if (backendId == DNN_BACKEND_WEBNN)
    {
#ifdef HAVE_WEBNN
        return Ptr<BackendWrapper>(new WebnnBackendWrapper(targetId, m));
#else
        CV_Error(Error::StsNotImplemented, "This OpenCV version is built without support of WebNN");
#endif
    }
    else if (backendId == DNN_BACKEND_VKCOM)
    {
        CV_Assert(haveVulkan());
#ifdef HAVE_VULKAN
        return Ptr<BackendWrapper>(new VkComBackendWrapper(m));
#endif  // HAVE_VULKAN
    }
    else if (backendId == DNN_BACKEND_CUDA)
    {
        CV_Assert(haveCUDA());

#ifdef HAVE_CUDA
        CV_CheckType(m.depth(), m.depth() == CV_32F || m.depth() == CV_8S || m.depth() == CV_8U || m.depth() == CV_32S || m.depth() == CV_64S || m.depth() == CV_Bool, "Unsupported type for CUDA");
        CV_Assert(IS_DNN_CUDA_TARGET(targetId));
        switch (m.depth())
        {
        case CV_32F:
            if (targetId == DNN_TARGET_CUDA_FP16)
                return CUDABackendWrapperFP16::create(m);
            else
                return CUDABackendWrapperFP32::create(m);
        case CV_8S:
            return CUDABackendWrapperINT8::create(m);
        case CV_8U:
            return CUDABackendWrapperUINT8::create(m);
        case CV_32S:
            return CUDABackendWrapperINT32::create(m);
        case CV_64S:
            return CUDABackendWrapperINT64::create(m);
        case CV_Bool:
            return CUDABackendWrapperBOOL::create(m);
        default:
            CV_Error(Error::BadDepth, "Unsupported mat type for CUDA");
        }
#endif
    }
    else if (backendId == DNN_BACKEND_TIMVX)
    {
        CV_Assert(haveTimVX());
#ifdef HAVE_TIMVX
        return Ptr<BackendWrapper>(new TimVXBackendWrapper(m));
#endif  // HAVE_TIMVX
    }
    else if (backendId == DNN_BACKEND_CANN)
    {
        CV_Assert(0 && "Internal error: DNN_BACKEND_CANN must be implemented through inheritance");
    }
    else
        CV_Error(Error::StsNotImplemented, "Unknown backend identifier");
    return Ptr<BackendWrapper>();  // TODO Error?
}  // wrapMat()


}  // namespace detail
CV__DNN_INLINE_NS_END
}}  // namespace cv::dnn
