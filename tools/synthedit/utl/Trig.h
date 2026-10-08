#pragma once

/**
 * Build the sine tables. The size of the quarter-turn table comes from `sin_table_size` of the
 * `math` configuration.
 *
 * @ghidraAddress 0x1001bb70
 */
void TrigInit();

/**
 * Free the quarter-turn sine tables.
 *
 * @ghidraAddress 0x1001bcc0
 */
void TrigTerminate();
