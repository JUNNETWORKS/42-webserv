FROM	httpd:latest

RUN			set -eux; \
			apt-get update && apt-get install -y \
				python3 \
				sqlite3 \
			&& apt-get clean
