#include "common.h"

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>
#include<cstdio>
#include<ctime>
#include <chrono>
#include <atomic>

erpc::Rpc<erpc::CTransport> *rpc;
erpc::MsgBuffer req;
erpc::MsgBuffer resp;
int session_num;
std::atomic<int64_t> received_num(0);
auto stop = std::chrono::high_resolution_clock::now();
bool block = true;

// this function is called if we receive a response from the server
void cont_func(void *, void *) { 
	block = false;
        received_num++;
        //printf("%s\n", resp.buf_);
	std::cout << "received_num: " << received_num << std::endl;
        if (received_num == 100){
            stop = std::chrono::high_resolution_clock::now();
        }
}

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void *) {}


//void add(std::vector<int>& _req){
//  //serialize vector into string
//        std::ostringstream oss;
//        if (!_req.empty()){
//            // Convert all but the last element to avoid a trailing ","
//            std::copy(_req.begin(), _req.end()-1,
//            std::ostream_iterator<int>(oss, ","));
//
//            // Now add the last element with no delimiter
//            oss << _req.back();
//        }   
//        std::string str = oss.str();
//        const char* chr = str.c_str();
//        req = rpc->alloc_msg_buffer_or_die(strlen(chr));
//        sprintf(reinterpret_cast<char *>(req.buf_), "%s", chr);
//        resp = rpc->alloc_msg_buffer_or_die(kMsgSize);
//
//        rpc->enqueue_request(session_num, 2, &req, &resp, cont_func, nullptr);
//        // rpc->run_event_loop(2);
//}


void hello(std::vector<int>& _req){
        std::ostringstream oss;
        if (!_req.empty()){
            // Convert all but the last element to avoid a trailing ","
            std::copy(_req.begin(), _req.end()-1,
                std::ostream_iterator<int>(oss, ","));

            // Now add the last element with no delimiter
            oss << _req.back();
        }   
        std::string str =  oss.str();
        const char* chr = str.c_str();
        req = rpc->alloc_msg_buffer_or_die(strlen(chr));
        sprintf(reinterpret_cast<char *>(req.buf_), "%s", chr);
        resp = rpc->alloc_msg_buffer_or_die(kMsgSize);

        rpc->enqueue_request(session_num, 1, &req, &resp, cont_func, nullptr);
	while (block) {
           int num_pkts = rpc->run_event_loop_once();
           if (num_pkts > 0)
             std::cout << "num_pkts: " << num_pkts << std::endl;
	}
	block = true;
}

int main() {
  std::string client_uri = kClientHostname + ":" + std::to_string(kUDPPortClient);
  erpc::Nexus nexus(client_uri);

  rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, sm_handler);

  std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
  session_num = rpc->create_session(server_uri, 1);

  while (!rpc->is_connected(session_num)) rpc->run_event_loop_once();
  std::cout << "connected to the server" << std::endl;

  long long n = 100;
  auto start = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < n; i ++){  
    std::vector<int> hello_req;
    hello_req.push_back(i);
    hello_req.push_back(i + 1);
    hello(hello_req);
  }

  while(received_num < n){
    sleep(1);
  }
  
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  std::cout << "eRPC hello time: " << duration.count() << " microsseconds" << std::endl;
  std::cout << received_num << std::endl;
  delete rpc;
}
