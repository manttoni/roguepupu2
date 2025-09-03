make
if [ $? -eq 0 ]; then
	valgrind -q --log-file=logs/valgrind.log ./roguepupu2 2> logs/error.log
	if [ $? -ne 0 ]; then
		reset
	fi
else
	echo "Compilation failed"
fi
