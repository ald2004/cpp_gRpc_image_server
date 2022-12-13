/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>


#include "image.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using boe::upLoadImage;
using boe::UploadReply;
using boe::ABImage;

class BoeImageClient {
 public:
  explicit BoeImageClient(std::shared_ptr<Channel> channel)
      : stub_(upLoadImage::NewStub(channel)) {}

  // Assembles the client's payload and sends it to the server.
  void upLoadImage(const std::string& user) {
    // Data we are sending to the server.
    ABImage request;

    // std::cout << "get data : " << std::string(user).substr(0,10) << "and " << size<< std::endl;
    request.set_data(user);
    // request.set_data(rowData);
    // request.set_data(user);
    // request.set_data("set_message");
    // request.set_msg("set_message");
    // request.set_width(300);request.set_height(300);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    // call->response_reader =
    //     stub_->PrepareAsyncSayHello(&call->context, request, &cq_);
    call->response_reader = 
        stub_->PrepareAsyncdoUpload(&call->context,request,&cq_);

    // StartCall initiates the RPC call
    call->response_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      if (call->status.ok())
        std::cout<<" .";
        // std::cout << "Greeter received: "<<call->status.ok() << " " << call->reply.message() << std::endl;
      else
        std::cout << "RPC failed: "<<call->status.ok() << " "<< call->reply.message()  << std::endl;
        

      // Once we're complete, deallocate the call object.
      delete call;
    }
  }

 private:
  // struct for keeping state and data information
  struct AsyncClientCall {
    // Container for the data we expect from the server.
    UploadReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Storage for the status of the RPC upon completion.
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<UploadReply>> response_reader;
  };

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<upLoadImage::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  BoeImageClient client(grpc::CreateChannel(
      "localhost:50052", grpc::InsecureChannelCredentials()));

  // Spawn reader thread that loops indefinitely
  std::thread thread_ = std::thread(&BoeImageClient::AsyncCompleteRpc, &client);
  // std::string path("/aiplatform/lifei/detect/dataset_car_train/reault_vehicle/train/186315.jpg");
  std::string path("../xxx.jpg");
  
  std::ifstream fs(path,std::ifstream::in | std::ios::binary);
  std::ostringstream os;
  std::string user;  
  if(fs.good()){
    os << fs.rdbuf();
    user=os.str();
  }else{
    std::cout << "------------------------------" << std::endl;
  }

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  
  for (int i = 0; i < atoi(argv[1]); i++) {
    // std::string user("world " + std::to_string(i));
    // client.upLoadImage(user);  // The actual RPC call!
    
    
    
    //   // std::string reply = client.upLoadImage(os.str());
    //   std::cout<< os.str().substr(0,20) <<std::endl;
      // std::cout << "ccccccccc "<< user.substr(0,8) <<std::endl;
      // std::cout << "sent data : " << user.substr(0,10) << "and " <<user.length()<< std::endl;
      client.upLoadImage(user);
      // std::cout<< user.max_size() <<std::endl;
    // }
  }


  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << std::endl<<std::flush;
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count() << "[us]" << std::endl<<std::flush;
  std::cout << "Press control-c to quit" << std::endl << std::endl;
  thread_.join();  // blocks forever

  return 0;
}
