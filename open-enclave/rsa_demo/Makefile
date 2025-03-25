# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

.PHONY: all build clean run simulate

OE_CRYPTO_LIB := openssl_3
export OE_CRYPTO_LIB

all: enclave/enclave.so.signed host/enclave_host.bin
	mkdir -p output
	cp host/enclave_host.bin enclave/enclave.so.signed output/

enclave/enclave.so.signed:
	$(MAKE) -C enclave

host/enclave_host.bin:
	$(MAKE) -C host

clean:
	$(MAKE) -C enclave clean
	$(MAKE) -C host clean
	rm -rf output

run:
	host/helloworld_host ./enclave/enclave.signed

simulate:
	host/helloworld_host ./enclave/enclave.signed --simulate

