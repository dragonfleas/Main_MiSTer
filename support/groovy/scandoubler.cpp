#include "scandoubler.h"

void gsd_init(gsd_state_t *s)
{
	s->session_active = 0;
	s->saved_cfg = 0;
}

int gsd_switchres_len_ok(size_t len)
{
	return len == 26 || len == 27;
}

gsd_action_t gsd_on_switchres(gsd_state_t *s, size_t len, uint8_t flags,
                              uint8_t current_cfg)
{
	if (len != 27)
		return GSD_NONE;

	if (!s->session_active)
	{
		s->session_active = 1;
		s->saved_cfg = current_cfg;
	}

	uint8_t want = (flags & GSD_FLAG_SCANDOUBLER) ? 1 : 0;
	if (want == (current_cfg ? 1 : 0))
		return GSD_NONE;
	return want ? GSD_SET_ON : GSD_SET_OFF;
}

gsd_action_t gsd_on_close(gsd_state_t *s, uint8_t current_cfg)
{
	if (!s->session_active)
		return GSD_NONE;

	uint8_t want = s->saved_cfg ? 1 : 0;
	s->session_active = 0;

	if (want == (current_cfg ? 1 : 0))
		return GSD_NONE;
	return want ? GSD_SET_ON : GSD_SET_OFF;
}
