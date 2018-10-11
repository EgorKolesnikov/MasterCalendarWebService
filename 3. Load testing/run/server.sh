# to fix error with external libraries load
export LD_LIBRARY_PATH=/usr/local/lib


if [ $# != 1 ]; then
	echo "Expecting one arg: [http|fastsgi]"
	exit 1
fi


server_type="$1"
conf_path=""


if [ "${server_type}" == "http" ]; then
    conf_path="../configs/server.conf.http.json"
elif [ "${server_type}" == "fastcgi" ]; then
	conf_path="../configs/server.conf.fastcgi.json"
else
	echo "Unknown server type ${server_type}. Available: [http|fastcgi]"
	exit 1
fi


echo "RUNNING SERVER. Config: ${conf_path}"
"../../1. hello world/src/hello" -c "${conf_path}"
