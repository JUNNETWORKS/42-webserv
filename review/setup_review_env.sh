set -eux

if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo:"
    echo "sudo $0 $*"
    exit 1
fi

# Nginx が80番ポートを使っている可能性があるので止めておく
systemctl stop nginx

rm -rf /var/webserv

mkdir -p /var/webserv/

cp -r error_pages /var/webserv/error_pages
cp -r server1 /var/webserv/server1
cp -r server2 /var/webserv/server2
cp -r server3 /var/webserv/server3
cp -r server4 /var/webserv/server4

if grep -q "127.0.0.1 webserv.com" /etc/hosts; then
  echo "127.0.0.1 webserv.com" >> /etc/hosts
fi
