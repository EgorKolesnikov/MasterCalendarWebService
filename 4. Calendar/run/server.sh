make

# to fix error with external libraries load
export LD_LIBRARY_PATH=/usr/local/lib


function init_nginx(){
	# When using fastcgi protocol we require http server before our server.
	# Init nginx with our config file and restart.

    sudo rm /etc/nginx/sites-enabled/cppcms.calendar
	sudo ln -s "$(pwd)/configs/nginx.conf" /etc/nginx/sites-enabled/cppcms.calendar
	sudo service nginx restart
}


#
# 	Expecting single argiment 
# 	with protocol type to use
#

if [ $# != 1 ]; then
	echo "Expecting one arg: [http|fastsgi]"
	exit 1
fi


protocol_type="$1"  # specified protocol type
conf_path=""        # path to the config (depends on specified protocol type)


#
# 	Recognize config file path
# 	by specified protocol type
#

if [ "${protocol_type}" == "http" ]; then
    conf_path="configs/conf.http.json"
elif [ "${protocol_type}" == "fastcgi" ]; then
	conf_path="configs/conf.fastcgi.json"
	init_nginx
else
	echo "Unknown server type ${protocol_type}. Available: [http|fastcgi]"
	exit 1
fi


#
#	Running server with recognized config
#

echo "RUNNING SERVER. Config: ${conf_path}"
./server_calendar -c "${conf_path}"
