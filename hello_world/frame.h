
// #include <vector>
// #include <string>
// #include <algorithm>
// #include <sstream>
// #include <iterator>
// #include <iostream>
// #include <stdio.h>


// class ERPC_Service {

//     public:    
//         // erpc::Rpc<erpc::CTransport> rpc;
//         public:
//             static void hello(std::vector<int>& _req){
//                 std::cout << "received a hello request" << std::endl;
//                 for(int i : _req)
//                     std::cout << "hello => " << i << std::endl;
//             }
//             static void add(int x, int y){
//                 std::cout << "received an add request" << std::endl;
//                 int z = x + y;
//                 std::cout << "Sum of " << x << " and " << y << " equals " << z << std::endl;
//             }
            
//             ERPC_Service(){}
//         // private:
//             // static void add_wrapper(erpc::ReqHandle *req_handle, void *) {
//             //     std::string s = reinterpret_cast<char *>(req_handle->get_req_msgbuf()->buf_);
//             //     //decode string back into vector
//             //     std::vector<int> vect;
//             //     std::stringstream ss(s);
//             //     for (int i; ss >> i;) {
//             //         vect.push_back(i);    
//             //         if (ss.peek() == ',')
//             //             ss.ignore();
//             //     }
//             //     int x = vect[0];
//             //     int y = vect[1];
//             //     add(x, y);

//             //     auto &resp = req_handle->pre_resp_msgbuf_;
//             //     rpc->resize_msg_buffer(&resp, strlen("add request was processed"));
//             //     sprintf(reinterpret_cast<char *>(resp.buf_), "add request was processed");
//             //     rpc->enqueue_response(req_handle, &resp);
//             // }

//             // static void hello_wrapper(erpc::ReqHandle *req_handle, void *) {
//             //     std::string s = reinterpret_cast<char *>(req_handle->get_req_msgbuf()->buf_);
//             //     //decode string back into vector
//             //     std::vector<int> vect;
//             //     std::stringstream ss(s);
//             //     for (int i; ss >> i;) {
//             //         vect.push_back(i);    
//             //         if (ss.peek() == ',')
//             //             ss.ignore();
//             //     }
                
//             //     hello(vect);

//             //     auto &resp = req_handle->pre_resp_msgbuf_;
//             //     rpc->resize_msg_buffer(&resp, strlen("hello request was processed"));
//             //     sprintf(reinterpret_cast<char *>(resp.buf_), "hello request was processed");
//             //     rpc->enqueue_response(req_handle, &resp);
//             // }
// };

// class ERPC_Client {

// };