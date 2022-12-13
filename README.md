# cpp_gRpc_image_server

## 1.git cloen https://github.com/grpc/grpc.git
    export MY_INSTALL_DIR=. && cmake -DgRPC_INSTALL=ON       -DgRPC_BUILD_TESTS=OFF       -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR       ../..
    make -j -s install
## 2. cd build
## 3. cmake ..
## 4. make -j -s 
## 5. ./boeRpcImageAsyncServer 
## 6. ./boeRpcAsyncClient
## 7. log
    Boeedge:/data/image_server/build> ./boeRpcImageAsyncServer 
     Server listening on 0.0.0.0:50051


## 8. pref 3M per picture, can be upload in 175ms for async request, and can be write done in 1s.
![image](https://user-images.githubusercontent.com/4443533/207253380-002d1b28-c34c-4af5-9965-2ce37aaca16e.png)
