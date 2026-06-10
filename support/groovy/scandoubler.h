#ifndef GROOVY_SCANDOUBLER_H
#define GROOVY_SCANDOUBLER_H

#include <stdint.h>
#include <stddef.h>

/* Scandoubler session control for protocol v2 (27-byte CMD_SWITCHRES).
 * Pure decision logic so it is unit-testable off-target; the caller applies
 * actions to cfg.forced_scandoubler + user_io_send_buttons(1).
 *
 * Byte 26 of the v2 packet is displayFlags: bit 0 requests the scandoubler
 * for the announced mode, bits 1-7 are reserved and ignored. */

#define GSD_FLAG_SCANDOUBLER 0x01

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GSD_NONE = 0,   /* hardware state already matches; do nothing */
	GSD_SET_ON,     /* set forced_scandoubler = 1 and refresh buttons */
	GSD_SET_OFF,    /* set forced_scandoubler = 0 and refresh buttons */
} gsd_action_t;

typedef struct {
	uint8_t session_active; /* a 27-byte switchres has taken control */
	uint8_t saved_cfg;      /* user's forced_scandoubler at takeover */
} gsd_state_t;

void gsd_init(gsd_state_t *s);

/* Valid CMD_SWITCHRES datagram lengths: 26 (legacy) or 27 (v2). */
int gsd_switchres_len_ok(size_t len);

/* Decide on a CMD_SWITCHRES. Legacy 26-byte packets never touch the
 * scandoubler. A 27-byte packet takes session control (saving current_cfg
 * the first time) and returns the action that makes the hardware match
 * flags bit 0, or GSD_NONE if it already does. */
gsd_action_t gsd_on_switchres(gsd_state_t *s, size_t len, uint8_t flags,
                              uint8_t current_cfg);

/* Decide on CMD_CLOSE (or core teardown): if a session took control, return
 * the action restoring the user's saved value and release control. */
gsd_action_t gsd_on_close(gsd_state_t *s, uint8_t current_cfg);

#ifdef __cplusplus
}
#endif

#endif /* GROOVY_SCANDOUBLER_H */
