#pragma once
#include "../daxa.inl"

// DAXA 3.0 Task-Head-Shader interface:
#if DAXA_SHADER
#define DAXA_DECL_TASK_HEAD_BEGIN(HEAD_NAME) \
    struct HEAD_NAME                         \
    {
#define DAXA_TH_IMAGE_NO_SHADER(TASK_ACCESS, NAME)
#define DAXA_TH_IMAGE_ID(TASK_ACCESS, VIEW_TYPE, NAME) daxa_ImageViewId NAME;
#define DAXA_TH_IMAGE_ID_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) daxa_ImageViewId NAME[SIZE];
#define DAXA_TH_IMAGE_ID_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) daxa_ImageViewId NAME[SIZE];
#define DAXA_TH_BUFFER_NO_SHADER(TASK_ACCESS, NAME)
#define DAXA_TH_BUFFER_ID(TASK_ACCESS, NAME) daxa_BufferId NAME;
#define DAXA_TH_BUFFER_PTR(TASK_ACCESS, PTR_TYPE, NAME) PTR_TYPE NAME;
#define DAXA_TH_BUFFER_ID_ARRAY(TASK_ACCESS, NAME, SIZE) daxa_BufferId NAME[SIZE];
#define DAXA_TH_BUFFER_PTR_ARRAY(TASK_ACCESS, PTR_TYPE, NAME, SIZE) PTR_TYPE NAME[SIZE];
#define DAXA_DECL_TASK_HEAD_END \
    }                           \
    ;
#define DAXA_TH_BLOB(HEAD_NAME) HEAD_NAME
#elif __cplusplus
#include "task_graph_types.hpp"
#else // C
#error TASK GRAPH INL ONLY SUPPORTED IN SHADERS AND C++!
#endif