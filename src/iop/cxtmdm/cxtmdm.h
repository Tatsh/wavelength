#pragma once

/**
 * The Conexant USB modem driver registers with usbd and drives each matching modem for the modem
 * library. The arguments `dial=<string>` and `lmode=AUTOLOAD` or `lmode=TESTLOAD` configure it.
 */

/**
 * Module entry. Parse the arguments and register the driver with usbd.
 *
 * @param argc Argument count.
 * @param argv Arguments.
 * @return #RESIDENT_END, #NO_RESIDENT_END when an automatic load found no modem or a test load
 * found none, or 5 when a test load found a modem.
 * @ghidraAddress NTSC-U/C: 0x00001be4
 */
extern "C" int start(int argc, char **argv);
