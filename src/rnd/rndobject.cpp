#include "rnd/rndobject.h"

#include "rnd/rndmanager.h"

RndObject::RndObject(const char *pszName) : mName(pszName), mInternal(0), mDeleting(0) {
    TheManager.SetObject(mName.c_str(), this);
}

RndObject::~RndObject() {
    mDeleting = 1;
    mRefs.unique();
    for (auto it = mRefs.begin(); it != mRefs.end();) {
        RndObject *pReferrer = *it;
        ++it;
        pReferrer->Replace(this, nullptr);
    }
    TheManager.RemoveObject(mName.c_str());
}

void RndObject::DumpText(PrnStream &stream) {
    stream << "[RndObject]\n";
    PrnStream &line = stream << "name:";
    line.Print(mName.c_str());
    line << " class:" << ClassName() << " internal:" << (mInternal != 0) << "\n";
    if (stream.GetDumpLevel() > 0) {
        stream << "references:" << mRefs << "\n";
    }
}

void RndObject::SetName(const char *pszName) {
    if (mName == pszName) {
        return;
    }
    TheManager.RemoveObject(mName.c_str());
    mName = pszName;
    TheManager.SetObject(mName.c_str(), this);
}

void RndObject::AddRef(RndObject *pReferrer) {
    if (pReferrer != this) {
        mRefs.push_back(pReferrer);
    }
}

void RndObject::RemoveRef(RndObject *pReferrer) {
    if (mDeleting != 0) {
        return;
    }
    for (auto it = mRefs.begin(); it != mRefs.end(); ++it) {
        if (*it == pReferrer) {
            mRefs.erase(it);
            return;
        }
    }
}

PrnStream &operator<<(PrnStream &stream, const RndObject *pObject) {
    if (pObject != nullptr) {
        stream << pObject->GetName();
    } else {
        stream << "no object";
    }
    return stream;
}
