#pragma once

/**
 * The SNProfil module serves a profiling buffer to the EE through a SIF RPC server. Every request
 * receives a ProfileBufferInfo that locates ProfBuffer in IOP memory.
 */

/** SIF RPC server identifier of the module. */
constexpr unsigned int kSnProfilRpcServer = 0x12345677;

/** Size of ProfileBuffer::header. */
constexpr int kProfileHeaderSize = 0x80;

/** Size of ProfileBuffer::samples. */
constexpr int kProfileSamplesSize = 0x8000;

/** The profiling buffer. The module itself never reads or writes it. */
struct ProfileBuffer {
    unsigned char header[kProfileHeaderSize];   /*!< Header area. */
    unsigned char samples[kProfileSamplesSize]; /*!< Sample area. */
};

/** Reply to every request. It overwrites the request buffer. */
struct ProfileBufferInfo {
    ProfileBuffer *buffer;  /*!< The profiling buffer. */
    int headerSize;         /*!< Size of ProfileBuffer::header. */
    unsigned char *samples; /*!< ProfileBuffer::samples. */
    int samplesSize;        /*!< Size of ProfileBuffer::samples. */
};

/**
 * Module entry. Start the thread that runs the RPC server.
 *
 * @param argc Argument count.
 * @param argv Arguments.
 * @return #RESIDENT_END, or #NO_RESIDENT_END when the thread cannot be created.
 * @ghidraAddress NTSC-U/C: 0x00000000
 */
extern "C" int start(int argc, char **argv);
