#pragma once

#include <memory>

class LuxSetterThread
{
  public:
    LuxSetterThread();
    ~LuxSetterThread();

    void setSensorLuminance(float lux);

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};