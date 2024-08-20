platforms = drumlogue minilogue-xd prologue nutekt-digital nts-1_mkii nts-3_kaoss

all:
	echo "Creating non-existent platform dirs..."
	for p in $(platforms); do \
		mkdir -p platform/$${p}; \
        done
	echo "Building units..."
	mkdir -p platform/bin
	git submodule update --init platform/ext/CMSIS
	logue-sdk/docker/run_cmd.sh --platform=platform build
	echo "Listing units..."
	ls -l platform/bin

image:
	echo "Creating logue-sdk Docker image..."
	git submodule update --init logue-sdk
	cd logue-sdk/docker; \
	./build_image.sh
	docker images

cli:
	echo "Setting up logue-cli binary..."
	logue-sdk/tools/logue-cli/get_logue_cli_linux.sh
	chmod 755 logue-sdk/tools/logue-cli/logue-cli-linux*/logue-cli
	ln -s logue-sdk/tools/logue-cli/logue-cli-linux*/logue-cli logue-cli

clean:
	echo "Cleaning up bin and build directories..."
	rm -rf platform/bin
	for p in $(platforms); do \
		for u in platform/$${p}/*; do \
			rm -rf $${u}/build; \
		done; \
	done

	echo "Cleaning up logue-cli..."
	rm -f logue-cli
	rm -rf logue-sdk/tools/logue-cli/logue-cli-linux*

	echo "Removing logue-sdk Docker image..."
	id=$(shell docker images | grep logue-sdk-dev-env | grep "latest" | awk '{print $$3}'); \
	if [ -n "$${id}" ]; then \
		docker rmi --force $${id}; \
	fi
	docker images
