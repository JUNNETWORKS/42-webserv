.PHONY: all
all:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint make test_webserv
	docker-compose -f docker-compose.yml up --build

.PHONY: clean
clean:
	docker-compose -f docker-compose.yml down

.PHONY: fclean
fclean: clean

.PHONY: re
re: fclean all

.PHONY: req-test
req-test: WEBSERV_PORT := 49200
req-test:
	$(MAKE) -C public/files
	python3 -u -m py-test --WEBSERV_PORT=$(WEBSERV_PORT)

.PHONY: g-test
g-test:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint make test_webserv test

.PHONY: cppcheck
cppcheck:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint cppcheck test_webserv --enable=all --output-file=test/cppcheck-err-list srcs -I srcs
	@echo
	cat cppcheck-err-list

.PHONY: webserv-it
webserv-it:
	docker exec -it webserv bash

.PHONY: nginx-it
nginx-it:
	docker exec -it nginx bash

.PHONY: nginx-reload
nginx-reload:
	docker exec -t nginx nginx -s reload

.PHONY: apache-it
apache-it:
	docker exec -it apache bash
