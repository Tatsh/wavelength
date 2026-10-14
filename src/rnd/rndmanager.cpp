#include "rnd/rndmanager.h"

#include <cstring>
#include <strings.h>

#include "os/chunkstream.h"
#include "os/debug.h"
#include "os/filepath.h"
#include "os/filestream.h"
#include "os/fileutil.h"
#include "os/loadfile.h"
#include "os/mem.h"
#include "os/memstream.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndbitmap.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmovie.h"
#include "rnd/rndrenderer.h"
#include "rnd/rndtex.h"
#include "rnd/rndtransformable.h"

namespace {

// The bits per pixel of a bitmap that ResourcePath() widens to 32.
constexpr int kPackedBpp = 24;
constexpr int kWideBpp = 32;

// The first mipmap level ResourcePath() looks for.
constexpr int kFirstMip = 1;

// The longest name of a cached resource.
constexpr int kResourcePathSize = 256;

// The files beside a bitmap that make its cached copy stale when newer.
// NTSC-U/C: 0x003b0a08
const char *sBitmapParts[] = {"a", "m1", "m2", "m3", "m4", "m5", nullptr};

// The name ResourcePath() reports.
// NTSC-U/C: 0x0043c698
char sResourcePath[kResourcePathSize];

// Read the modification time of a file as the one 64-bit word the comparisons use.
unsigned long long ModifiedTime(const FileStat &stat) {
    unsigned long long nTime;
    memcpy(&nTime, stat.mModified, sizeof(nTime));
    return nTime;
}

// The budget that lets a synchronous load first finish every queued load.
constexpr float kFinishQueuedLoadsMs = 1000000.0f;

// File::New() modes.
constexpr int kFileModeRead = 1;
constexpr int kFileFlagsNone = 0;

// The alignment argument of PoolMemAlloc(), here the default.
constexpr int kDefaultAlignment = 0;

// The marker Write() places after each object record.
constexpr unsigned kRecordEndMarker = 0xdeaddead;

// The classes SortObjects() moves to the front in turn, each ahead of the previous. The last,
// the textures, are also sorted by file.
const char *const kSortedClasses[] = {"Blur", "Mesh", "Text", "Font", "Mat", "TransAnim", "Tex"};
constexpr int kTexSortIndex = 6;

// Move every object of a class, by its registered name, ahead of the object that was first in
// the list. A moved object is visited again with the objects after it.
// NTSC-U/C: 0x002388a0, PAL: 0x00241420
void GroupByClass(std::list<RndObject *> &objects, const char *pszClass) {
    const auto first = objects.begin();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->ClassName() == pszClass) {
            objects.splice(first, objects, it);
        }
    }
}

} // namespace

int RndManager::sRev = 10;

// The unit's static initialiser at NTSC-U/C: 0x0023a6a8, PAL: 0x00243228, its global constructor
// at NTSC-U/C: 0x0023a6e8, PAL: 0x00243268, and its global destructor at NTSC-U/C: 0x0023a708,
// PAL: 0x00243288, construct and destroy it.
RndManager TheManager;

RndManager::ResourceLoader::ResourceLoader(File *pFile, const char *pszTag) {
    mTag = pszTag;
    mFile = pFile;
    int nUncompressed = pFile->UncompressedSize();
    int nStored = mFile->Size();
    mSize = nStored < nUncompressed ? nUncompressed : nStored;
    mBuffer = static_cast<char *>(PoolMemAlloc(mSize, mTag, kDefaultAlignment));
    mCompressed = nUncompressed != 0 ? mBuffer + mSize - nStored : nullptr;
    mFile->ReadAsync(mCompressed != nullptr ? mCompressed : mBuffer, nStored);
    Poll();
    mRefs = 1;
}

RndManager::ResourceLoader::~ResourceLoader() {
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    delete mFile;
    mFile = nullptr;
}

bool RndManager::ResourceLoader::Poll() {
    if (mFile == nullptr) {
        return true;
    }
    int nBytes;
    if (!mFile->ReadDone(&nBytes)) {
        return false;
    }
    if (mCompressed != nullptr) {
        GzipDecompressRamToRam(mCompressed, mFile->Tell(), mBuffer);
    }
    delete mFile;
    mFile = nullptr;
    return true;
}

int RndManager::ResourceLoader::GetData(void **ppData, int *pnSize) {
    while (!Poll()) {
    }
    *pnSize = mSize;
    if (mRefs >= 2) {
        *ppData = PoolMemAlloc(mSize, mTag, kDefaultAlignment);
        std::memcpy(*ppData, mBuffer, mSize);
    } else if (mRefs == 1) {
        *ppData = mBuffer;
        mBuffer = nullptr;
    } else {
        DebugWarn("No references left on resource");
    }
    return --mRefs;
}

RndManager::RndManager() {
}

RndManager::~RndManager() {
}

void RndManager::PollLoaders(float fBudgetMs) {
    const float fDeadline = fBudgetMs + SystemMs();
    while (!mLoaders.empty()) {
        mLoaders.front()->Poll(fDeadline);
        if (mLoaders.empty()) {
            return;
        }
        if (mLoaders.front()->IsLoaded()) {
            mLoaders.pop_front();
        }
        if (fDeadline < SystemMs()) {
            return;
        }
    }
}

void RndManager::RemoveLoader(RndLoader *pLoader) {
    mLoaders.remove(pLoader);
}

RndObject *RndManager::Find(const char *pszName) {
    const auto it = mObjects.find(pszName);
    return it != mObjects.end() ? it->second : nullptr;
}

RndLoader *RndManager::AddLoader(const char *pszFile,
                                 int nFlags,
                                 RndLoader::Callback *pCallback,
                                 BinStream *pStream) {
    if ((nFlags & RndLoader::kAsync) == 0) {
        TheManager.PollLoaders(kFinishQueuedLoadsMs);
    }
    RndLoader *pLoader = new RndLoader(pszFile, nFlags, pCallback, pStream);
    mLoaders.push_back(pLoader);
    return pLoader;
}

void RndManager::LoadFile(const char *pszFile) {
    delete AddLoader(pszFile, 0, nullptr, nullptr);
}

RndObject *RndManager::Create(const char *pszClass, const char *pszName) {
    const auto it = mClasses.find(pszClass);
    if (it == mClasses.end()) {
        DebugNotify("Class %s is unregistered", pszClass);
        return nullptr;
    }
    if (mObjects.count(pszName) != 0) {
        DebugNotify("%s already exists", pszName);
        return nullptr;
    }
    return it->second(pszName);
}

void RndManager::Clone(RndObject *pSource,
                       const char *pszPrefix,
                       std::list<RndObject *> *pClones,
                       int nFlags,
                       int bRecurse,
                       int bLink) {
    RndObject *pClone =
        Create(pSource->ClassName(), FormatString("%s%s", pszPrefix, pSource->mName.c_str()));
    if (pClone == nullptr) {
        return;
    }
    pClone->Copy(pSource, nFlags);
    pClones->push_back(pClone);
    if (bRecurse == 0 && bLink == 0) {
        return;
    }

    RndAnimatable *pAnim = pSource != nullptr ? dynamic_cast<RndAnimatable *>(pSource) : nullptr;
    RndCollideable *pCollide =
        pSource != nullptr ? dynamic_cast<RndCollideable *>(pSource) : nullptr;
    RndDrawable *pDraw = pSource != nullptr ? dynamic_cast<RndDrawable *>(pSource) : nullptr;
    RndTransformable *pTrans =
        pSource != nullptr ? dynamic_cast<RndTransformable *>(pSource) : nullptr;

    if (bRecurse != 0) {
        std::list<RndObject *> children;
        RndAnimatable::CollectChildren(children, pAnim);
        RndCollideable::CollectChildren(children, pCollide);
        RndDrawable::CollectChildren(children, pDraw);
        RndTransformable::CollectChildren(children, pTrans);
        children.sort();
        children.unique();
        for (RndObject *pChild : children) {
            Clone(pChild, pszPrefix, pClones, nFlags, 0, 0);
        }

        children.push_front(pSource);
        auto itClone = pClones->begin();
        for (auto itChild = children.begin(); itChild != children.end(); ++itChild, ++itClone) {
            for (auto it = pClones->begin(); it != pClones->end(); ++it) {
                if (it != itClone) {
                    (*it)->Replace(*itChild, *itClone);
                }
            }
        }
    }

    if (bLink == 0) {
        return;
    }
    if (pTrans != nullptr) {
        RndTransformable *pParent = pTrans->Parent();
        if (pParent != nullptr) {
            pParent->AddTrans(pClone != nullptr ? dynamic_cast<RndTransformable *>(pClone) :
                                                  nullptr);
        }
    }
    if (pCollide != nullptr) {
        RndCollideable *pParent = pCollide->Parent();
        if (pParent != nullptr) {
            pParent->AddCollide(pClone != nullptr ? dynamic_cast<RndCollideable *>(pClone) :
                                                    nullptr);
        }
    }
    if (pDraw != nullptr) {
        RndDrawable *pParent = pDraw->Parent();
        if (pParent != nullptr) {
            pParent->AddDraw(pClone != nullptr ? dynamic_cast<RndDrawable *>(pClone) : nullptr,
                             nullptr);
        }
    }
    if (pAnim != nullptr) {
        RndAnimatable *pParent = pAnim->Parent();
        if (pParent != nullptr) {
            pParent->AddAnim(pClone != nullptr ? dynamic_cast<RndAnimatable *>(pClone) : nullptr);
        }
    }
}

void RndManager::SortObjects(std::list<RndObject *> &objects) {
    for (int i = 0; i < kTexSortIndex; ++i) {
        // Yes, the binary takes the key of end() for an unregistered class.
        GroupByClass(objects, mClasses.find(kSortedClasses[i])->first);
    }

    const char *pszTex = mClasses.find(kSortedClasses[kTexSortIndex])->first;
    auto sorted = objects.begin();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->ClassName() != pszTex) {
            continue;
        }
        RndTex *pTex = *it != nullptr ? dynamic_cast<RndTex *>(*it) : nullptr;
        const char *pszFile = pTex->mFile.c_str();
        auto pos = objects.begin();
        for (; pos != sorted; ++pos) {
            RndTex *pOther = *pos != nullptr ? dynamic_cast<RndTex *>(*pos) : nullptr;
            if (strcmp(pszFile, pOther->mFile.c_str()) < 0) {
                break;
            }
        }
        if (it == sorted) {
            ++sorted;
        }
        objects.splice(pos, objects, it);
    }
}

void RndManager::Write(BinStream &stream) {
    std::list<RndObject *> objects;
    for (const auto &entry : mObjects) {
        if (entry.second->mInternal == 0) {
            objects.push_back(entry.second);
        }
    }
    SortObjects(objects);

    int nValue = sRev;
    stream.WriteEndian(&nValue, sizeof(nValue));
    nValue = static_cast<int>(objects.size());
    stream.WriteEndian(&nValue, sizeof(nValue));
    for (RndObject *pObject : objects) {
        stream.WriteString(pObject->ClassName()).WriteString(pObject->mName.c_str());
    }

    std::vector<FilePath> files;
    for (RndObject *pObject : objects) {
        RndTex *pTex = pObject != nullptr ? dynamic_cast<RndTex *>(pObject) : nullptr;
        if (pTex != nullptr) {
            files.push_back(pTex->mFile);
            continue;
        }
        RndMovie *pMovie = pObject != nullptr ? dynamic_cast<RndMovie *>(pObject) : nullptr;
        if (pMovie != nullptr) {
            files.push_back(pMovie->mFile);
        }
    }
    EndChunk(stream << files);

    for (RndObject *pObject : objects) {
        pObject->Save(stream);
        const unsigned nMarker = kRecordEndMarker;
        stream.WriteEndian(&nMarker, sizeof(nMarker));
        EndChunk(stream);
    }
}

RndObject *RndManager::NewTunnel(const char *pszName) {
    DebugNotify("Tunnel %s loaded as view", pszName);
    return TheManager.Create("View", pszName);
}

const char *RndManager::ResourceTag(const char *pszFile) {
    const char *pszExt = FileGetExt(pszFile);
    if (strcasecmp(pszExt, "bmp") == 0) {
        return "Resource_bmp";
    }
    if (strcasecmp(pszExt, "ipu") == 0) {
        return "Resource_ipu";
    }
    return "Resource_other";
}

const char *RndManager::ResourcePath(const char *pszFile) {
    if (strcasecmp(FileGetExt(pszFile), "bmp") != 0) {
        return pszFile;
    }
    GzipFileName(pszFile, 1, sResourcePath);
    if (UsingCD()) {
        return sResourcePath;
    }
    FileStat stat;
    if (FileGetStat(pszFile, &stat) < 0) {
        return nullptr;
    }
    unsigned long long nNewest = ModifiedTime(stat);
    const char *pszPath = FileGetPath(pszFile);
    const char *pszBase = FileGetBaseName(pszFile);
    FileStat cached;
    for (const char **ppszPart = sBitmapParts; *ppszPart != nullptr; ++ppszPart) {
        if (FileGetStat(FormatString("%s/%s_%s.bmp", pszPath, pszBase, *ppszPart), &stat) >= 0) {
            const unsigned long long nTime = ModifiedTime(stat);
            nNewest = nNewest < nTime ? nTime : nNewest;
        }
    }
    if (FileGetStat(sResourcePath, &cached) >= 0 && !(ModifiedTime(cached) < nNewest)) {
        return sResourcePath;
    }
    DebugPrint("Caching %s\n", pszFile);
    RndBitmap bitmap;
    RndBitmap converted;
    if (!bitmap.LoadBmp(pszFile)) {
        return nullptr;
    }
    bitmap.LoadAlpha(FormatString("%s/%s_a.bmp", pszPath, pszBase));
    for (int nMip = kFirstMip;
         bitmap.LoadMip(FormatString("%s/%s_m%d.bmp", pszPath, pszBase, nMip));
         ++nMip) {
    }
    converted.Create(bitmap,
                     bitmap.mBpp == kPackedBpp ? kWideBpp : bitmap.mBpp,
                     RndBitmap::kOrderRGBA | RndBitmap::kOrderGs);
    MemStream stream(true);
    converted.Save(stream);
    const int nSize =
        GzipCompressRamToRam(stream.mBuffer.data(), stream.mTell, stream.mBuffer.data());
    FileMkDir(FileGetPath(sResourcePath));
    FileStream file(sResourcePath, true, true, kFileFlagsNone);
    file.Write(stream.mBuffer.data(), nSize);
    return sResourcePath;
}

bool RndManager::AcquireTexture(const FilePath &path) {
    if (path.mLength == 0 || !TheRnd->SupportsTextures()) {
        return true;
    }
    const auto it = mResources.find(path);
    if (it != mResources.end()) {
        ++it->second->mRefs;
        return true;
    }
    File *pFile = File::New(ResourcePath(path.c_str()), kFileModeRead, kFileFlagsNone);
    if (pFile == nullptr) {
        return false;
    }
    ResourceLoader *&pLoader = mResources[path];
    pLoader = new ResourceLoader(pFile, ResourceTag(path.c_str()));
    return true;
}

void RndManager::GetResource(const FilePath &path, void **ppData, int *pnSize) {
    auto it = mResources.find(path);
    *ppData = nullptr;
    if (it == mResources.end()) {
        if (!AcquireTexture(path)) {
            DebugNotify("Couldn't load %s", path.c_str());
            return;
        }
        it = mResources.find(path);
    }
    if (it != mResources.end() && it->second->GetData(ppData, pnSize) == 0) {
        delete it->second;
        mResources.erase(it);
    }
}

int RndManager::IsTextureLoaded(const FilePath &path) {
    const auto it = mResources.find(path);
    if (it == mResources.end()) {
        return kTextureUnknown;
    }
    return it->second->Poll() ? kTextureLoaded : kTextureLoading;
}

void RndManager::DeleteLoadedObjects() {
    for (;;) {
        auto it = mObjects.begin();
        while (it != mObjects.end() && it->second->mInternal != 0) {
            ++it;
        }
        if (it == mObjects.end()) {
            break;
        }
        // Yes, the binary loops forever on a null entry. Only the destructor erases an entry.
        delete it->second;
    }
    ClearResources();
}

void RndManager::ReleaseTexture(const FilePath &path) {
    const auto it = mResources.find(path);
    if (it != mResources.end()) {
        delete it->second;
        mResources.erase(it);
    }
}

void RndManager::ClearResources() {
    for (const auto &entry : mResources) {
        delete entry.second;
    }
    mResources.clear();
}
