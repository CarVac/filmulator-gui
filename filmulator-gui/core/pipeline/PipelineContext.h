#ifndef PIPELINE_CONTEXT_H
#define PIPELINE_CONTEXT_H

#include "../interface.h"
#include <atomic>

namespace Pipeline {

enum Cache { HighCache, NoCache };
enum Histo { WithHisto, NoHisto };
enum QuickQuality { LowQuality, PreviewQuality, HighQuality };

struct PipelineContext
{
  // Shared interface for histograms / callbacks
  Interface *interface = nullptr;
  class ParameterManager *paramManager = nullptr;

  // Pipeline configuration
  Cache cache = NoCache;
  Histo histo = NoHisto;
  QuickQuality quality = HighQuality;
  int resolution = 0;

  // Abort flag
  std::atomic<bool> abortParams{ false };

  bool isAborted() const { return abortParams.load(); }

  void updateProgress(float p)
  {
    if (interface) { interface->setProgress(p); }
  }
};

}// namespace Pipeline

#endif// PIPELINE_CONTEXT_H
