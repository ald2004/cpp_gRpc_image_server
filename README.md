# cpp_gRpc_image_server

## 1.git cloen https://github.com/grpc/grpc.git
    export MY_INSTALL_DIR=. cmake -DgRPC_INSTALL=ON       -DgRPC_BUILD_TESTS=OFF       -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR       ../..
    make -j -s install
## 2. cd build
## 3. cmake ..
## 4. make -j -s 
## 5. ./boeRpcImageAsyncServer 
## 6. ./boeRpcAsyncClient
## 7. log
    Boeedge:/data/image_server/build> ./boeRpcImageAsyncServer 
     Server listening on 0.0.0.0:50051
