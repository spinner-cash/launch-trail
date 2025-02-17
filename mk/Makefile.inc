OS=$(shell uname -s | tr A-Z a-z)
ARCH=$(shell uname -m)
RELEASE_URL=https://github.com/$(GITHUB_REPO)/releases/download/$(RELEASE_TAG)

CANISTER_INFO=dist/canister-info
LAUNCH_TRAIL=dist/launchtrail.wasm dist/launchtrail.did
CHECKSUM_FILES=dist/launchtrail.wasm.sha256 dist/canister-info-$(ARCH)-$(OS).sha256
ARGUMENT=(record {bucket_size = 1000 : nat64; max_buckets = 1000 : nat64; config = record { revokers = vec { principal "SUBMITTER" }; submitters = vec { principal "SUBMITTER" }; min_schedule = 0 : nat64;}})

NETWORK?=local

default: dist/sha256.checksum $(LAUNCH_TRAIL) $(CANISTER_INFO)
	shasum -a 256 -c $<

dist:
	mkdir -p dist

dist/%.sha256: | dist
	curl -fL $(RELEASE_URL)/$(subst dist/,,$@) -o $@

dist/sha256.checksum: $(CHECKSUM_FILES) | dist
	cat $^ > $@

dist/launchtrail.wasm: | dist
	curl -fL $(RELEASE_URL)/launchtrail.wasm -o $@

dist/launchtrail.did: | dist
	curl -fL $(RELEASE_URL)/launchtrail.did -o $@

dist/canister-info: | dist
	curl -fL $(RELEASE_URL)/canister-info-$(ARCH)-$(OS) -o $@

dfx.json:
	echo '{"canisters":{"launchtrail":{"type":"custom","candid":"dist/launchtrail.did","wasm":"dist/launchtrail.wasm","build":""}}}' > $@

deploy: dfx.json
	dfx deploy launchtrail --network=$(NETWORK) --argument "$$(echo '$(ARGUMENT)'|sed -e "s/SUBMITTER/$$(dfx identity get-principal)/g")"

clean:
	rm -rf dist

.PHONY: default deploy clean
