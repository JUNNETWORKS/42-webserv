.PHONY: all
all:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint make test_webserv fclean
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint make test_webserv -j
	docker-compose -f docker-compose.yml up --build

.PHONY: clean
clean:
	docker-compose -f docker-compose.yml down

.PHONY: fclean
fclean: clean

.PHONY: re
re: fclean all

.PHONY: test
test:
	python3 test.py

.PHONY: g-test
g-test:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint make test_webserv test

.PHONY: cppcheck
cppcheck:
	docker run -t -v $(shell cd .. && pwd):/root/workdir --rm --entrypoint cppcheck test_webserv --enable=all --output-file=test/cppcheck-err-list srcs
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
