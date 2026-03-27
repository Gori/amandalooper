#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

class AmlpUIBehaviour : public te::UIBehaviour
{
public:
    AmlpUIBehaviour() = default;

    void runTaskWithProgressBar (te::ThreadPoolJobWithProgress&) override {}
};
