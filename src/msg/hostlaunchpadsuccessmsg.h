#pragma once

#include "msg/message.h"

/**
 * Identity that HostLaunchpadSuccessMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b60
 */
extern int g_nHostLaunchpadSuccessMsgType;

/**
 * Message that reports that this console now hosts an online session.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d6000`. Its
 * receivers here read only its type.
 */
class HostLaunchpadSuccessMsg : public Message {};
