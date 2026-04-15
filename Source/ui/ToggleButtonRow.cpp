#include "ToggleButtonRow.h"
#include "CommonIncludes.h"
namespace starflux::ui
{
ToggleButtonRow::ToggleButtonRow()
{
    for (auto* b : { &constellation, &voidMode, &crt, &twinkle, &wave })
        addAndMakeVisible(*b);
}

void ToggleButtonRow::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / 5;
    constellation.setBounds(r.removeFromLeft(w).reduced(2));
    voidMode.setBounds(r.removeFromLeft(w).reduced(2));
    crt.setBounds(r.removeFromLeft(w).reduced(2));
    twinkle.setBounds(r.removeFromLeft(w).reduced(2));
    wave.setBounds(r.reduced(2));
}
}
