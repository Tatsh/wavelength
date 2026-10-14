#ifndef DECI2_H
#define DECI2_H

#ifdef __cplusplus
extern "C" {
#endif

/** DECI2 protocol sockets to the development host, through the kernel DECI2 manager. */

/**
 * Opens a DECI2 socket.
 *
 * @param protocol Protocol number.
 * @param opt Pointer passed to @p handler.
 * @param handler Event handler, given the event, its parameter, and @p opt.
 * @return Socket, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031cca8
 * @ghidraAddress PAL: 0x003895c0
 */
int sceDeci2Open(unsigned short protocol,
                 void *opt,
                 void (*handler)(int event, int param, void *opt));

/**
 * Requests a send to a node. The handler receives a write event once the send may start.
 *
 * @param s Socket.
 * @param dest Destination node.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x0031ccf8
 * @ghidraAddress PAL: 0x00389610
 */
int sceDeci2ReqSend(int s, char dest);

/**
 * Runs the pending events of a socket.
 *
 * @param s Socket.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x0031cd28
 * @ghidraAddress PAL: 0x00389640
 */
int sceDeci2Poll(int s);

/**
 * Receives packet bytes inside the event handler.
 *
 * @param s Socket.
 * @param buf Destination.
 * @param len Byte count.
 * @return Bytes received, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031cd50
 * @ghidraAddress PAL: 0x00389668
 */
int sceDeci2ExRecv(int s, void *buf, unsigned short len);

/**
 * Sends packet bytes inside the event handler.
 *
 * @param s Socket.
 * @param buf Source.
 * @param len Byte count.
 * @return Bytes sent, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x0031cd88
 * @ghidraAddress PAL: 0x003896a0
 */
int sceDeci2ExSend(int s, void *buf, unsigned short len);

/**
 * Writes a string to the kernel console of the development host.
 *
 * @param s String.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x0031cdd0
 * @ghidraAddress PAL: 0x003896e8
 */
int kputs(char *s);

#ifdef __cplusplus
}
#endif

#endif
