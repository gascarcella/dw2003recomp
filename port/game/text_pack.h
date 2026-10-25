/* The text pack (text_pack.c; docs/PORT.md "Text packs"): translated entries laid over the English text files as the
 * game loads them. preset_language opens it for a language that has one (game_mods.c); the game's cdload reaches it
 * through port_text_sectors and port_text_loaded (include/port.h). */
#ifndef PORT_TEXT_PACK_H
#define PORT_TEXT_PACK_H

/* Reads and checks the pack `name` in preset_language's directory beside the binary (mods/preset_language/<name>),
 * or the file DW3_PORT_TEXT_PACK names (a test hook). Logs what it found; with no valid pack nothing is replaced and
 * the game plays in English. 1 when the pack is in use. */
int port_text_pack_open(const char *name);

#endif /* PORT_TEXT_PACK_H */
