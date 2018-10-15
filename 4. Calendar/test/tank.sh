#
# 	Expecting single argiment with test type
#	Possible test types: [post|get|mixed]
#

if [ $# != 1 ]; then
	echo "Expecting one arg: [post|get|mixed]"
	exit 1
fi


test_type="$1"
conf_name="tank.conf.yaml"
ammo_generator="generator.py"
ammo_file_path="ammo.txt"

#
#	Run ammo generation for specified testing type
#

python "${test_type}/${ammo_generator}" > "${test_type}/${ammo_file_path}"

#
#	Run tank
#

cd "${test_type}" && ./run.sh
