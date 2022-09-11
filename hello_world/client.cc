#include "common.h"
#include <atomic>
#include <chrono>
#include <iostream>


erpc::Rpc<erpc::CTransport> *rpc;
erpc::MsgBuffer req;
erpc::MsgBuffer resp;
int session_num;
int temp_pointer=10;
std::atomic<int64_t> received_num(0);
auto stop = std::chrono::high_resolution_clock::now();
std::atomic<bool> block(true);

// this function is called if we receive a response from the server
void cont_func(void *_context, void *_tag) {
  block = false;
  received_num++;
  std::cout << "received_num: " << received_num << std::endl;
  std::cout << "the held pointer(_tag): " << (*(int*)_tag) << std::endl;
  std::cout << "received message: " << (char*)resp.buf_ << std::endl;
  if (received_num == 5) {
    stop = std::chrono::high_resolution_clock::now();
  }
}

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void *) {}

void hello(int i) {
  std::string v = std::to_string(i); 
  const char *chr = v.c_str();
  req = rpc->alloc_msg_buffer_or_die(strlen(chr));
  sprintf(reinterpret_cast<char *>(req.buf_), "%s", chr);
  resp = rpc->alloc_msg_buffer_or_die(kMsgSize);

  rpc->enqueue_request(session_num, 1, &req, &resp, cont_func, &temp_pointer);
  while (block) {
    int num_pkts = rpc->run_event_loop_once();
  }
  block = true;
}

int main() {
  std::string client_uri =
      kClientHostname + ":" + std::to_string(kUDPPortClient);
  erpc::Nexus nexus(client_uri);

  rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, sm_handler);

  std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
  session_num = rpc->create_session(server_uri, 1);

  while (!rpc->is_connected(session_num)) rpc->run_event_loop_once();
  std::cout << "connected to the server" << std::endl;

  long long n = 5;
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < n; i++) {
    hello(i);
  }

  while (received_num < n) {
    sleep(1);
  }

  auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  std::cout << "eRPC hello time: " << duration.count() << " microsseconds"
            << std::endl;
  std::cout << received_num << std::endl;
  delete rpc;
}