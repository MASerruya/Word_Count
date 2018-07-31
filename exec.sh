if [ -z "$1" ];
then
echo '4 default processes in execution.'
mpirun -np 4 --hostfile machines example6 text.txt
exit 0
else
echo $1 'process(es) in execution'
mpirun -np $1 --hostfile machines example6 text.txt
exit 0
fi
