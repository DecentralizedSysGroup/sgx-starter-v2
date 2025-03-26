A template to start an SGX project with Rust SGX SDK v2.0.0-preview. 

The v2 enables cargo-std-aware and makes compiling third-party libs with custom std much easier.


```
git submodule update --init

./start-docker-real-sgx.sh
BUILD_STD=cargo make  # possible options for BUILD_STD: no, cargo, xargo. 
cd bin
./app
```

You should get 

```
[+] Init Enclave Successful 2!
This is a normal world string passed into Enclave!
This is a in-Enclave Rust string!
[+] ECall Success...
```