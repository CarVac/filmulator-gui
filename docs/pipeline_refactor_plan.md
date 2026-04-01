# Image Pipeline Refactor Plan

## Goal
Refactor the monolithic `ImagePipeline` class into a modular system of generic **Pipeline Stages**.
This will improve maintainability, testability, and flexibility of the image processing chain.

## User Review Required
> [!IMPORTANT]
> This is a pure refactor. No functionality changes are expected.
> The performance impact should be negligible (compile-time composition preferred).
> **Decision Point**: Should we use compile-time (templates/tuples) or runtime (polymorphism/variants) composition?
> *Recommendation*: **Compile-time** (std::tuple of stages) for type safety and performance, as the pipeline structure is static (always A -> B -> C, even if C is effectively a no-op).

## Concept: Pipeline Stage
A **Stage** is a distinct step in processing. It is defined by:
- **Input Type**: Data it consumes (e.g., `RawImage`, `FloatImage`).
- **Output Type**: Data it produces.
- **Parameters**: Configuration struct (POD).
- **Function**: `Output process(Input, Params, Context)`.

### Requirements
1.  **Type Safety**: Stages must define their I/O types.
2.  **Parameter Isolation**: Stages accept explicit parameter structs, not the full `ParameterManager`.
3.  **Cancellation**: Stages must support checking for abort signals.
4.  **Progress Reporting**: Stages report progress (0.0 to 1.0) which the pipeline scales.
5.  **Caching**: The pipeline (or stage wrapper) caches the output based on parameter hashes/versioning.

## Proposed Architecture

### 1. The Stage Interface
We can use a CRTP or simple Template pattern.

```cpp
template <typename InputT, typename OutputT, typename ParamsT>
class Stage {
public:
    using Input = InputT;
    using Output = OutputT;
    using Params = ParamsT;

    virtual ~Stage() = default;
    
    // Returns std::nullopt if aborted
    virtual std::optional<Output> process(const Input& input, 
                                          const Params& params, 
                                          PipelineContext& context) = 0;
};
```

### 2. The Context
Holds shared state like `ExifData` that might be mutated or read by multiple stages, and the `Interface` for callback (histograms). Or handles cancellation.

```cpp
struct PipelineContext {
    std::atomic<bool> abortParams{false};
    Interface* interface;
    // Helper to check abort status
    bool isAborted();
    // Helper to report progress
    void updateProgress(float p);
};
```

### 3. The Pipeline Coordinator
Manages the sequence of stages and caching.

```cpp
template<typename... Stages>
class Pipeline {
    std::tuple<Stages...> stages;
    std::tuple<typename Stages::Output...> cachedOutputs;
    std::tuple<typename Stages::Params...> cachedParams; // To check if changed
    
    // Logic to run stages:
    // For each stage I:
    //   If valid < Stage[I].validityLevel:
    //      Get Params
    //      Result = Stage[I].process(PreviousResult, Params, Context)
    //      Cache Result
    //      Update valid
};
```
*Note: Since the stages are heterogeneous (different types), the `previousResult` passing needs to be handled via template recursion or fold expressions.*

## Proposed Stages (Mapping)

| Existing Step | Input | Output | Params |
| I | DataFlow | DataFlow | `claim...Params` |
| --- | --- | --- | --- |
| **Loader** | `std::string` (Filename) | `RawImage` + `Exif` | `LoadParams` |
| **Demosaic** | `RawImage` | `FloatImage` (Crop) | `DemosaicParams` |
| **PostDemosaic** | `FloatImage` | `FloatImage` | `PostDemosaicParams` |
| **Denoise (NL)** | `FloatImage` | `FloatImage` (Lab) | `NlmeansNRParams` |
| **Denoise (Imp)**| `FloatImage` | `FloatImage` (Lab) | `ImpulseNRParams` |
| **Denoise (Chr)**| `FloatImage` | `FloatImage` (Lab) | `ChromaNRParams` |
| **PreFilm** | `FloatImage` | `FloatImage` | `PrefilmParams` |
| **Filmulation** | `FloatImage` | `FloatImage` (sRGB) | `FilmParams` |
| **Compositing** | `FloatImage` | `ShortImage` | `BlackWhiteParams` + ... |

## Implementation Plan

### Phase 1: Infrastructure
1.  Define `PipelineContext` and base `Stage` templates in `core/pipeline/`.
2.  Implement a simple `Pipeline` class that can chain 2 stages.

### Phase 2: Migration (Incremental)
We can migrate stages one by one.
1.  Create `LoadStage` class moving code from `processImage` case `Valid::load`.
2.  Create `DemosaicStage` moving code from `Valid::demosaic`.
3.  ...
For the transition, `ImagePipeline` will hold instances of these Stages and call them manually, replacing the inline code.

### Phase 3: The New Pipeline
1.  Once all logic is in Stage classes, replace the `ImagePipeline::processImage` giant switch with the generic `Pipeline` runner.
2.  Wire up `ParameterManager` to feed the `Pipeline`.

## Verification Plan
1.  **Unit Tests**:
    - Test `Pipeline` generic logic (caching, chaining).
    - Test individual `Stage` classes in isolation (mock inputs).
    - *New*: Create `tests/unit/test_pipeline_infrastructure.cpp`.
2.  **Regression Tests**:
    - Run `test_raw_pipeline.py` (End-to-End) to ensure the output image is identical pixel-for-pixel (or within float tolerance) to the current implementation.
    - Since we have "Golden Data", this is perfect.
