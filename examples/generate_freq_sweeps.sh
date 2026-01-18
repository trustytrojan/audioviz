#!/bin/sh

log_sweep() {
    local f1=$1
    local f2=$2
    local dur=$3
    
    ffmpeg -f lavfi -i "aevalsrc='sin(2*PI*$f1*($dur/log($f2/$f1))*(exp(t*log($f2/$f1)/$dur)-1))':d=$dur" "sweep-$f1-$f2-$dur.wav"
}

panned_sweep() {
    local f1=$1
    local f2=$2
    local dur=$3
    local pan_f=$4
    
    # Define the core sweep expression
    local S="sin(2*PI*$f1*($dur/log($f2/$f1))*(exp(t*log($f2/$f1)/$dur)-1))"
    
    ffmpeg -f lavfi -i "aevalsrc='$S*(0.5+0.5*sin(2*PI*$pan_f*t))|$S*(0.5-0.5*sin(2*PI*$pan_f*t))':d=$dur" "panned-sweep-$f1-$f2-$dur-$pan_f.wav"
}
