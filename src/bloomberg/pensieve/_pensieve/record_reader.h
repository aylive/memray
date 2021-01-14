#pragma once

#include <assert.h>
#include <fstream>
#include <functional>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Python.h"

#include "python_helpers.h"
#include "records.h"

namespace pensieve::api {

using namespace tracking_api;

class StackTraceTree
{
  public:
    using index_t = uint32_t;
    struct Node
    {
        frame_id_t frame_id;
        index_t parent_index;
    };

    inline const Node& nextNode(index_t index) const
    {
        assert(1 <= index && index <= d_graph.size());
        return d_graph[index - 1];
    }

    size_t getTraceIndex(const std::vector<frame_id_t>& stack_trace);

  private:
    struct NodeEdge
    {
        frame_id_t frame_id;
        index_t index;
        std::vector<NodeEdge> children;
    };
    NodeEdge d_root = {0, 0, {}};
    size_t d_current_tree_index = 1;
    std::vector<Node> d_graph{};
};

using reduced_snapshot_map_t = std::unordered_map<StackTraceTree::index_t, Allocation>;
using allocations_t = std::vector<Allocation>;

class RecordReader
{
  public:
    explicit RecordReader(const std::string& file_name);
    PyObject* Py_NextAllocationRecord();
    PyObject* Py_GetStackFrame(StackTraceTree::index_t index, size_t max_stacks = 0);
    PyObject* Py_HighWatermarkAllocationRecords();

    size_t totalAllocations() const noexcept;
    size_t totalFrames() const noexcept;

  private:
    // Aliases
    using stack_t = std::vector<frame_id_t>;
    using stack_traces_t = std::unordered_map<thread_id_t, stack_t>;
    using allocations_t = std::vector<Allocation>;

    // Data members
    std::ifstream d_input;
    HeaderRecord d_header;
    pyframe_map_t d_frame_map{};
    FrameCollection<Frame> d_allocation_frames;
    stack_traces_t d_stack_traces{};
    StackTraceTree d_tree{};
    mutable python_helpers::PyUnicode_Cache d_pystring_cache{};

    // Methods
    void parseFrame();
    void parseFrameIndex();
    AllocationRecord parseAllocationRecord();
    allocations_t parseAllocations();
    void correctAllocationFrame(stack_t& stack, int lineno);
    size_t getAllocationFrameIndex(const AllocationRecord& record);
};

}  // namespace pensieve::api
