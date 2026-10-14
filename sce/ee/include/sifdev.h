#ifndef SIFDEV_H
#define SIFDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SIF DMA and registers, the IOP heap, module loading, IOP reset and reboot, and the file service
 * the IOP provides to the Emotion Engine.
 */

/** One SIF DMA transfer from the Emotion Engine to the IOP. */
typedef struct {
    unsigned int data; /*!< Source address in main memory, 16-byte aligned. */
    unsigned int addr; /*!< Destination address in IOP memory. */
    unsigned int size; /*!< Size in bytes. */
    unsigned int mode; /*!< Transfer attribute bits such as #SIF_DMA_INT_O. */
} sceSifDmaData;

/** Transfer attribute that interrupts the IOP once the transfer ends. */
#define SIF_DMA_INT_O 0x04
/** Transfer attribute that ends the DMA chain with the transfer. */
#define SIF_DMA_ERT 0x40

/** SIF register of the EE command buffer address. The Emotion Engine writes it. */
#define SIF_REG_MAINADDR 1
/** SIF register of the IOP command buffer address. The IOP writes it. */
#define SIF_REG_SUBADDR 2
/** SIF register of flags the Emotion Engine sets for the IOP. */
#define SIF_REG_MSFLAG 3
/** SIF register of flags the IOP sets for the Emotion Engine. Writing a bit clears it. */
#define SIF_REG_SMFLAG 4

/** Software register of the IOP command buffer address the command layer sends to. */
#define SIF_SYSREG_SUBADDR 0x80000000
/** Software register of the address of the EE command layer state. */
#define SIF_SYSREG_MAINADDR 0x80000001
/** Software register that is nonzero once the IOP RPC layer has initialised. */
#define SIF_SYSREG_RPCINIT 0x80000002

/** Bit of #SIF_REG_SMFLAG the IOP sets once SIF has initialised. */
#define SIF_STAT_SIFINIT 0x10000
/** Bit of #SIF_REG_SMFLAG the IOP sets once its command layer has initialised. */
#define SIF_STAT_CMDINIT 0x20000
/** Bit of #SIF_REG_SMFLAG the IOP sets once it has booted. */
#define SIF_STAT_BOOTEND 0x40000

/** Width of the value sceSifGetIopAddr() and sceSifSetIopAddr() move. */
enum {
    SIF_IOP_VALUE_BYTE = 0, /*!< One byte. */
    SIF_IOP_VALUE_HALF = 1, /*!< Two bytes. */
    SIF_IOP_VALUE_WORD = 2, /*!< Four bytes. */
};

/** The entry point and global pointer of an executable the IOP loaded into main memory. */
typedef struct {
    unsigned int epc;   /*!< Entry point. */
    unsigned int gp;    /*!< Global pointer. */
    unsigned int sp;    /*!< Stack pointer, unused by the loader. */
    unsigned int dummy; /*!< Padding. */
} sceExecData;

/** Open for reading. */
#define SCE_RDONLY 0x0001
/** Open for writing. */
#define SCE_WRONLY 0x0002
/** Open for reading and writing. */
#define SCE_RDWR 0x0003
/** Create the file when it does not exist. */
#define SCE_CREAT 0x0200
/** Truncate the file on opening. */
#define SCE_TRUNC 0x0400
/** Return from each request before it completes. */
#define SCE_NOWAIT 0x8000

/** sceIoctl() request that reports whether a request of a #SCE_NOWAIT file is still running. */
#define SCE_FS_EXECUTING 0x1

/** Seek origins of sceLseek(). */
#define SCE_SEEK_SET 0
#define SCE_SEEK_CUR 1
#define SCE_SEEK_END 2

/**
 * Send a control request to a device of the IOP file system.
 *
 * @param name The device, such as "dev9x:".
 * @param cmd The request.
 * @param arg The argument, or null.
 * @param arglen The size of the argument.
 * @param bufp Receives the result, or null.
 * @param buflen The size of the result buffer.
 * @return 0 or a positive value on success, otherwise a negative error.
 * @ghidraAddress NTSC-U/C: 0x0031b7d0
 * @ghidraAddress PAL: 0x003880e8
 */
int sceDevctl(
    const char *name, int cmd, void *arg, unsigned int arglen, void *bufp, unsigned int buflen);

/**
 * Stop SIF DMA.
 *
 * @ghidraAddress NTSC-U/C: 0x003171f0
 * @ghidraAddress PAL: 0x00383ab0
 */
void sceSifStopDma(void);

/**
 * Report the state of a SIF DMA transfer.
 *
 * @param id Transfer identifier from sceSifSetDma().
 * @return Negative once the transfer has finished.
 * @ghidraAddress NTSC-U/C: 0x003172c0
 * @ghidraAddress PAL: 0x00383b80
 */
int sceSifDmaStat(unsigned int id);

/**
 * Report the state of a SIF DMA transfer from an interrupt handler.
 *
 * @param id Transfer identifier from isceSifSetDma().
 * @return Negative once the transfer has finished.
 * @ghidraAddress NTSC-U/C: 0x003172d0
 */
int isceSifDmaStat(unsigned int id);

/**
 * Queue SIF DMA transfers to the IOP.
 *
 * @param sdd Transfers.
 * @param len Number of transfers.
 * @return A transfer identifier, or zero when the queue is full.
 * @ghidraAddress NTSC-U/C: 0x003172e0
 * @ghidraAddress PAL: 0x00383ba0
 */
unsigned int sceSifSetDma(sceSifDmaData *sdd, int len);

/**
 * Queue SIF DMA transfers to the IOP from an interrupt handler.
 *
 * @param sdd Transfers.
 * @param len Number of transfers.
 * @return A transfer identifier, or zero when the queue is full.
 * @ghidraAddress NTSC-U/C: 0x003172f0
 * @ghidraAddress PAL: 0x00383bb0
 */
unsigned int isceSifSetDma(sceSifDmaData *sdd, int len);

/**
 * Start the SIF0 channel in chain mode, ready for the next packet from the IOP.
 *
 * @ghidraAddress NTSC-U/C: 0x00317300
 * @ghidraAddress PAL: 0x00383bc0
 */
void sceSifSetDChain(void);

/**
 * Start the SIF0 channel in chain mode from an interrupt handler.
 *
 * @ghidraAddress NTSC-U/C: 0x00317310
 * @ghidraAddress PAL: 0x00383bd0
 */
void isceSifSetDChain(void);

/**
 * Write a SIF register.
 *
 * @param reg Register number such as #SIF_REG_SMFLAG or #SIF_SYSREG_SUBADDR.
 * @param val Value.
 * @return A kernel-defined value.
 * @ghidraAddress NTSC-U/C: 0x00317320
 * @ghidraAddress PAL: 0x00383be0
 */
int sceSifSetReg(unsigned int reg, int val);

/**
 * Read a SIF register.
 *
 * @param reg Register number such as #SIF_REG_SMFLAG or #SIF_SYSREG_SUBADDR.
 * @return The register value.
 * @ghidraAddress NTSC-U/C: 0x00317330
 * @ghidraAddress PAL: 0x00383bf0
 */
int sceSifGetReg(unsigned int reg);

/**
 * Bind the IOP heap service, retrying until the server exists.
 *
 * @return Zero, or -1 when the bind request could not be sent.
 * @ghidraAddress NTSC-U/C: 0x0031ba08
 * @ghidraAddress PAL: 0x00388320
 */
int sceSifInitIopHeap(void);

/**
 * Allocate IOP memory.
 *
 * @param size Size in bytes.
 * @return The IOP address, or null when the service is not bound or the call fails.
 * @ghidraAddress NTSC-U/C: 0x0031ba90
 * @ghidraAddress PAL: 0x003883a8
 */
void *sceSifAllocIopHeap(unsigned int size);

/**
 * Release IOP memory.
 *
 * @param addr IOP address from sceSifAllocIopHeap().
 * @return The server's result, zero when the service is not bound, or -1 when the call fails.
 * @ghidraAddress NTSC-U/C: 0x0031bb00
 * @ghidraAddress PAL: 0x00388418
 */
int sceSifFreeIopHeap(void *addr);

/**
 * Release IOP memory. The routine is sceSifFreeIopHeap() under another name.
 *
 * @param addr IOP address.
 * @return As sceSifFreeIopHeap().
 * @ghidraAddress NTSC-U/C: 0x0031bb78
 * @ghidraAddress PAL: 0x00388490
 */
int sceSifFreeSysMemory(void *addr);

/**
 * Load and start an IOP module from a file.
 *
 * @param filename Module path, cut to 251 characters.
 * @param args Size of @p argp in bytes. At most 252 bytes are sent.
 * @param argp Arguments, each terminated by a zero byte, or null.
 * @return The module identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031c2a8
 * @ghidraAddress PAL: 0x00388bc0
 */
int sceSifLoadModule(const char *filename, int args, const char *argp);

/**
 * Stop an IOP module.
 *
 * @param modid Module identifier.
 * @param args Size of @p argp in bytes. At most 252 bytes are sent.
 * @param argp Arguments, each terminated by a zero byte, or null.
 * @param result Receives the result of the module stop routine.
 * @return The module identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031bd38
 * @ghidraAddress PAL: 0x00388650
 */
int sceSifStopModule(int modid, int args, const char *argp, int *result);

/**
 * Unload a stopped IOP module.
 *
 * @param modid Module identifier.
 * @return The module identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031bf40
 * @ghidraAddress PAL: 0x00388858
 */
int sceSifUnloadModule(int modid);

/**
 * Find a loaded IOP module by name.
 *
 * @param modulename Module name, cut to 251 characters.
 * @return The module identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031bfd0
 * @ghidraAddress PAL: 0x003888e8
 */
int sceSifSearchModuleByName(const char *modulename);

/**
 * Send the IOP a reset command that reboots it with an argument string.
 *
 * The argument is copied without its terminator and without a length check.
 *
 * @param arg Argument string of up to 80 characters, such as `rom0:UDNL <image>`.
 * @param mode Reset mode the IOP receives.
 * @return Nonzero once the command is queued, zero when the DMA queue is full.
 * @ghidraAddress NTSC-U/C: 0x0031c2f0
 * @ghidraAddress PAL: 0x00388c08
 */
int sceSifResetIop(const char *arg, int mode);

/**
 * Report whether the IOP has finished rebooting. The console is closed once it has, and the next
 * console write opens it again.
 *
 * @return Nonzero once it has.
 * @ghidraAddress NTSC-U/C: 0x0031c430
 * @ghidraAddress PAL: 0x00388d48
 */
int sceSifSyncIop(void);

/**
 * Reboot the IOP with a replacement image.
 *
 * @param imgname Image path. With the `rom0:UDNL ` prefix and its terminator it must fit in 80
 *     bytes.
 * @return Nonzero once the reboot request is sent, zero when the path is too long or the DMA
 *     queue is full.
 * @ghidraAddress NTSC-U/C: 0x0031c468
 * @ghidraAddress PAL: 0x00388d80
 */
int sceSifRebootIop(const char *imgname);

/**
 * Bind the file service client to the server and register its command handlers. The file calls
 * bind on first use.
 *
 * @return Zero, -1 when the server cannot be bound, or -0x10001 when its initialisation call
 * fails.
 * @ghidraAddress NTSC-U/C: 0x0031a288
 * @ghidraAddress PAL: 0x00386ba0
 */
int sceFsInit(void);

/**
 * Unbind the file service client after an IOP reboot. The next call binds the new server.
 *
 * @return Zero.
 * @ghidraAddress NTSC-U/C: 0x0031a520
 * @ghidraAddress PAL: 0x00386e38
 */
int sceFsReset(void);

/**
 * Open a file.
 *
 * @param filename Path, with its device prefix.
 * @param flag Open flags such as #SCE_RDONLY.
 * @return A descriptor, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031a558
 * @ghidraAddress PAL: 0x00386e70
 */
int sceOpen(const char *filename, int flag, ...);

/**
 * Close a file.
 *
 * @param fd Descriptor.
 * @return Zero, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031a7e0
 * @ghidraAddress PAL: 0x003870f8
 */
int sceClose(int fd);

/**
 * Read from a file.
 *
 * @param fd Descriptor.
 * @param buf Destination.
 * @param nbyte Byte count.
 * @return The bytes read, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031ab98
 * @ghidraAddress PAL: 0x003874b0
 */
int sceRead(int fd, void *buf, int nbyte);

/**
 * Write to a file.
 *
 * @param fd Descriptor.
 * @param buf Source.
 * @param nbyte Byte count.
 * @return The bytes written, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031adf8
 * @ghidraAddress PAL: 0x00387710
 */
int sceWrite(int fd, const void *buf, int nbyte);

/**
 * Move the position of a file.
 *
 * @param fd Descriptor.
 * @param offset Offset from @p where.
 * @param where Origin such as #SCE_SEEK_SET.
 * @return The new position, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031a960
 * @ghidraAddress PAL: 0x00387278
 */
int sceLseek(int fd, int offset, int where);

/**
 * Send a control request for an open file to its device.
 *
 * @param fd Descriptor.
 * @param req Request code.
 * @param arg Request argument.
 * @return A request-defined value, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031b0b8
 * @ghidraAddress PAL: 0x003879d0
 */
int sceIoctl(int fd, int req, void *arg);

/** Status of a file sceGetstat() reports. */
struct sce_stat {
    unsigned int st_mode;       /*!< File type and access bits. */
    unsigned int st_attr;       /*!< Device-specific attributes. */
    unsigned int st_size;       /*!< Size in bytes, low word. */
    unsigned char st_ctime[8];  /*!< Creation time. */
    unsigned char st_atime[8];  /*!< Last access time. */
    unsigned char st_mtime[8];  /*!< Last modification time. */
    unsigned int st_hisize;     /*!< Size in bytes, high word. */
    unsigned int st_private[6]; /*!< Device-specific data. */
};

/**
 * Report the status of a file.
 *
 * @param name Path, with its device prefix.
 * @param buf Receives the status.
 * @return Zero, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031b610
 * @ghidraAddress PAL: 0x00387f28
 */
int sceGetstat(const char *name, struct sce_stat *buf);

/**
 * Create a directory.
 *
 * @param path Path, with its device prefix.
 * @param mode Access bits.
 * @return Zero, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x0031b448
 * @ghidraAddress PAL: 0x00387d60
 */
int sceMkdir(const char *path, int mode);

#ifdef __cplusplus
}
#endif

#endif
