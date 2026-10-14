#include "gfx/camslide.h"

#include "os/string.h"
#include "rnd/rndmanager.h"

RndTransformable *CamSlide::sSlide = nullptr;

void CamSlide::Init() {
    sSlide = dynamic_cast<RndTransformable *>(TheManager.Find(FormatString("tnl cam slide1")));
}

void CamSlide::Terminate() {
    sSlide = nullptr;
}
