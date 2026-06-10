#ifndef GROOVY_SCANDOUBLER_H
#define GROOVY_SCANDOUBLER_H

#include <stdint.h>
#include <stddef.h>

/* Protocol v2 scandoubler session control: pure decision logic, applied by
 * the caller via cfg.forced_scandoubler + user_io_send_buttons(1).
 * displayFlags (byte 26): bit 0 = scandoubler, bits 1-7 reserved. */

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

/* Legacy 26-byte packets decide GSD_NONE. A 27-byte packet takes session
 * control (saving current_cfg once) and returns the action matching bit 0. */
gsd_action_t gsd_on_switchres(gsd_state_t *s, size_t len, uint8_t flags,
                              uint8_t current_cfg);

/* On CMD_CLOSE / core teardown: restore the saved value, release control. */
gsd_action_t gsd_on_close(gsd_state_t *s, uint8_t current_cfg);

#ifdef __cplusplus
}
#endif

#endif /* GROOVY_SCANDOUBLER_H */
