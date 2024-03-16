#pragma once

#include "daxa/daxa.inl"
#include "daxa/utils/task_graph.inl"

DAXA_DECL_TASK_HEAD_BEGIN(DrawTri, 1)
DAXA_TH_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, color)
DAXA_DECL_TASK_HEAD_END