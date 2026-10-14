#include "rnd/rndloader.h"

#include <cstring>

#include "os/chunkstream.h"
#include "os/debug.h"
#include "os/fileutil.h"
#include "os/system.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmovie.h"
#include "rnd/rndtex.h"

namespace {

// The deadline of a synchronous load, thirty days in milliseconds.
constexpr float kNoDeadlineMs = 2592000000.0f;

// The texture reads an asynchronous load keeps in flight.
constexpr int kTextureSlots = 8;

// The block size of the stream LoadFile() opens.
constexpr int kChunkSize = 65536;

// The sizes of the names and the paths the tables record.
constexpr int kClassNameSize = 32;
constexpr int kObjectNameSize = 128;
constexpr int kPathSize = 256;

// The versions that changed the format.
constexpr int kFirstFlagVersion = 1;
constexpr int kFirstMarkerVersion = 2;
constexpr int kFirstMixinNamesVersion = 3;
constexpr int kFirstSpriteVersion = 4;
constexpr int kFirstMovieVersion = 5;
constexpr int kFirstGeneratorVersion = 6;
constexpr int kFirstAsyncVersion = 7;
constexpr int kLastFlagVersion = 7;
constexpr int kFirstLineVersion = 7;
constexpr int kFirstTextureListVersion = 9;
constexpr int kFirstTextureOwnerVersion = 10;

// The bytes of the end marker 0xdeaddead, in the order a record ends with them.
constexpr unsigned char kMarkerLow = 0xad;
constexpr unsigned char kMarkerHigh = 0xde;

// Read up to and through the end marker of a record.
inline void SkipToMarker(BinStream &stream) {
    unsigned char nByte;
    stream.Read(&nByte, sizeof(nByte));
    unsigned char nFirst = nByte;
    for (;;) {
        if (nFirst != kMarkerLow) {
            stream.Read(&nByte, sizeof(nByte));
            nFirst = nByte;
            continue;
        }
        stream.Read(&nByte, sizeof(nByte));
        const unsigned char nSecond = nByte;
        if (nSecond != kMarkerHigh) {
            nFirst = nSecond;
            continue;
        }
        stream.Read(&nByte, sizeof(nByte));
        if (nByte != nFirst) {
            // Yes, the binary discards the mismatched byte without testing it as a new start.
            stream.Read(&nByte, sizeof(nByte));
            nFirst = nByte;
            continue;
        }
        stream.Read(&nByte, sizeof(nByte));
        if (nByte == nSecond) {
            return;
        }
        nFirst = nByte;
    }
}

} // namespace

void RndLoader::ObjectLoader::Load(BinStream &stream, int nVersion) {
    if (mLoad != 0) {
        mObject->Load(stream);
        if (nVersion < kFirstMarkerVersion) {
            return;
        }
        SkipToMarker(stream);
        return;
    }
    if (nVersion < kFirstMarkerVersion) {
        RndObject *pTemp = TheManager.Create(mObject->ClassName(), "_TEMP_");
        pTemp->Load(stream); // Yes, the binary loads before it tests for null.
        if (pTemp != nullptr) {
            delete pTemp;
        }
        return;
    }
    SkipToMarker(stream);
}

void RndLoader::ObjectLoader::Release() {
    if (mDelete != 0 && mObject != nullptr) {
        delete mObject;
    }
}

RndLoader::RndLoader(const char *pszFile, int nFlags, Callback *pCallback, BinStream *pStream)
    : mOwnsStream(0), mFile(pszFile), mFlags(nFlags), mStream(pStream),
      mTextures(kTextureSlots, FilePath()), mCallback(pCallback) {
    if (mFile.mLength == 0) {
        mDir = FilePath::sRoot.c_str();
    } else {
        mDir = FileGetPath(mFile.c_str());
    }
    mState = &RndLoader::LoadFile;
    if ((mFlags & kAsync) == 0) {
        (this->*mState)(kNoDeadlineMs);
    }
}

RndLoader::~RndLoader() {
    Finish();
    if ((mFlags & kDeleteObjects) != 0) {
        for (RndObject *pObject : mObjects) {
            delete pObject;
        }
    }
    TheManager.RemoveLoader(this);
}

bool RndLoader::IsLoaded() const {
    return mState == &RndLoader::DoneLoading;
}

void RndLoader::Poll(float fDeadlineMs) {
    (this->*mState)(fDeadlineMs);
}

void RndLoader::LoadFile(float fDeadlineMs) {
    if (mStream == nullptr) {
        mStream = new ChunkStream(mFile.c_str(), ChunkStream::kModeRead, kChunkSize, true, true);
        mOwnsStream = 1;
        if (mStream->Fail()) {
            DebugNotify("Could not open file: %s", mFile.c_str());
            Finish();
            return;
        }
    }
    mState = &RndLoader::PreloadHeader;
    (this->*mState)(fDeadlineMs);
}

void RndLoader::RemapClassName(char *pszClass) {
    if (mVersion < kFirstMixinNamesVersion) {
        if (strcmp(pszClass, "AnimObject") == 0) {
            strcpy(pszClass, "Animatable");
        } else if (strcmp(pszClass, "DrawObject") == 0) {
            strcpy(pszClass, "Drawable");
        } else if (strcmp(pszClass, "CollideObject") == 0) {
            strcpy(pszClass, "Collideable");
        } else if (strcmp(pszClass, "TransObject") == 0) {
            strcpy(pszClass, "Transformable");
        }
    }
    if (mVersion < kFirstSpriteVersion && strcmp(pszClass, "DrawRect") == 0) {
        strcpy(pszClass, "Sprite");
    }
    if (mVersion < kFirstMovieVersion && strcmp(pszClass, "TexMovie") == 0) {
        strcpy(pszClass, "Movie");
    }
    if (mVersion < kFirstGeneratorVersion && strcmp(pszClass, "MeshGenerator") == 0) {
        strcpy(pszClass, "Generator");
    }
    if (mVersion < kFirstLineVersion && strcmp(pszClass, "String") == 0) {
        strcpy(pszClass, "Line");
    }
}

void RndLoader::PreloadHeader(float fDeadlineMs) {
    while (mStream->Eof()) {
        if (fDeadlineMs < SystemMs()) {
            return;
        }
    }
    mStream->ReadEndian(&mVersion, sizeof(mVersion));
    if (RndManager::sRev < mVersion) {
        DebugNotify("Can't load new Manager in %s", mFile.c_str());
        Finish();
        return;
    }
    // Files older than the async version store their names as NUL-terminated text.
    if (mVersion < kFirstAsyncVersion) {
        if ((mFlags & kAsync) != 0) {
            DebugNotify("Can't load %s async", mFile.c_str());
            Finish();
            return;
        }
        SetStringReadsText(1);
    }
    mStream->ReadEndian(&mRemaining, sizeof(mRemaining));
    if (mCallback != nullptr) {
        mCallback->SetObjectCount(mRemaining);
    }
    mState = &RndLoader::CreateObjects;
    (this->*mState)(fDeadlineMs);
}

void RndLoader::CreateObjects(float fDeadlineMs) {
    while (mRemaining-- != 0) {
        char szClass[kClassNameSize];
        mStream->ReadString(szClass, sizeof(szClass));
        RemapClassName(szClass);
        char szName[kObjectNameSize];
        mStream->ReadString(szName, sizeof(szName));
        RndObject *pObject = TheManager.Find(szName);
        if (mVersion >= kFirstFlagVersion && mVersion <= kLastFlagVersion) {
            unsigned char nUnused;
            mStream->Read(&nUnused, sizeof(nUnused)); // Yes, the binary discards this flag.
        }

        int bLoad;
        if (mCallback != nullptr) {
            const int nVerdict = mCallback->ShouldLoad(pObject, szName, szClass);
            if (nVerdict < 0) {
                return; // Yes, the binary stops without ending the load.
            }
            bLoad = nVerdict != 0;
        } else {
            bLoad = pObject == nullptr;
        }

        if (pObject != nullptr) {
            if (pObject->mInternal != 0 || strcmp(pObject->ClassName(), szClass) != 0) {
                DebugNotify("Can't load object %s", szName);
                Finish();
                return;
            }
            mObjectLoaders.push_back(ObjectLoader{pObject, bLoad, 0});
        } else {
            pObject = TheManager.Create(szClass, szName);
            if (pObject != nullptr) {
                mObjectLoaders.push_back(ObjectLoader{pObject, bLoad, bLoad ^ 1});
                if (bLoad != 0) {
                    mObjects.push_back(pObject);
                }
            } else {
                DebugWarn("Failed to create object %s of class %s", szName, szClass);
            }
        }
        if (fDeadlineMs < SystemMs()) {
            return;
        }
    }

    if ((mFlags & kReportExisting) != 0) {
        String existing;
        for (const ObjectLoader &loader : mObjectLoaders) {
            if (loader.mLoad == 0) {
                existing.Printf("  %s\n", loader.mObject->mName.c_str());
            }
        }
        if (existing.mLength != 0) {
            DebugNotify("These objects in %s\nwon't be loaded because they\nalready exist:\n%s",
                        mFile.c_str(),
                        existing.c_str());
        }
    }
    mStream->ReadEndian(&mRemaining, sizeof(mRemaining));
    FilePath::SetRoot(mDir.c_str());
    mDir = FilePath::sRoot.c_str();
    mCurLoader = mObjectLoaders.begin();
    if (mVersion < kFirstTextureListVersion) {
        mState = &RndLoader::LoadObjects;
    } else {
        mState = &RndLoader::LoadResources;
    }
    (this->*mState)(fDeadlineMs);
}

void RndLoader::LoadResources(float fDeadlineMs) {
    if ((mFlags & kAsync) != 0) {
        if (mDir != FilePath::sRoot.c_str()) {
            FilePath::SetRoot(mDir.c_str());
        }
        int nSlot = 0;
        int nIdle = 0;
        if (!mTextures.empty()) {
            do {
                FilePath &texture = mTextures[nSlot];
                if (texture.mLength != 0 &&
                    TheManager.IsTextureLoaded(texture) == RndManager::kTextureLoading) {
                    nIdle = 0;
                } else if (mRemaining == 0) {
                    ++nIdle;
                    texture.Clear();
                } else {
                    nIdle = 0;
                    char szPath[kPathSize];
                    mStream->ReadString(szPath, sizeof(szPath));
                    texture.Set(szPath);
                    --mRemaining;
                    if (mVersion >= kFirstTextureOwnerVersion) {
                        while (mCurLoader->mObject->ClassName() != RndTex::sClassName &&
                               mCurLoader->mObject->ClassName() != RndMovie::sClassName) {
                            ++mCurLoader;
                        }
                        const ObjectLoader &owner = *mCurLoader++;
                        if (owner.mLoad != 1) {
                            // Yes, the binary fills the same slot again without a deadline test.
                            texture.Clear();
                            continue;
                        }
                    }
                    TheManager.AcquireTexture(texture);
                }
                if (++nSlot == static_cast<int>(mTextures.size())) {
                    nSlot = 0;
                }
                if (fDeadlineMs < SystemMs()) {
                    return;
                }
            } while (nIdle != static_cast<int>(mTextures.size()));
        }
    } else {
        while (mRemaining-- != 0) {
            char szPath[kPathSize];
            mStream->ReadString(szPath, sizeof(szPath));
        }
    }
    mCurLoader = mObjectLoaders.begin();
    mState = &RndLoader::LoadObjects;
    (this->*mState)(fDeadlineMs);
}

void RndLoader::LoadObjects(float fDeadlineMs) {
    if (mDir != FilePath::sRoot.c_str()) {
        FilePath::SetRoot(mDir.c_str());
    }
    while (mCurLoader != mObjectLoaders.end()) {
        if (!mStream->Eof()) {
            mCurLoader->Load(*mStream, mVersion);
            ++mCurLoader;
        }
        if (fDeadlineMs < SystemMs()) {
            return;
        }
    }
    mCurLoader = mObjectLoaders.begin();
    mState = &RndLoader::DeleteObjects;
    (this->*mState)(fDeadlineMs);
}

void RndLoader::DeleteObjects(float fDeadlineMs) {
    while (mCurLoader != mObjectLoaders.end()) {
        mCurLoader->Release();
        ++mCurLoader;
        if (fDeadlineMs < SystemMs()) {
            return;
        }
    }
    Finish();
}

void RndLoader::DoneLoading([[maybe_unused]] float fDeadlineMs) {
}

void RndLoader::Finish() {
    if (IsLoaded()) {
        return;
    }
    SetStringReadsText(0);
    mObjectLoaders.clear();
    if (mOwnsStream != 0) {
        delete mStream;
        mStream = nullptr;
    }
    if (mState != &RndLoader::DeleteObjects) {
        TheManager.ClearResources();
    }
    mState = &RndLoader::DoneLoading;
}
