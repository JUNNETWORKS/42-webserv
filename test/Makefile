.PHONY: all
all:
	docker-compose -f docker-compose.yml up --build

.PHONY: clean
clean:
	docker-compose -f docker-compose.yml down

.PHONY: fclean
fclean: clean

.PHONY: re
re: fclean all

.PHONY: webserv-it
webserv-it:
	docker exec -it webserv bash

.PHONY: nginx-it
nginx-it:
	docker exec -it nginx bash

.PHONY: nginx-reload
nginx-reload:
	docker exec -t nginx nginx -s reload
