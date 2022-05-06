for app in gaussian unsharp harris_color camera_pipeline_2x2
do
    echo $app
    aha halide apps/$app 
    aha pipeline apps/$app --width 32 --height 16 
    aha glb apps/$app
done

