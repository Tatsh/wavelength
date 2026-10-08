#include "gfx/camslide.h"

#include "os/string.h"
#include "rnd/manager.h"

Rnd::Transformable *CamSlide::sSlide = nullptr;

void CamSlide::Init() {
    sSlide =
        dynamic_cast<Rnd::Transformable *>(Rnd::TheManager.Find(FormatString("tnl cam slide1")));
}

void CamSlide::Terminate() {
    sSlide = nullptr;
}
