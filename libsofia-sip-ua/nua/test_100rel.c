/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005 Nokia Corporation.
 *
 * Contact: Pekka Pessi <pekka.pessi@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**@CFILE test_nua_100rel.c
 * @brief NUA-10 tests: early session, PRACK, UPDATE, precondition.
 *
 * @author Pekka Pessi <Pekka.Pessi@nokia.com>
 * @author Martti Mela <Martti Mela@nokia.com>
 *
 * @date Created: Wed Aug 17 12:12:12 EEST 2005 ppessi
 */

#include "config.h"

#include "test_nua.h"
#include <sofia-sip/su_tag_class.h>

#if HAVE_FUNC
#elif HAVE_FUNCTION
#define __func__ __FUNCTION__
#else
#define __func__ "test_call_hold"
#endif

/* ======================================================================== */

/*
 X  accept_pracked    ep
 |-------INVITE------>|
 |        (sdp)       |
 |                    |
 |<----100 Trying-----|
 |                    |
 |<-------180---------|
 |       (sdp)        |
 |-------PRACK------->|
 |<-------200---------|
 |                    |
 |<------200 OK-------|
 |--------ACK-------->|
 |                    |
*/
int accept_pracked(CONDITION_PARAMS)
{
  if (!(check_handle(ep, call, nh, SIP_500_INTERNAL_SERVER_ERROR)))
    return 0;

  save_event_in_list(ctx, event, ep, call);

  switch (event) {
  case nua_i_prack:
    if (200 <= status && status < 300) {
      RESPOND(ep, call, nh, SIP_200_OK, TAG_END());
      ep->next_condition = until_ready;
    }
  default:
    break;
  }

  switch (callstate(tags)) {
  case nua_callstate_received:
    RESPOND(ep, call, nh, SIP_180_RINGING,
	    TAG_IF(call->sdp, SOATAG_USER_SDP_STR(call->sdp)),
	    TAG_END());
    return 0;
  case nua_callstate_terminated:
    if (call)
      nua_handle_destroy(call->nh), call->nh = NULL;
    return 1;
  default:
    return 0;
  }
}


/*
 X  ringing_pracked    ep
 |-------INVITE------>|
 |<----100 Trying-----|
 |                    |
 |<-------183---------|
 |-------PRACK------->|
 |<-------200---------|
 |                    |
 |<-------180---------|
 |-------PRACK------->|
 |<-------200---------|
 |                    |
 |<------200 OK-------|
 |--------ACK-------->|
 |                    |
*/
int ringing_pracked(CONDITION_PARAMS)
{
  if (!(check_handle(ep, call, nh, SIP_500_INTERNAL_SERVER_ERROR)))
    return 0;

  save_event_in_list(ctx, event, ep, call);

  switch (event) {
  case nua_i_prack:
    if (200 <= status && status < 300) {
      RESPOND(ep, call, nh, SIP_180_RINGING, TAG_END());
      ep->next_condition = accept_pracked;
    }
  default:
    break;
  }

  switch (callstate(tags)) {
  case nua_callstate_received:
    RESPOND(ep, call, nh, SIP_183_SESSION_PROGRESS,
	    TAG_IF(call->sdp, SOATAG_USER_SDP_STR(call->sdp)),
	    TAG_END());
    return 0;
  case nua_callstate_terminated:
    if (call)
      nua_handle_destroy(call->nh), call->nh = NULL;
    return 1;
  default:
    return 0;
  }
}

/*
 X  accept_updated    ep
 |-------INVITE------>|
 |       (sdp)        |
 |<----100 Trying-----|
 |                    |
 |<-------183---------|
 |       (sdp)        |
 |-------PRACK------->|
 |       (sdp)        |
 |<-------200---------|
 |       (sdp)        |
 |                    |
 |-------UPDATE------>|
 |       (sdp)        |
 |<-------200---------|
 |       (sdp)        |
 |                    |
<using  acccept_pracked>
 |                    |
 |<-------180---------|
 |-------PRACK------->|
 |<-------200---------|
 |                    |
 |<------200 OK-------|
 |--------ACK-------->|
 |                    |
*/
int ringing_updated(CONDITION_PARAMS)
{
  if (!(check_handle(ep, call, nh, SIP_500_INTERNAL_SERVER_ERROR)))
    return 0;

  save_event_in_list(ctx, event, ep, call);

  switch (event) {
  case nua_i_update:
    if (200 <= status && status < 300) {
      RESPOND(ep, call, nh, SIP_180_RINGING, TAG_END());
      ep->next_condition = accept_pracked;
    }
    return 0;
  default:
    break;
  }

  switch (callstate(tags)) {
  case nua_callstate_received:
    RESPOND(ep, call, nh, SIP_183_SESSION_PROGRESS,
	    SIPTAG_REQUIRE_STR("100rel"),
	    TAG_IF(call->sdp, SOATAG_USER_SDP_STR(call->sdp)),
	    TAG_END());
    return 0;
  case nua_callstate_early:
    return 0;
  case nua_callstate_ready:
    return 1;
  case nua_callstate_terminated:
    if (call)
      nua_handle_destroy(call->nh), call->nh = NULL;
    return 1;
  default:
    return 0;
  }
}

/*
 X  accept_updated    ep
 |-------INVITE------>|
 |       (sdp)        |
 |<----100 Trying-----|
 |                    |
 |<-------183---------|
 |       (sdp)        |
 |-------PRACK------->|
 |       (sdp)        |
 |<-------200---------|
 |       (sdp)        |
 |                    |
 |-------UPDATE------>|
 |       (sdp)        |
 |<-------200---------|
 |       (sdp)        |
 |                    |
 |                    |
 |<-------180---------|
 |                    |
 |<------200 OK-------|
 |--------ACK-------->|
 |                    |
*/
int accept_updated(CONDITION_PARAMS)
{
  if (!(check_handle(ep, call, nh, SIP_500_INTERNAL_SERVER_ERROR)))
    return 0;

  save_event_in_list(ctx, event, ep, call);

  switch (event) {
  case nua_i_update:
    if (200 <= status && status < 300) {
      RESPOND(ep, call, nh, SIP_180_RINGING, TAG_END());
    }
    return 0;
  default:
    break;
  }

  switch (callstate(tags)) {
  case nua_callstate_received:
    RESPOND(ep, call, nh, SIP_183_SESSION_PROGRESS,
	    SIPTAG_REQUIRE_STR("100rel"),
	    TAG_IF(call->sdp, SOATAG_USER_SDP_STR(call->sdp)),
	    TAG_END());
    return 0;
  case nua_callstate_early:
    if (status == 180)
      RESPOND(ep, call, nh, SIP_200_OK, TAG_END());
    return 0;
  case nua_callstate_ready:
    return 1;
  case nua_callstate_terminated:
    if (call)
      nua_handle_destroy(call->nh), call->nh = NULL;
    return 1;
  default:
    return 0;
  }
}

int test_100rel(struct context *ctx)
{
  BEGIN();

  struct endpoint *a = &ctx->a,  *b = &ctx->b;
  struct call *a_call = a->call, *b_call = b->call;
  struct event *e;
  sip_t *sip;

  if (print_headings)
    printf("TEST NUA-10.1: Call with 100rel and 180\n");

/* Test for 100rel:

   A			B
   |-------INVITE------>|
   |<----100 Trying-----|
   |			|
   |<-------180---------|
   |-------PRACK------->|
   |<-------200---------|
   |			|
   |<------200 OK-------|
   |--------ACK-------->|
   |			|
   |<-------BYE---------|
   |-------200 OK-------|
   |			|

*/

  a_call->sdp = "m=audio 5008 RTP/AVP 8";
  b_call->sdp = "m=audio 5010 RTP/AVP 0 8";

  nua_set_params(ctx->a.nua,
		 NUTAG_EARLY_MEDIA(1),
		 TAG_END());
  run_a_until(ctx, nua_r_set_params, until_final_response);

  nua_set_params(ctx->b.nua,
		 NUTAG_EARLY_MEDIA(1),
		 TAG_END());
  run_b_until(ctx, nua_r_set_params, until_final_response);

  TEST_1(a_call->nh = nua_handle(a->nua, a_call, SIPTAG_TO(b->to), TAG_END()));

  INVITE(a, a_call, a_call->nh,
	 TAG_IF(!ctx->proxy_tests, NUTAG_URL(b->contact->m_url)),
	 SOATAG_USER_SDP_STR(a_call->sdp),
	 TAG_END());

  run_ab_until(ctx, -1, until_ready, -1, accept_pracked);

  /* Client transitions:
     INIT -(C1)-> CALLING: nua_invite(), nua_i_state
     CALLING -(C2)-> PROCEEDING: nua_r_invite, nua_i_state, nua_r_prack
     PROCEEDING -(C3+C4)-> READY: nua_r_invite, nua_i_state
  */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_calling); /* CALLING */
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 180);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  /*
   Server transitions:
   INIT -(S1)-> RECEIVED: nua_i_invite, nua_i_state
   RECEIVED -(S2a)-> EARLY: nua_respond(), nua_i_state
   EARLY -(S3b)-> COMPLETED: nua_respond(), nua_i_state
   COMPLETED -(S4)-> READY: nua_i_ack, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_i_invite);
  TEST(e->data->e_status, 100);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_received); /* RECEIVED */
  TEST_1(is_offer_recv(e->data->e_tags));

  /* Responded with 180 Ringing */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_answer_sent(e->data->e_tags));

  /* 180 is PRACKed */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  /* Does not have effect on call state */

  /* Respond with 200 OK */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_completed); /* COMPLETED */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_ack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  if (print_headings)
    printf("TEST NUA-10.1: PASSED\n");

  if (print_headings)
    printf("TEST NUA-10.1.1: terminate call\n");

  BYE(b, b_call, b_call->nh, TAG_END());
  run_ab_until(ctx, -1, until_terminated, -1, until_terminated);

  /* B transitions:
   READY --(T2)--> TERMINATING: nua_bye()
   TERMINATING --(T3)--> TERMINATED: nua_r_bye, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_r_bye);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  /* A: READY -(T1)-> TERMINATED: nua_i_bye, nua_i_state */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_bye);
  TEST(e->data->e_status, 200);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  nua_handle_destroy(a_call->nh), a_call->nh = NULL;
  nua_handle_destroy(b_call->nh), b_call->nh = NULL;

  if (print_headings)
    printf("TEST NUA-10.1.1: PASSED\n");

  /* -------------------------------------------------------------------- */

  if (print_headings)
    printf("TEST NUA-10.2: Call with 100rel, 183 and 180\n");

  TEST_1(a_call->nh = nua_handle(a->nua, a_call, SIPTAG_TO(b->to), TAG_END()));

  INVITE(a, a_call, a_call->nh,
	 TAG_IF(!ctx->proxy_tests, NUTAG_URL(b->contact->m_url)),
	 SOATAG_USER_SDP_STR(a_call->sdp),
	 TAG_END());

  run_ab_until(ctx, -1, until_ready, -1, ringing_pracked);

  /* Client transitions:
     INIT -(C1)-> CALLING: nua_invite(), nua_i_state
     CALLING -(C2)-> PROCEEDING: nua_r_invite, nua_i_state, nua_r_prack
     PROCEEDING -(C2)-> PROCEEDING: nua_r_invite, nua_i_state, nua_r_prack
     PROCEEDING -(C3+C4)-> READY: nua_r_invite, nua_i_state
  */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_calling); /* CALLING */
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 183);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 180);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  /*
   Server transitions:
   INIT -(S1)-> RECEIVED: nua_i_invite, nua_i_state
   RECEIVED -(S2a)-> EARLY: nua_respond(), nua_i_state
   EARLY -(S3b)-> COMPLETED: nua_respond(), nua_i_state
   COMPLETED -(S4)-> READY: nua_i_ack, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_i_invite);
  TEST(e->data->e_status, 100);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_received); /* RECEIVED */
  TEST_1(is_offer_recv(e->data->e_tags));

  /* Responded with 183 Session Progress */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 183);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_answer_sent(e->data->e_tags));

  /* 183 is PRACKed */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  /* Does not have effect on call state */

  /* Responded with 180 Ringing */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 180);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  /* 180 is PRACKed */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  /* Does not have effect on call state */

  /* Respond with 200 OK */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 200);
  TEST(callstate(e->data->e_tags), nua_callstate_completed); /* COMPLETED */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_ack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  if (print_headings)
    printf("TEST NUA-10.2: PASSED\n");

  if (print_headings)
    printf("TEST NUA-10.2.1: terminate call\n");

  BYE(b, b_call, b_call->nh, TAG_END());
  run_ab_until(ctx, -1, until_terminated, -1, until_terminated);

  /* B transitions:
   READY --(T2)--> TERMINATING: nua_bye()
   TERMINATING --(T3)--> TERMINATED: nua_r_bye, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_r_bye);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  /* A: READY -(T1)-> TERMINATED: nua_i_bye, nua_i_state */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_bye);
  TEST(e->data->e_status, 200);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  nua_handle_destroy(a_call->nh), a_call->nh = NULL;
  nua_handle_destroy(b_call->nh), b_call->nh = NULL;

  if (print_headings)
    printf("TEST NUA-10.2.1: PASSED\n");

  /* -------------------------------------------------------------------- */

  if (print_headings)
    printf("TEST NUA-10.3: Call with 100rel and preconditions\n");

/* Test for precondition:

   A			B
   |-------INVITE------>|
   |<----100 Trying-----|
   |			|
   |<-------183---------|
   |-------PRACK------->|
   |<-------200---------|
   |			|
   |-------UPDATE------>|
   |<-------200---------|
   |			|
   |<-------180---------|
   |-------PRACK------->|
   |<-------200---------|
   |			|
   |<------200 OK-------|
   |--------ACK-------->|
   |			|
   |<-------BYE---------|
   |-------200 OK-------|
   |			|

*/

  a_call->sdp = "m=audio 5008 RTP/AVP 8";
  b_call->sdp = "m=audio 5010 RTP/AVP 0 8";

  nua_set_params(ctx->a.nua,
		 NUTAG_EARLY_MEDIA(1),
		 SIPTAG_SUPPORTED_STR("100rel, precondition"),
		 TAG_END());
  run_a_until(ctx, nua_r_set_params, until_final_response);

  nua_set_params(ctx->b.nua,
		 NUTAG_EARLY_MEDIA(0),
		 SIPTAG_SUPPORTED_STR("100rel, precondition, timer"),
		 TAG_END());
  run_b_until(ctx, nua_r_set_params, until_final_response);

  TEST_1(a_call->nh = nua_handle(a->nua, a_call, SIPTAG_TO(b->to), TAG_END()));

  INVITE(a, a_call, a_call->nh,
	 TAG_IF(!ctx->proxy_tests, NUTAG_URL(b->contact->m_url)),
	 SOATAG_USER_SDP_STR(a_call->sdp),
	 SIPTAG_SUPPORTED_STR("100rel"),
	 SIPTAG_REQUIRE_STR("precondition"),
	 TAG_END());

  run_ab_until(ctx, -1, until_ready, -1, ringing_updated);

  /* Client transitions:
     INIT -(C1)-> CALLING: nua_invite(), nua_i_state
     CALLING -(C2)-> PROCEEDING: nua_r_invite, nua_i_state
     PROCEEDING -(C3+C4)-> READY: nua_r_invite, nua_i_state
  */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_calling); /* CALLING */
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 183);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(!is_answer_recv(e->data->e_tags));
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_update);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 180);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding); /* PROCEEDING */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);
  /* Does not have effect on call state */

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 200);
  TEST_1(sip = sip_object(e->data->e_msg));
  TEST_1(sip->sip_session_expires);
  TEST_S(sip->sip_session_expires->x_refresher, "uas");
  TEST_1(!sip_has_supported(sip->sip_require, "timer"));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  /*
   Server transitions:
   INIT -(S1)-> RECEIVED: nua_i_invite, nua_i_state
   RECEIVED -(S2a)-> EARLY: nua_respond(), nua_i_state
   EARLY -(S3b)-> COMPLETED: nua_respond(), nua_i_state
   COMPLETED -(S4)-> READY: nua_i_ack, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_i_invite);
  TEST(e->data->e_status, 100);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_received); /* RECEIVED */

  /* Responded with 183 Session Progress */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 183);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_answer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_offer_recv(e->data->e_tags));
  TEST_1(is_answer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_update);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_offer_recv(e->data->e_tags));
  TEST_1(is_answer_sent(e->data->e_tags));

  /* Responded with 180 Ringing */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 180);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  /* 180 PRACKed */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  /* Does not have effect on call state */

  /* Responded with 200 OK */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(e->data->e_status, 200);
  TEST(callstate(e->data->e_tags), nua_callstate_completed); /* COMPLETED */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_ack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  if (print_headings)
    printf("TEST NUA-10.3: PASSED\n");

  if (print_headings)
    printf("TEST NUA-10.3.1: terminate call\n");

  BYE(b, b_call, b_call->nh, TAG_END());
  run_ab_until(ctx, -1, until_terminated, -1, until_terminated);

  /* B transitions:
   READY --(T2)--> TERMINATING: nua_bye()
   TERMINATING --(T3)--> TERMINATED: nua_r_bye, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_r_bye);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  /* A: READY -(T1)-> TERMINATED: nua_i_bye, nua_i_state */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_bye);
  TEST(e->data->e_status, 200);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  nua_handle_destroy(a_call->nh), a_call->nh = NULL;
  nua_handle_destroy(b_call->nh), b_call->nh = NULL;

  if (print_headings)
    printf("TEST NUA-10.3.1: PASSED\n");

  if (print_headings)
    printf("TEST NUA-10.4: Call with preconditions and non-100rel 180\n");

/* Test 100rel and preconditions with NUTAG_ONLY183_100REL(1):

   A			B
   |-------INVITE------>|
   |<----100 Trying-----|
   |			|
   |<-------183---------|
   |-------PRACK------->|
   |<-------200---------|
   |			|
   |<-------180---------|
   |			|
   |<------200 OK-------|
   |--------ACK-------->|
   |			|
   |<-------BYE---------|
   |-------200 OK-------|
   |			|

*/

  a_call->sdp = "m=audio 5008 RTP/AVP 8";
  b_call->sdp = "m=audio 5010 RTP/AVP 0 8";

  nua_set_params(ctx->a.nua,
		 NUTAG_EARLY_MEDIA(1),
		 SIPTAG_SUPPORTED_STR("100rel, precondition"),
		 TAG_END());
  run_a_until(ctx, nua_r_set_params, until_final_response);

  nua_set_params(ctx->b.nua,
		 NUTAG_EARLY_MEDIA(1),
		 NUTAG_ONLY183_100REL(1),
		 SIPTAG_SUPPORTED_STR("100rel, precondition"),
		 TAG_END());
  run_b_until(ctx, nua_r_set_params, until_final_response);

  TEST_1(a_call->nh = nua_handle(a->nua, a_call, SIPTAG_TO(b->to), TAG_END()));

  INVITE(a, a_call, a_call->nh,
	 TAG_IF(!ctx->proxy_tests, NUTAG_URL(b->contact->m_url)),
	 SOATAG_USER_SDP_STR(a_call->sdp),
	 SIPTAG_SUPPORTED_STR("100rel"),
	 SIPTAG_REQUIRE_STR("precondition"),
	 TAG_END());

  run_ab_until(ctx, -1, until_ready, -1, accept_updated);

  /* Client transitions:
     INIT -(C1)-> CALLING: nua_invite(), nua_i_state
     CALLING -(C2)-> PROCEEDING: nua_r_invite, nua_i_state
     PROCEEDING -(C3+C4)-> READY: nua_r_invite, nua_i_state
  */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_calling); /* CALLING */
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 183);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_prack);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  /* Send UPDATE */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(!is_answer_recv(e->data->e_tags));
  TEST_1(is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_update);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding);
  TEST_1(is_answer_recv(e->data->e_tags));
  TEST_1(!is_offer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 180);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_proceeding); /* PROCEEDING */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_r_invite);
  TEST(e->data->e_status, 200);

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  /*
   Server transitions:
   INIT -(S1)-> RECEIVED: nua_i_invite, nua_i_state
   RECEIVED -(S2a)-> EARLY: nua_respond(), nua_i_state
   EARLY -(S3b)-> COMPLETED: nua_respond(), nua_i_state
   COMPLETED -(S4)-> READY: nua_i_ack, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_i_invite);
  TEST(e->data->e_status, 100);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_received); /* RECEIVED */

  /* Responded with 183 Session Progress */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_answer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_prack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_offer_recv(e->data->e_tags));
  TEST_1(is_answer_sent(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_update);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(is_offer_recv(e->data->e_tags));
  TEST_1(is_answer_sent(e->data->e_tags));

  /* Responded with 180 Ringing */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_early); /* EARLY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  /* Responded with 200 OK */
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_completed); /* COMPLETED */
  TEST_1(!is_offer_answer_done(e->data->e_tags));

  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_ack);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_ready); /* READY */
  TEST_1(!is_offer_answer_done(e->data->e_tags));
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  if (print_headings)
    printf("TEST NUA-10.4: PASSED\n");

  if (print_headings)
    printf("TEST NUA-10.4.1: terminate call\n");

  BYE(b, b_call, b_call->nh, TAG_END());
  run_ab_until(ctx, -1, until_terminated, -1, until_terminated);

  /* B transitions:
   READY --(T2)--> TERMINATING: nua_bye()
   TERMINATING --(T3)--> TERMINATED: nua_r_bye, nua_i_state
  */
  TEST_1(e = b->events->head); TEST_E(e->data->e_event, nua_r_bye);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, b->events);

  /* A: READY -(T1)-> TERMINATED: nua_i_bye, nua_i_state */
  TEST_1(e = a->events->head); TEST_E(e->data->e_event, nua_i_bye);
  TEST(e->data->e_status, 200);
  TEST_1(e = e->next); TEST_E(e->data->e_event, nua_i_state);
  TEST(callstate(e->data->e_tags), nua_callstate_terminated); /* TERMINATED */
  TEST_1(!e->next);
  free_events_in_list(ctx, a->events);

  nua_handle_destroy(a_call->nh), a_call->nh = NULL;
  nua_handle_destroy(b_call->nh), b_call->nh = NULL;

  if (print_headings)
    printf("TEST NUA-10.4.1: PASSED\n");

  END();
}