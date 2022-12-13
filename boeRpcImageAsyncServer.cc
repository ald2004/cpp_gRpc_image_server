#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <fstream>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>


#include "image.grpc.pb.h"

int count_=0;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using boe::upLoadImage;
using boe::UploadReply;
using boe::ABImage;

inline
std::string get_uuid_32() {
    std::string uuid_dev = "/proc/sys/kernel/random/uuid";

    std::ifstream file(uuid_dev);
    //std::cout << uuid_dev << std::endl;
    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        //std::cout << line << std::endl;
        line.erase(std::remove(line.begin(), line.end(), '-'), line.end());
        //std::cout << line << std::endl;
        file.close();
        return line;
    }
    else {
        //std::cout << "not opened" << std::endl;
        file.close();
        return "";
    }

}


class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
    std::string server_address("0.0.0.0:50052");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(upLoadImage::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      count_++;
      if (status_ == CREATE) {
        // std::cout<< "xxxxxxxxxxxxxxxxxxxxxx ";
        // std::cout<< count_ << " xxxxxxxxxxxxx" <<std::endl;
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        // service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
        //                           this);
        service_->RequestdoUpload(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // std::cout<< "yyyyyyyyyyyyyyyyyyyy ";
        // std::cout<< count_ << " yyyyyyyyyyyyyyyy" <<std::endl;
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.
        std::string prefix("Hello ");
        // reply_.set_message(prefix + request_.name());
        // reply_.set_message(prefix + request_.data());
        std::string tmpimagefilename("/dev/shm/"+get_uuid_32()+".jpg");
        std::ofstream outfile(tmpimagefilename,std::ofstream::binary|std::ofstream::out);
        // std::string rowData = request_.data();
        // std::cout << "server got data: "<<rowData.substr(0,20) << " and " <<rowData.length() <<std::endl;
        // grpc::string mat = request_.data();
        // std::cout << mat.c_str() <<std::endl;
        if(outfile.good()){
            outfile.write(request_.data().c_str(),request_.data().length());
            reply_.set_message(prefix + "file generated!["+std::to_string(request_.data().length())+"]");
            outfile.close();
            
        }
        else{
            reply_.set_message(prefix + "error!["+std::to_string(count_)+"]");
        }
        // And
        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        // std::cout<< "zzzzzzzzzzzzzzzz ";
        // std::cout<< count_ << " zzzzzzzzzzzzz" <<std::endl;
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    upLoadImage::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    ABImage request_;
    // What we send back to the client.
    UploadReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<UploadReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  upLoadImage::AsyncService service_;
  std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) {
  ServerImpl server;
  server.Run();

  return 0;
}
