#include <torch/csrc/lazy/core/config.h>

C10_DEFINE_bool(torch_lazy_ir_debug, false, "Enable lazy tensor IR debugging");

C10_DEFINE_bool(
    torch_lazy_param_aliasing,
    false,
    "Enable parameter aliasing support");

C10_DEFINE_int(
    torch_lazy_compilation_cache_size,
    1024,
    "Size of the compilation cache");

C10_DEFINE_int(
    torch_lazy_device_data_cache_size,
    128,
    "Size of the DeviceData cache");

C10_DEFINE_int(
    torch_lazy_io_thread_pool_size,
    // TODO: measure which default value will give better
    // performance, std::thread::hardware_concurrency()?
    1,
    "Size of the execution thread pool");

C10_DEFINE_int(torch_lazy_metrics_samples, 1024, "Max metrics sample size");

C10_DEFINE_int(
    torch_lazy_trim_graph_check_frequency,
    5000,
    "How often to check for whether a graph needs to be split");

C10_DEFINE_int(
    torch_lazy_trim_graph_size,
    100000,
    "The threshold (in terms of the number of nodes) for splitting a graph");

C10_DEFINE_string(
    torch_lazy_metrics_percentiles,
    "0.01:0.05:0.1:0.2:0.5:0.8:0.9:0.95:0.99",
    "Metrics percentiles to be collected, using : as the delimiter");
