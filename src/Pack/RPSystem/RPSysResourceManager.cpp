#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPSysResourceManager.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
#include <Pack/RPParty/RPPartyCommon.h>
#include <Pack/RPSports/RPSportsCommon.h>
#include <Pack/RPSystem.h>

#include <nw4r/ut.h>

#include <cstring>

void fake_function(...);

RPSysResourceManager* RPSysResourceManager::spInstance = NULL;

// Wii Play: DestroyInstance is not in the binary (unused, dead-stripped)
void RPSysResourceManager::CreateInstance() {
    spInstance = new RPSysResourceManager();
}

/**
 * @brief List of scenes whose resources should be cached
 */
const RPSysSceneCreator::ESceneID RPSysResourceManager::CACHED_SCENES[] = {
    RPSysSceneCreator::ESceneID_RPSysPlayerSelectScene,
    RPSysSceneCreator::ESceneID_RPSysNunchukCheckScene,

#if defined(PACK_SPORTS)
    RPSysSceneCreator::ESceneID_RPBsbScene,
    RPSysSceneCreator::ESceneID_RPBowScene,
    RPSysSceneCreator::ESceneID_RPGolScene,
    RPSysSceneCreator::ESceneID_RPTnsScene,
    RPSysSceneCreator::ESceneID_RPBoxScene,
    RPSysSceneCreator::ESceneID_RPSportsPackTitleScene,
    RPSysSceneCreator::ESceneID_RPSportsMainScene,
    RPSysSceneCreator::ESceneID_RPSportsTrainingMenuScene,
    RPSysSceneCreator::ESceneID_RPSportsPhysicalMenuScene,
    RPSysSceneCreator::ESceneID_RPSportsPhysicalPreviewScene,
    RPSysSceneCreator::ESceneID_RPSportsPhysicalResultScene,
    RPSysSceneCreator::ESceneID_RPGolCourseSelectScene,
#elif defined(PACK_PARTY)
    RPSysSceneCreator::ESceneID_RPFshScene,
    RPSysSceneCreator::ESceneID_RPHkyScene,
    RPSysSceneCreator::ESceneID_RPDucScene,
    RPSysSceneCreator::ESceneID_RPPnpScene,
    RPSysSceneCreator::ESceneID_RPBilScene,
    RPSysSceneCreator::ESceneID_RPCowScene,
    RPSysSceneCreator::ESceneID_RPWlyScene,
    RPSysSceneCreator::ESceneID_RPTnkScene,
    RPSysSceneCreator::ESceneID_RPBomScene,
    RPSysSceneCreator::ESceneID_RPPartyPackTitleScene,
    // Wii Play: no RPPartyRootScene
    RPSysSceneCreator::ESceneID_RPPartyMainScene,
#endif
};

/**
 * @brief Static directory names for each pack
 */
// clang-format off
const char* RPSysResourceManager::STATIC_DIR_NAMES[RPSysSceneCreator::EPackID_Max] = {
    "SportsStatic/", // EPackID_SportsPack
    "PartyStatic/",  // EPackID_PartyPack
    "HealthStatic/", // EPackID_HealthPack
    "MusicStatic/",  // EPackID_MusicPack
};
// clang-format on

/**
 * @brief Absolute path to the kokeshi asset archive
 */
const char* RPSysResourceManager::KOKESHI_ARCHIVE_PATH =
    "Common/Kokeshi/common.carc";

/**
 * @brief Constructor
 */
// Wii Play: inlined everywhere
inline RPSysResourceManager::RPSysResourceManager() {
    NW4R_UT_LIST_INIT(mResourceLists[EList_FileCache], RPSysFile);
    NW4R_UT_LIST_INIT(mResourceLists[EList_FileList], RPSysFile);

    mpPathWork = new char[RPSysFile::PATH_MAX];
}

/**
 * @brief Destructor
 */
RPSysResourceManager::~RPSysResourceManager() {}

/**
 * @brief Gets the path to the common asset folder for the specified scene
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 * @param id Scene ID (defaults to the current scene)
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
void RPSysResourceManager::GetGameCommonPath(char* pBuffer, u32 /* maxlen */,
                                             s32 id) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);

    std::strcpy(pBuffer, "Common/");
    std::strcat(pBuffer, pSceneCreator->getResDirName(id));
}

/**
 * @brief Loads the common asset archive for the specified scene
 *
 * @param id Scene ID (defaults to the current scene)
 * @param pHeap Heap to use for allocations (defaults to the current heap)
 */
EGG::Archive* RPSysResourceManager::LoadGameCommonArchive(s32 id,
                                                          EGG::Heap* pHeap) {

    instance()->GetGameCommonPath(instance()->mpPathWork, RPSysFile::PATH_MAX,
                                  id);

    std::strcat(instance()->mpPathWork, "common.carc");

    if (pHeap == NULL) {
        pHeap = EGG::Heap::getCurrentHeap();
    }

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pHeap);

    return EGG::Archive::mount(pArcBinary, pHeap);
}

/**
 * @brief Gets the path to the local asset folder for the specified scene
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 * @param id Scene ID (defaults to the current scene)
 */
// Wii Play: inlined everywhere
inline void RPSysResourceManager::GetGameLocalPath(char* pBuffer, u32 /* maxlen */,
                                            s32 id) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    pBuffer[0] = '\0';
    pProjectLocal->appendLocalDirectory(pBuffer);

    std::strcat(pBuffer, pSceneCreator->getResDirName(id));
}

/**
 * @brief Loads the local asset archive for the specified scene
 *
 * @param id Scene ID (defaults to the current scene)
 * @param pHeap Heap to use for allocations (defaults to the current heap)
 */
EGG::Archive* RPSysResourceManager::LoadGameLocalArchive(s32 id,
                                                         EGG::Heap* pHeap) {
    instance()->GetGameLocalPath(instance()->mpPathWork, RPSysFile::PATH_MAX,
                                 id);

    std::strcat(instance()->mpPathWork, "local.carc");

    if (pHeap == NULL) {
        pHeap = EGG::Heap::getCurrentHeap();
    }

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pHeap);

    return EGG::Archive::mount(pArcBinary, pHeap);
}

/**
 * @brief Gets the path to the stage asset folder for the specified scene
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 * @param id Scene ID (defaults to the current scene)
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
void RPSysResourceManager::GetGameStagePath(char* pBuffer, u32 /* maxlen */,
                                            s32 id) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);

    std::strcpy(pBuffer, "Stage/");
    std::strcat(pBuffer, pSceneCreator->getResDirName(id));
}

/**
 * @brief Loads the specified archive from the current scene's stage assets
 *
 * @param pName Archive name
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
EGG::Archive* RPSysResourceManager::LoadGameStageArchive(const char* pName) {
    instance()->GetGameStagePath(instance()->mpPathWork, RPSysFile::PATH_MAX);
    std::strcat(instance()->mpPathWork, pName);

    EGG::Heap* pHeap = EGG::Heap::getCurrentHeap();

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pHeap);

    return EGG::Archive::mount(pArcBinary, pHeap);
}

/**
 * @brief Gets the specified file from the provided archive
 *
 * @param pArchive Archive containing the file
 * @param pName File name
 * @param[out] pSize Size of the specified file
 */
void* RPSysResourceManager::GetFileFromArchive(EGG::Archive* pArchive,
                                               const char* pName, u32* pSize) {

    EGG::Archive::FileInfo fileInfo;
    void* pFile = pArchive->getFile(pName, &fileInfo);

    if (pSize != NULL) {
        *pSize = fileInfo.getLength();
    }

    return pFile;
}

/**
 * @brief Gets the specified file from the message archive
 *
 * @param pName Message file name
 */
void* RPSysResourceManager::GetMessageResource(const char* pName) {
    return GetFileFromArchive(instance()->GetMessageArchive(), pName);
}

/**
 * @brief Tests whether a file exists at the specified path
 *
 * @param pPath Path to examine
 */

bool RPSysResourceManager::IsExist(const char* pPath) {
    EGG::DvdFile f;
#if defined(VERSION_RSPE01_00)
    return f.open(pPath);
#elif (defined(VERSION_RSPE01_01) || defined(VERSION_RHAE01))
    return f.open(pPath, GetMultiHandle());
#endif
}

/**
 * @brief Removes a file from the list of decompressed files
 * @note RPSysFile automatically removes itself when it is destroyed.
 *
 * @param pFile File to remove
 */
void RPSysResourceManager::RemoveFromFileList(RPSysFile* pFile) {
    nw4r::ut::List_Remove(&mResourceLists[EList_FileList], pFile);
}

/**
 * @brief Loads all archives and other assets with static lifetime
 */
void RPSysResourceManager::LoadStaticArchives() {
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);
    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();

    mpStaticCommonArchive =
        LoadStaticCommonArchive(STATIC_DIR_NAMES[pProjectLocal->getPack()]);

    mpStaticLocalArchive =
        LoadStaticLocalArchive(STATIC_DIR_NAMES[pProjectLocal->getPack()]);

#if defined(PACK_SPORTS)
    mpAppMiiManager = new (pResHeap) RPSportsAppMiiManager(pResHeap);
    mpAppMiiManager->LoadResource();
#else
    // Wii Play: the slot is cleared
    unk38 = NULL;
#endif

    LoadMessageArchive();
    LoadFontArchive();

    RPSysKokeshiManager* pKokeshiManager = RP_GET_INSTANCE(RPSysKokeshiManager);
#if defined(PACK_SPORTS)
    pKokeshiManager->SetBodyManager(new (pResHeap) RPSportsBodyManager());
#elif defined(PACK_PARTY)
    pKokeshiManager->SetBodyManager(new (pResHeap) RPPartyBodyManager());
#endif

    RP_GET_INSTANCE(RPSysFontManager)->LoadResFonts();
}

/**
 * @brief Loads the specified file and adds it to the resource cache
 * @details If the file has already been cached, this function does nothing.
 *
 * @param pPath File path
 * @param list List that the file should be moved to
 * @param pHeap Heap to use for allocations
 */
// Wii Play: inlined everywhere
inline void RPSysResourceManager::LoadCachedFile(const char* pPath, EList list,
                                          EGG::Heap* pHeap) {
    RPSysFile* pFile = FindFile(pPath, EList_FileCache);
    if (pFile != NULL) {
        return;
    }

    s32 fileSize = 0;
    u8* pFileData =
        LoadFromDVD(pPath, pHeap, EGG::DvdRipper::ALLOC_DIR_TAIL, &fileSize);

    DCFlushRange(pFileData, fileSize);
    MakeFile(pPath, list, fileSize, pFileData, pHeap);
}

/**
 * @brief Loads all archives designated for cached access
 */
void RPSysResourceManager::LoadCacheArchives() {
    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();

    for (int i = 0; i < static_cast<int>(ARRAY_SIZE(CACHED_SCENES)); i++) {
        GetGameCommonPath(instance()->mpPathWork, RPSysFile::PATH_MAX,
                          CACHED_SCENES[i]);

        std::strcat(instance()->mpPathWork, "common.carc");
        LoadCachedFile(instance()->mpPathWork, EList_FileCache, pResHeap);

        // Local archives in Wii Play are small enough to also cache
#if defined(PACK_PARTY)
        GetGameLocalPath(instance()->mpPathWork, RPSysFile::PATH_MAX,
                         CACHED_SCENES[i]);

        std::strcat(instance()->mpPathWork, "local.carc");
        LoadCachedFile(instance()->mpPathWork, EList_FileCache, pResHeap);
#endif
    }
}

/**
 * @brief Loads the archive containing all kokeshi assets
 */
void RPSysResourceManager::LoadKokeshiArchive() {
    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();

    std::strcpy(instance()->mpPathWork, KOKESHI_ARCHIVE_PATH);

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pResHeap);

    mpKokeshiArchive = EGG::Archive::mount(pArcBinary, pResHeap);
}

/**
 * @brief Gets the path to the common sound folder
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 */
void RPSysResourceManager::GetStaticSoundCommonPath(char* pBuffer,
                                                    u32 /* maxlen */) {
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    std::strcpy(pBuffer, "Sound/");
    std::strcat(pBuffer, STATIC_DIR_NAMES[pProjectLocal->getPack()]);
}

/**
 * @brief Gets the path to the local sound folder
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 */
void RPSysResourceManager::GetStaticSoundLocalPath(char* pBuffer,
                                                   u32 /* maxlen */) {
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    std::strcpy(pBuffer, "Sound/");
    std::strcat(pBuffer, STATIC_DIR_NAMES[pProjectLocal->getPack()]);
    pProjectLocal->appendLocalDirectory(pBuffer);
}

/**
 * @brief Gets the path to the common sound folder for the specified scene
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 * @param id Scene ID (defaults to the current scene)
 */
void RPSysResourceManager::GetGameSoundCommonPath(char* pBuffer,
                                                  u32 /* maxlen */, s32 id) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);

    std::strcpy(pBuffer, "Sound/");
    std::strcat(pBuffer, pSceneCreator->getResDirName(id));
}

/**
 * @brief Gets the path to the local sound folder for the specified scene
 *
 * @param[out] pBuffer Path output buffer
 * @param maxlen Output buffer size
 * @param id Scene ID (defaults to the current scene)
 */
void RPSysResourceManager::GetGameSoundLocalPath(char* pBuffer,
                                                 u32 /* maxlen */, s32 id) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    std::strcpy(pBuffer, "Sound/");
    std::strcat(pBuffer, pSceneCreator->getResDirName(id));
    pProjectLocal->appendLocalDirectory(pBuffer);
}

/**
 * @brief Loads the local archive in the specified static asset directory
 *
 * @param pStaticDir Static asset directory
 */
EGG::Archive*
RPSysResourceManager::LoadStaticLocalArchive(const char* pStaticDir) {
    RPSysSceneCreator* pSceneCreator = RP_GET_INSTANCE(RPSysSceneCreator);
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    mpPathWork[0] = '\0';
    pProjectLocal->appendLocalDirectory(mpPathWork);

    std::strcat(mpPathWork, pStaticDir);
    std::strcat(mpPathWork, "local.carc");

    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pResHeap);

    return EGG::Archive::mount(pArcBinary, pResHeap);
}

/**
 * @brief Loads the common archive in the specified static asset directory
 *
 * @param pStaticDir Static asset directory
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
EGG::Archive*
RPSysResourceManager::LoadStaticCommonArchive(const char* pStaticDir) {
    std::strcpy(mpPathWork, "Common/");
    std::strcat(mpPathWork, pStaticDir);
    std::strcat(mpPathWork, "common.carc");

    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();

    void* pArcBinary = LoadCompressed(mpPathWork, pResHeap);
    return EGG::Archive::mount(pArcBinary, pResHeap);
}

// Wii Play: string pool order (local and unmangled, so the linker strips it)
extern "C" {
static void RPSysResourceManager_cpp_strings_0(void) {
    fake_function("effect.carc", "local.carc");
}
}

/**
 * @brief Loads the archive containing all message data
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
void RPSysResourceManager::LoadMessageArchive() {
    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    instance()->mpPathWork[0] = '\0';
    pProjectLocal->appendLocalDirectory(instance()->mpPathWork);
    std::strcat(instance()->mpPathWork, "Message/message.carc");

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pResHeap);

    mpMessageArchive = EGG::Archive::mount(pArcBinary, pResHeap);
}

/**
 * @brief Loads the archive containing all font data
 */
// Wii Play: inlined everywhere; the out-of-line copy (its strings are in the pool) is dead-stripped
void RPSysResourceManager::LoadFontArchive() {
    EGG::Heap* pResHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();
    RPSysProjectLocal* pProjectLocal = RP_GET_INSTANCE(RPSysProjectLocal);

    instance()->mpPathWork[0] = '\0';
    pProjectLocal->appendLocalDirectory(instance()->mpPathWork);
    std::strcat(instance()->mpPathWork, "Font/font.carc");

    void* pArcBinary =
        instance()->LoadCompressed(instance()->mpPathWork, pResHeap);

    mpFontArchive = EGG::Archive::mount(pArcBinary, pResHeap);
}

// Wii Play: the FileList lookup in LoadCompressed goes through a list pointer
// (the original recomputes this+0x10 at each List_GetNext instead of keeping it)
static inline RPSysFile* RPSysResourceManager_FindInList(nw4r::ut::List* pList,
                                                       const char* pPath) {
    RP_NW4R_LIST_FOREACH (RPSysFile, it, *pList) {
        if (std::strcmp(pPath, it->GetPath()) == 0) {
            return it;
        }
    }

    return NULL;
}

/**
 * @brief Loads the specified file from the DVD, decompressing its contents
 * if applicable
 *
 * @param pPath File path
 * @param pHeap Heap to use for allocations
 * @return Raw file contents
 */
u8* RPSysResourceManager::LoadCompressed(const char* pPath, EGG::Heap* pHeap) {
    // See if the file has already been decompressed
    RPSysFile* pFile =
        RPSysResourceManager_FindInList(&mResourceLists[EList_FileList], pPath);
    // NONMATCHING-DEBT: RPSysFile keeps its data as const u8* (its constructor takes const void*,
    // mangled PCv), while this returns the mutable buffer; both casts below stay.
    if (pFile != NULL) {
        return const_cast<u8*>(pFile->GetData());
    }

    s32 fileSize = 0;
    pFile = FindFile(pPath, EList_FileCache);

    // If the file is waiting in the cache, decompress it
    if (pFile != NULL) {
        u8* pFileData = const_cast<u8*>(pFile->GetData());
        fileSize = pFile->GetSize();

        s32 expandSize = 0;
        u8* pExpandData =
            ExpandCompressed(pFileData, fileSize, &expandSize, pHeap);

        DCFlushRange(pExpandData, expandSize);

        MakeFile(pPath, EList_FileList, expandSize, pExpandData, pHeap);
        return pExpandData;
    }
    // Cache miss, need to load AND decompress the file
    else {
        u8* pFileData = LoadFromDVD(pPath, pHeap,
                                    EGG::DvdRipper::ALLOC_DIR_HEAD, &fileSize);

        s32 expandSize = 0;
        u8* pExpandData =
            ExpandCompressed(pFileData, fileSize, &expandSize, pHeap);

        DCFlushRange(pExpandData, expandSize);

        // Original compressed data is temporary memory
        delete pFileData;

        MakeFile(pPath, EList_FileList, expandSize, pExpandData, pHeap);
        return pExpandData;
    }
}

/**
 * @brief Loads the specified file from the DVD
 *
 * @param pPath Path to the file
 * @param pHeap Heap to use for allocations
 * @param allocDir Direction of heap allocations
 * @param[out] pSize Size of the specified file
 */
u8* RPSysResourceManager::LoadFromDVD(const char* pPath, EGG::Heap* pHeap,
                                      EGG::DvdRipper::EAllocDirection allocDir,
                                      s32* pSize) {
    EGG::DvdFile f;

#if defined(VERSION_RSPE01_00)
    if (!f.open(pPath)) {
        return NULL;
    }
#elif (defined(VERSION_RSPE01_01) || defined(VERSION_RHAE01))
    if (!f.open(pPath, GetMultiHandle())) {
        return NULL;
    }
#endif

    s32 fileSize = ROUND_UP(f.getFileSize(), 32);

    if (pSize != NULL) {
        *pSize = fileSize;
    }

    int align = 32;
    if (allocDir != EGG::DvdRipper::ALLOC_DIR_TAIL) {
        align = -32;
    }

    u8* pBuffer = new (pHeap, align) u8[fileSize];

    if (pBuffer == NULL) {
        return NULL;
    }

    return EGG::DvdRipper::loadToMainRAM(&f, pBuffer, pHeap,
                                         EGG::DvdRipper::ALLOC_DIR_HEAD);
}

// Wii Play: string pool order (local and unmangled, so the linker strips it)
extern "C" {
static void RPSysResourceManager_cpp_strings_1(void) {
    fake_function("/", "Effect/", "Stage/", "", "Common/");
}
}

/**
 * @brief Searches the specified resource list for the provided filepath
 *
 * @param pPath File path
 * @param list Resource list
 */
// Wii Play: inlined everywhere (the out-of-line copy is dead-stripped)
RPSysFile* RPSysResourceManager::FindFile(const char* pPath, EList list) const {
    RP_NW4R_LIST_FOREACH (RPSysFile, it, mResourceLists[list]) {
        if (std::strcmp(pPath, it->GetPath()) == 0) {
            return it;
        }
    }

    return NULL;
}

/**
 * @brief Creates a new entry in the specified resource list
 *
 * @param pPath File path
 * @param list Resource list
 * @param size File size
 * @param pData File data
 * @param pHeap Heap to use for allocations
 */
// Wii Play: inlined everywhere
inline void RPSysResourceManager::MakeFile(const char* pPath, EList list, s32 size,
                                    const void* pData, EGG::Heap* pHeap) {

    RPSysFile* pFile = new (pHeap) RPSysFile(pPath, size, pData);
    nw4r::ut::List_Append(&mResourceLists[list], pFile);
}

/**
 * @brief Decompresses the contents of the specified data
 * @details If the data is not compressed, this function returns the
 * original file contents
 *
 * @param pData File data
 * @param size Data size
 * @param[out] pExpandSize The data size after decompression
 * @param pHeap Heap to use for allocations
 * @return Decompressed data
 */
// Wii Play: inlined everywhere
inline u8* RPSysResourceManager::ExpandCompressed(u8* pData, s32 size,
                                           s32* pExpandSize,
                                           EGG::Heap* pHeap) const {
    u8* pExpandData = NULL;

    if (EGG::Decomp::checkCompressed(pData) != EGG::Decomp::cCompress_None) {
        *pExpandSize = EGG::Decomp::getExpandSize(pData);
        pExpandData = new (pHeap, 32) u8[*pExpandSize];
        EGG::Decomp::decode(pData, pExpandData);
    } else {
        *pExpandSize = size;
        pExpandData = new (pHeap, 32) u8[*pExpandSize];
        std::memcpy(pExpandData, pData, *pExpandSize);
    }

    return pExpandData;
}
