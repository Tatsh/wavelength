#include "memcard/mcunformatcardtask.h"

MCUnformatCardTask::MCUnformatCardTask() {
    mGetInfo = new MCGetInfoTask;
    mUnformat = new MCUnformatTask;
    Add(mGetInfo);
    Add(mUnformat);
}
