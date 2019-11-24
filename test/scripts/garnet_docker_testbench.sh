
echo 'Running pointwise...'
./test/scripts/compare_app.sh pointwise
if [ $? -neq 0]
then
  echo 'Pointwise FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
fi


#docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_app.sh pointwise; echo $?; echo \"done with pointwise\""
##docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/pointwise_res.txt ./pointwise_res.txt
##echo 'pointwise_res.txt...'
##cat pointwise_res.txt
##awk '/./{line=$0} END{print line}' pointwise_res.txt > last_line.txt
##pw_res=`cat last_line.txt`
##echo 'Last line...'
##echo $pw_res
##if [ $pw_res = "Images are equivalent!" ]
##then
  ##echo 'Test passed...'
##else
  ##echo 'Pointwise FAILED'
  ##exit 1
##fi

#echo 'Pointwise result?'
#echo $?

#echo 'Running conv_3_3...'
#docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_test.sh conv_3_3;"
#echo 'Conv result?'
#echo $?

echo 'passed' > tb_res.txt
