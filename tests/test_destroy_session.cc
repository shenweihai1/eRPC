#include <gtest/gtest.h>
#include <atomic>
#include <map>
#include <thread>
#include "rpc.h"

using namespace ERpc;

#define NEXUS_UDP_PORT 31851
#define EVENT_LOOP_MS 2000

#define SERVER_APP_TID 100
#define CLIENT_APP_TID 200

/* Shared between client and server thread */
std::atomic<bool> server_ready; /* Client starts after server is ready */
std::atomic<bool> client_done;  /* Server ends after client is done */

const uint8_t phy_port = 0;
const size_t numa_node = 0;
char local_hostname[kMaxHostnameLen];

struct client_context_t {
  size_t nb_sm_events;
  SessionMgmtErrType exp_err;

  client_context_t() { nb_sm_events = 0; }
};

void test_sm_hander(Session *session, SessionMgmtEventType sm_event_type,
                    SessionMgmtErrType sm_err_type, void *_context) {
  _unused(session);
  _unused(sm_event_type);

  client_context_t *context = (client_context_t *)_context;
  context->nb_sm_events++;

  /* Check that the error type matches the expected value */
  ASSERT_EQ(sm_err_type, context->exp_err);
}

/* The server thread used by all tests */
void server_thread_func(Nexus *nexus, uint8_t app_tid) {
  Rpc<IBTransport> rpc(nexus, nullptr, app_tid, &test_sm_hander,
                               phy_port, numa_node);

  server_ready = true;

  while (!client_done) { /* Wait for the client */
    rpc.run_event_loop_timeout(EVENT_LOOP_MS);
  }

  ASSERT_EQ(rpc.num_active_sessions(), 0);
}

//
// Simple successful disconnection of one session, and other simple tests
//
void simple_disconnect(Nexus *nexus) {
  while (!server_ready) { /* Wait for server */
    usleep(1);
  }

  auto *client_context = new client_context_t();
  Rpc<IBTransport> rpc(nexus, (void *)client_context, CLIENT_APP_TID,
                               &test_sm_hander, phy_port, numa_node);

  /* Connect the session */
  client_context->exp_err = SessionMgmtErrType::kNoError;
  Session *session =
      rpc.create_session(local_hostname, SERVER_APP_TID, phy_port);

  /* Try to disconnect the session before it is connected. This should fail. */
  ASSERT_EQ(rpc.destroy_session(session), false);

  rpc.run_event_loop_timeout(EVENT_LOOP_MS);

  ASSERT_EQ(client_context->nb_sm_events, 1); /* The connect event */
  ASSERT_EQ(session->state, SessionState::kConnected);

  /* Disconnect the session */
  client_context->exp_err = SessionMgmtErrType::kNoError;
  rpc.destroy_session(session);
  rpc.run_event_loop_timeout(EVENT_LOOP_MS);

  ASSERT_EQ(client_context->nb_sm_events, 2); /* The disconnect event */
  ASSERT_EQ(rpc.num_active_sessions(), 0);

  // Other simple tests

  /* Try to disconnect the session again. This should fail. */
  ASSERT_EQ(rpc.destroy_session(session), false);

  /* Try to disconnect an invalid session. This should fail. */
  ASSERT_EQ(rpc.destroy_session(nullptr), false);

  client_done = true;
}

TEST(SimpleDisconnect, SimpleDisconnect) {
  Nexus nexus(NEXUS_UDP_PORT, .8);
  server_ready = false;
  client_done = false;

  std::thread server_thread(server_thread_func, &nexus, SERVER_APP_TID);
  std::thread client_thread(simple_disconnect, &nexus);
  server_thread.join();
  client_thread.join();
}

//
// Repeat: Create a session to the server and disconnect it.
//
void disconnect_multi(Nexus *nexus) {
  while (!server_ready) { /* Wait for server */
    usleep(1);
  }

  auto *client_context = new client_context_t();
  Rpc<IBTransport> rpc(nexus, (void *)client_context, CLIENT_APP_TID,
                               &test_sm_hander, phy_port, numa_node);

  for (size_t i = 0; i < 3; i++) {
    client_context->nb_sm_events = 0;
    client_context->exp_err = SessionMgmtErrType::kNoError;

    /* Connect the session */
    Session *session =
        rpc.create_session(local_hostname, SERVER_APP_TID, phy_port);

    rpc.run_event_loop_timeout(EVENT_LOOP_MS);

    ASSERT_EQ(client_context->nb_sm_events, 1); /* The connect event */
    ASSERT_EQ(session->state, SessionState::kConnected);

    /* Disconnect the session */
    client_context->exp_err = SessionMgmtErrType::kNoError;
    rpc.destroy_session(session);
    rpc.run_event_loop_timeout(EVENT_LOOP_MS);

    ASSERT_EQ(client_context->nb_sm_events, 2); /* The disconnect event */
    ASSERT_EQ(rpc.num_active_sessions(), 0);
  }

  client_done = true;
}

TEST(DisconnectMulti, DisconnectMulti) {
  Nexus nexus(NEXUS_UDP_PORT, .8);
  server_ready = false;
  client_done = false;

  std::thread server_thread(server_thread_func, &nexus, SERVER_APP_TID);
  std::thread client_thread(disconnect_multi, &nexus);
  server_thread.join();
  client_thread.join();
}

//
// Disconnect a session that encountered a local or a remote error. This should
// succeed.
//
void disconnect_error(Nexus *nexus) {
  while (!server_ready) { /* Wait for server */
    usleep(1);
  }

  auto *client_context = new client_context_t();
  Rpc<IBTransport> rpc(nexus, (void *)client_context, CLIENT_APP_TID,
                               &test_sm_hander, phy_port, numa_node);

  /* Try to connect the session */
  client_context->exp_err = SessionMgmtErrType::kInvalidRemotePort;
  Session *session =
      rpc.create_session(local_hostname, SERVER_APP_TID, phy_port + 1);

  rpc.run_event_loop_timeout(EVENT_LOOP_MS);

  ASSERT_EQ(client_context->nb_sm_events, 1); /* The connect failed event */
  ASSERT_EQ(session->state, SessionState::kError);

  /* Disconnect the session */
  client_context->exp_err = SessionMgmtErrType::kNoError;
  rpc.destroy_session(session);
  rpc.run_event_loop_timeout(EVENT_LOOP_MS);

  ASSERT_EQ(client_context->nb_sm_events, 2); /* The disconnect event */
  ASSERT_EQ(rpc.num_active_sessions(), 0);

  client_done = true;
}

TEST(DisconnectError, DisconnectError) {
  Nexus nexus(NEXUS_UDP_PORT, .8);
  server_ready = false;
  client_done = false;

  std::thread server_thread(server_thread_func, &nexus, SERVER_APP_TID);
  std::thread client_thread(disconnect_error, &nexus);
  server_thread.join();
  client_thread.join();
}

int main(int argc, char **argv) {
  Nexus::get_hostname(local_hostname);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}