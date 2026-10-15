#ifndef LIBMC_H
#define LIBMC_H

#include <sifrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Memory card access through the mcserv server on the IOP. */

/** Results the server reports. */
#define sceMcResSucceed 0        /*!< The command succeeded. */
#define sceMcResChangedCard -1   /*!< A different card was inserted since the last access. */
#define sceMcResNoFormat -2      /*!< The card is not formatted. */
#define sceMcResFullDevice -3    /*!< The card has no free space. */
#define sceMcResNoEntry -4       /*!< The file or directory does not exist. */
#define sceMcResDeniedPermit -5  /*!< The file attributes deny the access. */
#define sceMcResNotEmpty -6      /*!< The directory is not empty. */
#define sceMcResUpLimitHandle -7 /*!< Every file descriptor is in use. */
#define sceMcResFailReplace -8   /*!< The card could not replace a bad block. */

/** Card types sceMcGetInfo() reports. */
#define sceMcTypeNoCard 0 /*!< No card is inserted. */
#define sceMcTypePS1 1    /*!< A PlayStation card. */
#define sceMcTypePS2 2    /*!< A PlayStation 2 card. */
#define sceMcTypePDA 3    /*!< A PocketStation. */

/** Mode bits of sceMcOpen(). */
#define sceMcFileAttrReadable 0x0001  /*!< Open for reading. */
#define sceMcFileAttrWriteable 0x0002 /*!< Open for writing. */
#define sceMcFileCreateDir 0x0040     /*!< Create a directory. */
#define sceMcFileCreateFile 0x0200    /*!< Create the file when it does not exist. */

/** Modes of sceMcSync(). */
#define sceMcWait 0   /*!< Wait for the command to finish. */
#define sceMcNoWait 1 /*!< Report the state of the command at once. */

/** Values sceMcSync() returns. */
#define sceMcExecIdle -1  /*!< No command was issued. */
#define sceMcExecRun 0    /*!< The command is still running. */
#define sceMcExecFinish 1 /*!< The command finished. */

/** Command numbers sceMcSync() reports. */
#define sceMcFuncNoCardInfo 1  /*!< sceMcGetInfo(). */
#define sceMcFuncNoOpen 2      /*!< sceMcOpen(). */
#define sceMcFuncNoClose 3     /*!< sceMcClose(). */
#define sceMcFuncNoSeek 4      /*!< sceMcSeek(). */
#define sceMcFuncNoRead 5      /*!< sceMcRead(). */
#define sceMcFuncNoWrite 6     /*!< sceMcWrite(). */
#define sceMcFuncNoFlush 10    /*!< Writing the cached blocks of a file. */
#define sceMcFuncNoMkdir 11    /*!< sceMcMkdir(). */
#define sceMcFuncNoChDir 12    /*!< Changing the current directory. */
#define sceMcFuncNoGetDir 13   /*!< sceMcGetDir(). */
#define sceMcFuncNoFileInfo 14 /*!< Changing the times or the attributes of an entry. */
#define sceMcFuncNoDelete 15   /*!< sceMcDelete(). */
#define sceMcFuncNoFormat 16   /*!< sceMcFormat(). */
#define sceMcFuncNoUnformat 17 /*!< sceMcUnformat(). */
#define sceMcFuncNoEntSpace 18 /*!< Measuring the space the entries of a directory could use. */
#define sceMcFuncNoRename 19   /*!< Renaming an entry. */
#define sceMcFuncNoChgPrior 20 /*!< Changing the priority of the server thread. */

/** A time stamp of a directory entry, in the card's local time. */
typedef struct {
    unsigned char Resv2; /*!< Reserved. */
    unsigned char Sec;   /*!< Seconds. */
    unsigned char Min;   /*!< Minutes. */
    unsigned char Hour;  /*!< Hours. */
    unsigned char Day;   /*!< Day of the month. */
    unsigned char Month; /*!< Month. */
    unsigned short Year; /*!< Year. */
} sceMcStDateTime;

/** One directory entry, as sceMcGetDir() lists it. */
typedef struct {
    sceMcStDateTime _Create;     /*!< Creation time. */
    sceMcStDateTime _Modify;     /*!< Modification time. */
    unsigned int FileSizeByte;   /*!< File size in bytes. */
    unsigned short AttrFile;     /*!< Attribute bits. */
    unsigned short Reserve1;     /*!< Reserved. */
    unsigned int Reserve2;       /*!< Reserved. */
    unsigned int PdaAplNo;       /*!< PocketStation application number. */
    unsigned char EntryName[32]; /*!< Entry name, terminated by a zero byte. */
} sceMcTblGetDir;

/** The icon.sys file the browser reads to show a save directory. */
typedef struct {
    unsigned char Head[4];       /*!< "PS2D". */
    unsigned short Reserv1;      /*!< Reserved. */
    unsigned short OffsLF;       /*!< Byte offset of the line break in TitleName. */
    unsigned int Reserv2;        /*!< Reserved. */
    unsigned int TransRate;      /*!< Background transparency. */
    int BgColor[4][4];           /*!< Colours of the four background corners. */
    float LightDir[3][4];        /*!< Directions of the three lights. */
    float LightColor[3][4];      /*!< Colours of the three lights. */
    float Ambient[4];            /*!< Ambient colour. */
    unsigned char TitleName[68]; /*!< Title in Shift JIS. */
    unsigned char FnameView[64]; /*!< Icon file shown while the entry is listed. */
    unsigned char FnameCopy[64]; /*!< Icon file shown while the entry is copied. */
    unsigned char FnameDel[64];  /*!< Icon file shown while the entry is deleted. */
    unsigned char Reserve3[512]; /*!< Reserved. */
} sceMcIconSys;

/**
 * The reply buffer every call receives the server's result in.
 *
 * The initialisation call also receives the two module versions.
 */
typedef struct {
    int nResult;         /*!< Result of the last command. */
    int nServerVersion;  /*!< mcserv version, received by sceMcInitLibrary(). */
    int nManagerVersion; /*!< mcman version, received by sceMcInitLibrary(). */
    int anReserved[12];  /*!< Not written by the library. */
    int nSemaId;         /*!< Not written by the library. */
} sceMcRpcResult;

/**
 * Bind the memory card server and check the module versions.
 *
 * Waits for a command in flight to finish first. The PAL build also creates the semaphore that the
 * library's polling delay waits on.
 *
 * @return The server's result, the RPC error less 100, -120 when mcserv is too old, or -121 when
 * mcman is too old.
 * @ghidraAddress NTSC-U/C: 0x00312f60
 * @ghidraAddress PAL: 0x0037f5f8
 */
int sceMcInitLibrary(void);

/**
 * Delete the semaphores sceMcInitLibrary() created.
 *
 * @return 1.
 * @ghidraAddress NTSC-U/C: 0x00313118
 * @ghidraAddress PAL: 0x0037f7c8
 */
int sceMcEnd(void);

/**
 * Start opening a file.
 *
 * @param nPort Port.
 * @param nSlot Slot of a multitap, or zero.
 * @param pszName Path of the file.
 * @param nMode sceMcFileAttrReadable, sceMcFileAttrWriteable, and sceMcFileCreateFile bits.
 * @return Zero once the command was sent, -100 when the library is not bound, -200 while another
 * command runs, -210 for an empty name, or the RPC error. sceMcSync() reports the descriptor.
 * @ghidraAddress NTSC-U/C: 0x00313160
 * @ghidraAddress PAL: 0x0037f820
 */
int sceMcOpen(int nPort, int nSlot, const char *pszName, int nMode);

/**
 * Start creating a directory.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pszName Path of the directory.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313288
 * @ghidraAddress PAL: 0x0037f948
 */
int sceMcMkdir(int nPort, int nSlot, const char *pszName);

/**
 * Start closing a file.
 *
 * @param nFd Descriptor.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x003132c0
 * @ghidraAddress PAL: 0x0037f980
 */
int sceMcClose(int nFd);

/**
 * Start moving the position of a file.
 *
 * @param nFd Descriptor.
 * @param nOffset Offset in bytes.
 * @param nMode Origin of the offset.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313378
 * @ghidraAddress PAL: 0x0037fa38
 */
int sceMcSeek(int nFd, int nOffset, int nMode);

/**
 * Start reading from a file.
 *
 * @param nFd Descriptor.
 * @param pBuffer Destination.
 * @param nSize Size in bytes.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x003134e0
 * @ghidraAddress PAL: 0x0037fba0
 */
int sceMcRead(int nFd, void *pBuffer, int nSize);

/**
 * Start writing to a file.
 *
 * @param nFd Descriptor.
 * @param pBuffer Source.
 * @param nSize Size in bytes.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x003135f8
 * @ghidraAddress PAL: 0x0037fcb8
 */
int sceMcWrite(int nFd, const void *pBuffer, int nSize);

/**
 * Wait for, or poll, the command in flight.
 *
 * @param nMode sceMcWait or sceMcNoWait.
 * @param pnCommand Receives the sceMcFuncNo value of the command, or null.
 * @param pnResult Receives the command's result once it has finished, or null.
 * @return sceMcExecIdle, sceMcExecRun, or sceMcExecFinish.
 * @ghidraAddress NTSC-U/C: 0x003137e0
 * @ghidraAddress PAL: 0x0037fe70
 */
int sceMcSync(int nMode, int *pnCommand, int *pnResult);

/**
 * Start reading the card type, the free space, and the format state.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pnType Receives the sceMcType value, or null.
 * @param pnFree Receives the free space in kilobytes, or null.
 * @param pnFormat Receives one when the card is formatted, or null.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313918
 * @ghidraAddress PAL: 0x0037ffa8
 */
int sceMcGetInfo(int nPort, int nSlot, int *pnType, int *pnFree, int *pnFormat);

/**
 * Start listing the entries that match a path.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pszName Path, optionally ending in a wildcard.
 * @param nMode Zero to start a listing, nonzero to continue it.
 * @param nMaxEntries Capacity of pTable.
 * @param pTable Receives the entries.
 * @return As sceMcOpen(). sceMcSync() reports the number of entries.
 * @ghidraAddress NTSC-U/C: 0x00313aa0
 * @ghidraAddress PAL: 0x00380130
 */
int sceMcGetDir(int nPort,
                int nSlot,
                const char *pszName,
                unsigned int nMode,
                int nMaxEntries,
                sceMcTblGetDir *pTable);

/**
 * Start formatting a card.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313bf0
 * @ghidraAddress PAL: 0x00380280
 */
int sceMcFormat(int nPort, int nSlot);

/**
 * Start deleting a file or an empty directory.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pszName Path of the entry.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313cc0
 * @ghidraAddress PAL: 0x00380350
 */
int sceMcDelete(int nPort, int nSlot, const char *pszName);

/**
 * Start erasing the format of a card.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return As sceMcOpen().
 * @ghidraAddress NTSC-U/C: 0x00313dd8
 * @ghidraAddress PAL: 0x00380468
 */
int sceMcUnformat(int nPort, int nSlot);

#ifdef __cplusplus
}
#endif

#endif
