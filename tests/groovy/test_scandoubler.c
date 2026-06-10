/* Unit tests for the scandoubler session decision module (pure logic, no
 * hardware). Specifies the v2 protocol behavior of dragonfleas/Groovy_MiSTer#1. */
#include <criterion/criterion.h>
#include <stdint.h>
#include "support/groovy/scandoubler.h"

/* T1: CMD_SWITCHRES is valid at 26 bytes (legacy) or 27 bytes (v2);
 * every other length is dropped, as the server has always done. */
Test(scandoubler, switchres_length_acceptance)
{
   cr_assert(gsd_switchres_len_ok(26), "legacy 26-byte packet is valid");
   cr_assert(gsd_switchres_len_ok(27), "v2 27-byte packet is valid");
   cr_assert(!gsd_switchres_len_ok(25), "short packet dropped");
   cr_assert(!gsd_switchres_len_ok(28), "long packet dropped");
   cr_assert(!gsd_switchres_len_ok(0),  "empty packet dropped");
}

/* T2: the first v2 switchres requesting the scandoubler (cfg currently off)
 * decides ON, takes session control, and saves the user's original value. */
Test(scandoubler, first_enable_takes_control_and_saves_original)
{
   gsd_state_t s;
   gsd_init(&s);

   gsd_action_t a = gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, /*current_cfg=*/0);

   cr_assert_eq(a, GSD_SET_ON, "must turn the scandoubler on");
   cr_assert(s.session_active, "session control must be latched");
   cr_assert_eq(s.saved_cfg, 0, "original cfg value must be saved for restore");
}

/* T3: repeating a request that matches hardware state decides NONE, so mode
 * switches don't spam redundant button refreshes. */
Test(scandoubler, matching_request_is_idempotent)
{
   gsd_state_t s;
   gsd_init(&s);

   gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, 0);          /* ON applied */
   gsd_action_t a = gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, /*now on*/1);

   cr_assert_eq(a, GSD_NONE, "already on: no action");
}

/* T4: during a v2 session the client owns the state: flag clear decides OFF
 * even when the user's ini had forced_scandoubler on. */
Test(scandoubler, flag_clear_disables_even_over_ini_setting)
{
   gsd_state_t s;
   gsd_init(&s);

   gsd_action_t a = gsd_on_switchres(&s, 27, 0x00, /*ini had it on*/1);

   cr_assert_eq(a, GSD_SET_OFF, "client-controlled session must turn it off");
   cr_assert_eq(s.saved_cfg, 1, "the ini value must be saved for restore");
}

/* T5: legacy 26-byte packets are inert in every state. */
Test(scandoubler, legacy_packet_never_touches_state)
{
   gsd_state_t s;
   gsd_init(&s);

   cr_assert_eq(gsd_on_switchres(&s, 26, 0xFF, 0), GSD_NONE,
                "legacy packet with garbage trailing byte arg: no action");
   cr_assert(!s.session_active, "legacy packet must not take control");

   gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, 0); /* v2 session active */
   cr_assert_eq(gsd_on_switchres(&s, 26, 0x00, 1), GSD_NONE,
                "legacy packet mid-session: still no action");
}

/* T6: reserved displayFlags bits 1-7 are ignored. */
Test(scandoubler, reserved_flag_bits_are_ignored)
{
   gsd_state_t s;
   gsd_init(&s);
   cr_assert_eq(gsd_on_switchres(&s, 27, 0xFE, 0), GSD_NONE,
                "bit 0 clear: off, whatever the reserved bits say");

   gsd_init(&s);
   cr_assert_eq(gsd_on_switchres(&s, 27, 0x03, 0), GSD_SET_ON,
                "bit 0 set: on, reserved bit 1 ignored");
}

/* T7: closing the session restores the user's saved ini value and releases
 * control; closing without ever taking control does nothing. */
Test(scandoubler, close_restores_saved_value_and_releases)
{
   gsd_state_t s;
   gsd_init(&s);

   /* never took control: nothing to restore */
   cr_assert_eq(gsd_on_close(&s, 0), GSD_NONE);

   /* ini off, session turned it on, close turns it back off */
   gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, 0);
   cr_assert_eq(gsd_on_close(&s, /*hardware now on*/1), GSD_SET_OFF,
                "must restore the user's original (off)");
   cr_assert(!s.session_active, "control must be released on close");

   /* released: a second close is inert */
   cr_assert_eq(gsd_on_close(&s, 0), GSD_NONE);
}

/* T8: ini already on and the first request is also on: no action, but
 * control is still taken, so a later "off" works and close restores to on. */
Test(scandoubler, ini_on_edge_latches_control_without_action)
{
   gsd_state_t s;
   gsd_init(&s);

   cr_assert_eq(gsd_on_switchres(&s, 27, GSD_FLAG_SCANDOUBLER, 1), GSD_NONE,
                "already on from ini: no redundant action");
   cr_assert(s.session_active, "control must still be latched");
   cr_assert_eq(s.saved_cfg, 1);

   cr_assert_eq(gsd_on_switchres(&s, 27, 0x00, 1), GSD_SET_OFF,
                "480p switch must disable despite the ini");
   cr_assert_eq(gsd_on_close(&s, 0), GSD_SET_ON,
                "close must restore the ini's on");
}
