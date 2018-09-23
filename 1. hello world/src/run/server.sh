(cd .. && make)

# to fix error with external libraries load
export LD_LIBRARY_PATH=/usr/local/lib

echo 'RUNNING SERVER'
../hello -c ../config.json
