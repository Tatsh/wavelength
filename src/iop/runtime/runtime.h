#pragma once

/**
 * Run the constructors of the module's static objects, last listed first.
 *
 * The softFX module's copy of the routine is at 0x00003784.
 */
void RunGlobalConstructors();

/**
 * Run the destructors of the module's static objects, last constructed first.
 *
 * The softFX module's copy of the routine is at 0x00003720.
 */
void RunGlobalDestructors();
